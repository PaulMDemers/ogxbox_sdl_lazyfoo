param(
    [int]$ExpectedImplementedCount = 15,
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
$apps = @(Get-MakefileList $makefile "LAZYFOO_APPS")

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
}

foreach ($required in @("README.md", "LICENSE", "common\lazyfoo_demo.c", "common\lazyfoo_demo.h", "docs\ROADMAP.md")) {
    if (-not (Test-Path (Join-Path $repo $required))) {
        Add-Failure "Missing required file: $required"
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
