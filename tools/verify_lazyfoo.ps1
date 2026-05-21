param(
    [int]$ExpectedImplementedCount = 0,
    [switch]$CheckReleaseArtifacts,
    [string]$ReleaseRoot = ""
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
$makefilePath = Join-Path $repo "Makefile"

function Add-Failure {
    param([string]$Message)
    $script:failures += $Message
}

function Get-MakefileList {
    param(
        [string]$Text,
        [string]$Name
    )

    $match = [regex]::Match($Text, "(?ms)^$([regex]::Escape($Name))\s*=\s*\\\r?\n(?<body>.*?)(?:\r?\n\r?\n)")
    if (-not $match.Success) {
        throw "Could not find $Name in $makefilePath"
    }

    $items = @()
    foreach ($line in ($match.Groups["body"].Value -split "\r?\n")) {
        $item = ($line -replace "\\", "").Trim()
        if (-not [string]::IsNullOrWhiteSpace($item)) {
            $items += $item
        }
    }
    return $items
}

function Get-EvaluatedAppList {
    $output = & make -s -C $repo print-apps 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "Could not evaluate LAZYFOO_APPS with make:`n$($output -join "`n")"
    }

    return @($output | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
}

function Get-AppIsoName {
    param([string]$App)

    $appMakefilePath = Join-Path $repo (Join-Path $App "Makefile")
    if (-not (Test-Path $appMakefilePath)) {
        return "xblazyfoo_$App.iso"
    }
    $appMakefile = Get-Content -LiteralPath $appMakefilePath -Raw
    if ($appMakefile -match "(?m)^XBE_TITLE\s*=\s*(\S+)") {
        return "$($matches[1]).iso"
    }
    return "xblazyfoo_$App.iso"
}

$failures = @()
$makefile = Get-Content -LiteralPath $makefilePath -Raw
$apps = @(Get-EvaluatedAppList)

if ($apps.Count -ne $ExpectedImplementedCount) {
    if ($ExpectedImplementedCount -eq 0) {
        $ExpectedImplementedCount = $apps.Count
    }
    if ($ExpectedImplementedCount -eq 47 -and ($apps -contains "021_sound_effects_music")) {
        $ExpectedImplementedCount = 48
    }
}

if ($apps.Count -ne $ExpectedImplementedCount) {
    Add-Failure "LAZYFOO_APPS has $($apps.Count) entries, expected $ExpectedImplementedCount."
}

foreach ($app in $apps) {
    $appDir = Join-Path $repo $app
    if (-not (Test-Path $appDir)) {
        Add-Failure "Missing app directory: $app"
        continue
    }
    foreach ($required in @("Makefile", "main.c")) {
        if (-not (Test-Path (Join-Path $appDir $required))) {
            Add-Failure "Missing $required in $app"
        }
    }

    $mainPath = Join-Path $appDir "main.c"
    $lessonMakefilePath = Join-Path $appDir "Makefile"
    if (Test-Path $mainPath) {
        $mainText = Get-Content -LiteralPath $mainPath -Raw
        if ($mainText -match "lazyfoo_demo|\\.\\./common") {
            Add-Failure "$app/main.c still depends on the old shared demo source."
        }
        if ($mainText -match "switch\s*\(\s*LAZYFOO_LESSON\s*\)") {
            Add-Failure "$app/main.c still multiplexes lesson rendering through LAZYFOO_LESSON."
        }
        if ($mainText -notmatch "#define\s+LAZYFOO_LESSON\s+\d+") {
            Add-Failure "$app/main.c does not declare its standalone lesson number."
        }
    }
    if (Test-Path $lessonMakefilePath) {
        $lessonMakefile = Get-Content -LiteralPath $lessonMakefilePath -Raw
        if ($lessonMakefile -match "\\.\\./common|-DLAZYFOO_LESSON") {
            Add-Failure "$app/Makefile still wires the old common lesson build."
        }
    }
}

foreach ($required in @("README.md", "LICENSE", "docs\ROADMAP.md")) {
    if (-not (Test-Path (Join-Path $repo $required))) {
        Add-Failure "Missing required file: $required"
    }
}

foreach ($removed in @("common\lazyfoo_demo.c", "common\lazyfoo_demo.h")) {
    if (Test-Path (Join-Path $repo $removed)) {
        Add-Failure "Obsolete shared tutorial file is still present: $removed"
    }
}

if ($CheckReleaseArtifacts) {
    if ([string]::IsNullOrWhiteSpace($ReleaseRoot)) {
        $ReleaseRoot = Join-Path $repo "dist\release\lazyfoo"
    }
    $isoRoot = Join-Path $ReleaseRoot "isos"
    $xbeRoot = Join-Path $ReleaseRoot "xbes"
    foreach ($root in @($isoRoot, $xbeRoot)) {
        if (-not (Test-Path $root)) {
            Add-Failure "Missing release artifact directory: $root"
        }
    }
    foreach ($app in $apps) {
        $iso = Join-Path $isoRoot (Get-AppIsoName $app)
        $xbe = Join-Path $xbeRoot (Join-Path $app "default.xbe")
        if (-not (Test-Path $iso)) {
            Add-Failure "Missing release ISO: $iso"
        }
        if (-not (Test-Path $xbe)) {
            Add-Failure "Missing release XBE: $xbe"
        }
    }
}

if ($failures.Count -gt 0) {
    throw "Lazy Foo coverage verification failed:`n$($failures -join "`n")"
}

$suffix = if ($CheckReleaseArtifacts) { " with release artifacts" } else { "" }
Write-Host "Verified $($apps.Count) Lazy Foo SDL lessons$suffix."
