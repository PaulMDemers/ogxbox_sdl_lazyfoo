param(
    [string]$CaptureRoot = "",
    [string]$CompareRoot = "",
    [string]$OutputPath = "",
    [ValidateRange(1,12)]
    [int]$Columns = 4,
    [ValidateRange(120,1200)]
    [int]$ThumbWidth = 320,
    [ValidateRange(90,900)]
    [int]$ThumbHeight = 240
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
if ([string]::IsNullOrWhiteSpace($CaptureRoot)) {
    $CaptureRoot = Join-Path $repo "dist\captures\xemu\lazyfoo"
}
$CaptureRoot = (Resolve-Path $CaptureRoot).Path

if (-not [string]::IsNullOrWhiteSpace($CompareRoot)) {
    $CompareRoot = (Resolve-Path $CompareRoot).Path
}

if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $CaptureRoot "contact_sheet.png"
}

Add-Type -AssemblyName System.Drawing

function Get-CaptureEntries {
    param([string]$Root)

    $manifestPath = Join-Path $Root "manifest.json"
    if (Test-Path $manifestPath) {
        $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
        $entries = @()
        foreach ($capture in $manifest.captures) {
            $path = $capture.path
            if ([string]::IsNullOrWhiteSpace($path)) {
                continue
            }
            if (-not [System.IO.Path]::IsPathRooted($path)) {
                $path = Join-Path $Root $path
            }
            $entries += [ordered]@{
                name = $capture.app
                path = $path
            }
        }
        return ,$entries
    }

    return ,@(Get-ChildItem -LiteralPath $Root -Filter "*.png" |
        Where-Object { $_.Name -notmatch "contact_sheet|comparison_sheet" } |
        Sort-Object Name |
        ForEach-Object {
            [ordered]@{
                name = [System.IO.Path]::GetFileNameWithoutExtension($_.Name)
                path = $_.FullName
            }
        })
}

function New-Thumbnail {
    param(
        [string]$Path,
        [int]$Width,
        [int]$Height
    )

    $thumb = New-Object System.Drawing.Bitmap $Width, $Height
    $graphics = [System.Drawing.Graphics]::FromImage($thumb)
    try {
        $graphics.Clear([System.Drawing.Color]::FromArgb(8, 13, 24))
        if (-not (Test-Path $Path)) {
            return $thumb
        }

        $source = [System.Drawing.Image]::FromFile($Path)
        try {
            $scale = [Math]::Min($Width / [double]$source.Width, $Height / [double]$source.Height)
            $drawWidth = [Math]::Max(1, [int]($source.Width * $scale))
            $drawHeight = [Math]::Max(1, [int]($source.Height * $scale))
            $x = [int](($Width - $drawWidth) / 2)
            $y = [int](($Height - $drawHeight) / 2)
            $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $graphics.DrawImage($source, $x, $y, $drawWidth, $drawHeight)
        } finally {
            $source.Dispose()
        }
    } finally {
        $graphics.Dispose()
    }
    return $thumb
}

function Get-MeanAbsoluteDelta {
    param(
        [System.Drawing.Bitmap]$A,
        [System.Drawing.Bitmap]$B
    )

    $samples = 0
    $total = 0.0
    for ($y = 0; $y -lt $A.Height; $y += 8) {
        for ($x = 0; $x -lt $A.Width; $x += 8) {
            $pa = $A.GetPixel($x, $y)
            $pb = $B.GetPixel($x, $y)
            $total += ([Math]::Abs($pa.R - $pb.R) + [Math]::Abs($pa.G - $pb.G) + [Math]::Abs($pa.B - $pb.B)) / 3.0
            $samples++
        }
    }
    if ($samples -eq 0) {
        return 0.0
    }
    return $total / $samples
}

$entries = Get-CaptureEntries -Root $CaptureRoot
if ($entries.Count -eq 0) {
    throw "No captures found under $CaptureRoot"
}

$compareEntries = @{}
if (-not [string]::IsNullOrWhiteSpace($CompareRoot)) {
    foreach ($entry in (Get-CaptureEntries -Root $CompareRoot)) {
        $compareEntries[$entry.name] = $entry.path
    }
}

$labelHeight = 26
$innerGap = if ($compareEntries.Count -gt 0) { 8 } else { 0 }
$cellWidth = if ($compareEntries.Count -gt 0) { ($ThumbWidth * 2) + $innerGap } else { $ThumbWidth }
$cellHeight = $ThumbHeight + $labelHeight
$rows = [int][Math]::Ceiling($entries.Count / [double]$Columns)
$sheetWidth = $Columns * $cellWidth
$sheetHeight = $rows * $cellHeight

$sheet = New-Object System.Drawing.Bitmap $sheetWidth, $sheetHeight
$graphics = [System.Drawing.Graphics]::FromImage($sheet)
$font = New-Object System.Drawing.Font "Consolas", 11
$brush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::White)
$mutedBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(190, 210, 220))
$backgroundBrush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(8, 13, 24))
$comparison = @()

try {
    $graphics.Clear([System.Drawing.Color]::FromArgb(8, 13, 24))
    for ($i = 0; $i -lt $entries.Count; ++$i) {
        $entry = $entries[$i]
        $col = $i % $Columns
        $row = [int][Math]::Floor($i / $Columns)
        $x = $col * $cellWidth
        $y = $row * $cellHeight
        $graphics.FillRectangle($backgroundBrush, $x, $y, $cellWidth, $cellHeight)

        $left = New-Thumbnail -Path $entry.path -Width $ThumbWidth -Height $ThumbHeight
        $right = $null
        $delta = $null
        try {
            $label = $entry.name
            if ($compareEntries.Count -gt 0) {
                $comparePath = $compareEntries[$entry.name]
                if ($comparePath) {
                    $right = New-Thumbnail -Path $comparePath -Width $ThumbWidth -Height $ThumbHeight
                    $delta = Get-MeanAbsoluteDelta -A $left -B $right
                    $label = "{0}  delta={1:N1}" -f $entry.name, $delta
                } else {
                    $right = New-Object System.Drawing.Bitmap $ThumbWidth, $ThumbHeight
                    $missingGraphics = [System.Drawing.Graphics]::FromImage($right)
                    try {
                        $missingGraphics.Clear([System.Drawing.Color]::FromArgb(30, 15, 15))
                        $missingGraphics.DrawString("missing reference", $font, $mutedBrush, 12, 12)
                    } finally {
                        $missingGraphics.Dispose()
                    }
                    $label = "{0}  missing reference" -f $entry.name
                }
            }

            $graphics.DrawString($label, $font, $brush, $x + 4, $y + 5)
            $graphics.DrawImage($left, $x, $y + $labelHeight, $ThumbWidth, $ThumbHeight)
            if ($right) {
                $graphics.DrawImage($right, $x + $ThumbWidth + $innerGap, $y + $labelHeight, $ThumbWidth, $ThumbHeight)
            }

            $comparison += [ordered]@{
                name = $entry.name
                capture = $entry.path
                reference = if ($compareEntries.Count -gt 0) { $compareEntries[$entry.name] } else { $null }
                mean_absolute_delta = $delta
            }
        } finally {
            $left.Dispose()
            if ($right) {
                $right.Dispose()
            }
        }
    }

    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutputPath) | Out-Null
    $sheet.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
} finally {
    $graphics.Dispose()
    $font.Dispose()
    $brush.Dispose()
    $mutedBrush.Dispose()
    $backgroundBrush.Dispose()
    $sheet.Dispose()
}

$manifest = [ordered]@{
    generated_at = (Get-Date).ToString("o")
    capture_root = $CaptureRoot
    compare_root = if ([string]::IsNullOrWhiteSpace($CompareRoot)) { $null } else { $CompareRoot }
    output_path = $OutputPath
    columns = $Columns
    thumb_width = $ThumbWidth
    thumb_height = $ThumbHeight
    entries = $comparison
}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath ([System.IO.Path]::ChangeExtension($OutputPath, ".json")) -Encoding ASCII
Write-Host "Wrote contact sheet $OutputPath"
