param(
    [string[]]$Apps = @(),
    [double]$DelaySeconds = 15.0,
    [ValidateRange(1,10)]
    [int]$LaunchAttempts = 3,
    [string]$IsoRoot = "",
    [string]$OutputRoot = "",
    [string]$EmuRoot = "",
    [switch]$NoSnapshot,
    [switch]$ResumeExisting
)

$ErrorActionPreference = "Stop"

$repo = Resolve-Path (Join-Path $PSScriptRoot "..")
if ([string]::IsNullOrWhiteSpace($IsoRoot)) {
    $IsoRoot = Join-Path $repo "dist\release\lazyfoo\isos"
} else {
    $IsoRoot = (Resolve-Path $IsoRoot).Path
}
if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
    $OutputRoot = Join-Path $repo "dist\captures\xemu\lazyfoo"
}

if ([string]::IsNullOrWhiteSpace($EmuRoot)) {
    $emuCandidates = @(
        (Join-Path $repo "Xbox-Emulator-Files"),
        (Join-Path (Split-Path -Parent $repo) "Xbox-Emulator-Files")
    )
    foreach ($candidate in $emuCandidates) {
        if (Test-Path (Join-Path $candidate "xemu\xemu.exe")) {
            $EmuRoot = $candidate
            break
        }
    }
    if ([string]::IsNullOrWhiteSpace($EmuRoot)) {
        $EmuRoot = $emuCandidates[0]
    }
}

$xemuRoot = Join-Path $EmuRoot "xemu"
$xemuExe = Join-Path $xemuRoot "xemu.exe"
$configPath = Join-Path $xemuRoot "xemu-local.toml"
$biosPath = Join-Path $EmuRoot "bios\Complex_4627.bin"
$mcpxPath = Join-Path $EmuRoot "mcpx\mcpx_1.0.bin"
$hddPath = Join-Path $EmuRoot "hdd\xbox_hdd.qcow2"
$eepromDir = Join-Path $EmuRoot "eeprom"
$eepromPath = Join-Path $eepromDir "eeprom.bin"

foreach ($required in @($xemuExe, $biosPath, $mcpxPath, $hddPath)) {
    if (!(Test-Path $required)) {
        throw "Missing required xemu file: $required"
    }
}

if (!(Test-Path $eepromDir)) {
    New-Item -ItemType Directory -Path $eepromDir -Force | Out-Null
}
$defaultAppDataEeprom = Join-Path $env:APPDATA "xemu\xemu\eeprom.bin"
if (!(Test-Path $eepromPath) -and (Test-Path $defaultAppDataEeprom)) {
    Copy-Item -LiteralPath $defaultAppDataEeprom -Destination $eepromPath
}

$config = @"
[general]
show_welcome = false
skip_boot_anim = true

[display.window]
fullscreen_on_startup = false

[sys]
mem_limit = '64'

[sys.files]
bootrom_path = '$mcpxPath'
flashrom_path = '$biosPath'
eeprom_path = '$eepromPath'
hdd_path = '$hddPath'
"@
Set-Content -Path $configPath -Value $config -Encoding ASCII

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;

public static class LazyFooXemuCapture {
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct POINT {
        public int X;
        public int Y;
    }

    [DllImport("user32.dll")]
    public static extern bool GetClientRect(IntPtr hWnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool ClientToScreen(IntPtr hWnd, ref POINT point);

    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);

    [DllImport("user32.dll")]
    public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);

    [DllImport("user32.dll")]
    public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    public static extern bool SetProcessDPIAware();

    public delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

    [DllImport("user32.dll")]
    public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);

    [DllImport("user32.dll")]
    public static extern int GetWindowTextLength(IntPtr hWnd);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    public static extern int GetWindowText(IntPtr hWnd, System.Text.StringBuilder text, int count);

    [DllImport("user32.dll")]
    public static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);

    [DllImport("user32.dll")]
    public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam);

    public static readonly IntPtr HWND_TOPMOST = new IntPtr(-1);
    public const uint SWP_SHOWWINDOW = 0x0040;
    public const int SW_RESTORE = 9;
    public const uint WM_CLOSE = 0x0010;
}
"@

[LazyFooXemuCapture]::SetProcessDPIAware() | Out-Null

function Get-AppList {
    if ($Apps.Count -gt 0) {
        $out = @()
        foreach ($appToken in $Apps) {
            foreach ($part in ($appToken -split ",")) {
                if (![string]::IsNullOrWhiteSpace($part)) {
                    $out += $part.Trim()
                }
            }
        }
        return $out
    }

    $makeOutput = & make -s -C $repo print-apps
    if ($LASTEXITCODE -ne 0) {
        throw "make print-apps failed"
    }
    return @($makeOutput | Where-Object { ![string]::IsNullOrWhiteSpace($_) })
}

function Get-MainWindowHandle {
    param([System.Diagnostics.Process]$Process)

    for ($i = 0; $i -lt 150; $i++) {
        $Process.Refresh()
        if ($Process.HasExited) {
            throw "xemu exited before capture"
        }
        if ($Process.MainWindowHandle -ne [IntPtr]::Zero) {
            return $Process.MainWindowHandle
        }
        Start-Sleep -Milliseconds 100
    }
    throw "No xemu window handle for process $($Process.Id)"
}

function Set-XemuCaptureWindow {
    param([IntPtr]$Handle)

    [LazyFooXemuCapture]::ShowWindow($Handle, [LazyFooXemuCapture]::SW_RESTORE) | Out-Null
    [LazyFooXemuCapture]::SetWindowPos($Handle, [LazyFooXemuCapture]::HWND_TOPMOST, 20, 20, 800, 600, [LazyFooXemuCapture]::SWP_SHOWWINDOW) | Out-Null
    [LazyFooXemuCapture]::SetForegroundWindow($Handle) | Out-Null
}

function Close-XemuUpdateDialogs {
    param([System.Diagnostics.Process]$Process)

    $targetPid = [uint32]$Process.Id
    $callback = [LazyFooXemuCapture+EnumWindowsProc]{
        param([IntPtr]$hWnd, [IntPtr]$lParam)

        $windowPid = [uint32]0
        [LazyFooXemuCapture]::GetWindowThreadProcessId($hWnd, [ref]$windowPid) | Out-Null
        if ($windowPid -ne $targetPid) {
            return $true
        }

        $length = [LazyFooXemuCapture]::GetWindowTextLength($hWnd)
        if ($length -le 0) {
            return $true
        }

        $builder = New-Object System.Text.StringBuilder ($length + 1)
        [LazyFooXemuCapture]::GetWindowText($hWnd, $builder, $builder.Capacity) | Out-Null
        $title = $builder.ToString()
        if ($title -match '(?i)update|newer version|version of xemu') {
            [LazyFooXemuCapture]::PostMessage($hWnd, [LazyFooXemuCapture]::WM_CLOSE, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
        }
        return $true
    }

    [LazyFooXemuCapture]::EnumWindows($callback, [IntPtr]::Zero) | Out-Null
}

function Get-CaptureStats {
    param([System.Drawing.Bitmap]$Bitmap)

    $samples = 0
    $total = 0.0
    $nonDark = 0
    for ($y = 0; $y -lt $Bitmap.Height; $y += 16) {
        for ($x = 0; $x -lt $Bitmap.Width; $x += 16) {
            $pixel = $Bitmap.GetPixel($x, $y)
            $value = ($pixel.R + $pixel.G + $pixel.B) / 3.0
            $total += $value
            if ($value -gt 10.0) {
                $nonDark++
            }
            $samples++
        }
    }
    if ($samples -eq 0) {
        $samples = 1
    }

    return [ordered]@{
        mean_brightness = $total / $samples
        non_dark_ratio = $nonDark / $samples
    }
}

function Copy-XemuClientToBitmap {
    param(
        [IntPtr]$Handle,
        [System.Drawing.Bitmap]$Target
    )

    $clientPoint = New-Object LazyFooXemuCapture+POINT
    $clientPoint.X = 0
    $clientPoint.Y = 0
    [LazyFooXemuCapture]::ClientToScreen($Handle, [ref]$clientPoint) | Out-Null

    $graphics = [System.Drawing.Graphics]::FromImage($Target)
    try {
        $graphics.CopyFromScreen($clientPoint.X, $clientPoint.Y, 0, 0, $Target.Size)
    } finally {
        $graphics.Dispose()
    }
}

function Get-RowMeanBrightness {
    param(
        [System.Drawing.Bitmap]$Bitmap,
        [int]$Y
    )

    $yClamped = [Math]::Max(0, [Math]::Min($Bitmap.Height - 1, $Y))
    $samples = 0
    $total = 0.0
    for ($x = 0; $x -lt $Bitmap.Width; $x += 8) {
        $pixel = $Bitmap.GetPixel($x, $yClamped)
        $total += ($pixel.R + $pixel.G + $pixel.B) / 3.0
        $samples++
    }
    if ($samples -eq 0) {
        return 0.0
    }
    return $total / $samples
}

function Remove-XemuMenuBarIfPresent {
    param([System.Drawing.Bitmap]$Bitmap)

    if ($Bitmap.Height -le 260) {
        return $Bitmap
    }

    $row0 = Get-RowMeanBrightness -Bitmap $Bitmap -Y 0
    $row10 = Get-RowMeanBrightness -Bitmap $Bitmap -Y 10
    $row20 = Get-RowMeanBrightness -Bitmap $Bitmap -Y 20
    $row24 = Get-RowMeanBrightness -Bitmap $Bitmap -Y 24

    $hasMenuBar = (($row0 -gt 20.0) -and ($row10 -gt 30.0) -and ($row20 -gt 20.0) -and ($row24 -lt ($row10 - 15.0)))
    if (-not $hasMenuBar) {
        return $Bitmap
    }

    $cropTop = 22
    $cropRect = New-Object System.Drawing.Rectangle 0, $cropTop, $Bitmap.Width, ($Bitmap.Height - $cropTop)
    $cropped = New-Object System.Drawing.Bitmap $cropRect.Width, $cropRect.Height
    $graphics = [System.Drawing.Graphics]::FromImage($cropped)
    try {
        $targetRect = New-Object System.Drawing.Rectangle 0, 0, $cropped.Width, $cropped.Height
        $graphics.DrawImage($Bitmap, $targetRect, $cropRect, [System.Drawing.GraphicsUnit]::Pixel)
    } finally {
        $graphics.Dispose()
    }
    $Bitmap.Dispose()
    return $cropped
}

function Capture-XemuIso {
    param(
        [string]$Iso,
        [string]$OutPath
    )

    $args = @("-config_path", $configPath, "-dvd_path", $Iso)
    if (-not $NoSnapshot) {
        $args += "-snapshot"
    }

    $lastFailure = $null
    for ($launchAttempt = 1; $launchAttempt -le $LaunchAttempts; ++$launchAttempt) {
        $proc = $null
        try {
            Write-Host ("Starting xemu launch attempt {0}/{1}: {2}" -f $launchAttempt, $LaunchAttempts, $Iso)
            $proc = Start-Process -FilePath $xemuExe -ArgumentList $args -WorkingDirectory $xemuRoot -PassThru
            $handle = Get-MainWindowHandle -Process $proc
            Set-XemuCaptureWindow -Handle $handle
            Start-Sleep -Milliseconds ([int]($DelaySeconds * 1000))
            Close-XemuUpdateDialogs -Process $proc
            Set-XemuCaptureWindow -Handle $handle
            Start-Sleep -Milliseconds 500

            $rect = New-Object LazyFooXemuCapture+RECT
            [LazyFooXemuCapture]::GetClientRect($handle, [ref]$rect) | Out-Null
            $width = [Math]::Max(1, $rect.Right - $rect.Left)
            $height = [Math]::Max(1, $rect.Bottom - $rect.Top)
            $bitmap = New-Object System.Drawing.Bitmap $width, $height
            try {
                New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutPath) | Out-Null
                Copy-XemuClientToBitmap -Handle $handle -Target $bitmap
                $stats = Get-CaptureStats -Bitmap $bitmap
                if ($stats.non_dark_ratio -lt 0.001) {
                    throw ("Capture looks blank; mean_brightness={0:N2} non_dark_ratio={1:N4}" -f $stats.mean_brightness, $stats.non_dark_ratio)
                }
                $bitmap = Remove-XemuMenuBarIfPresent -Bitmap $bitmap
                $bitmap.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
                Write-Host "Captured $OutPath"
                return $stats
            } finally {
                $bitmap.Dispose()
            }
        } catch {
            $lastFailure = $_.Exception.Message
        } finally {
            if ($proc -ne $null -and -not $proc.HasExited) {
                Stop-Process -Id $proc.Id -Force
            }
        }

        if ($launchAttempt -lt $LaunchAttempts) {
            Write-Warning "$lastFailure; retrying with a fresh xemu process"
            Start-Sleep -Milliseconds 1500
        }
    }

    throw $lastFailure
}

$appList = Get-AppList
New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
$captures = @()

foreach ($app in $appList) {
    $lessonNumber = ($app -split "_")[0]
    $iso = Join-Path $IsoRoot ("xblazyfoo_{0}.iso" -f ($app -replace "^\d+_", "$lessonNumber`_"))
    if (-not (Test-Path $iso)) {
        $matchingIso = Get-ChildItem -LiteralPath $IsoRoot -Filter ("xblazyfoo_{0}_*.iso" -f $lessonNumber) | Select-Object -First 1
        if ($null -eq $matchingIso) {
            throw "Missing ISO for $app under $IsoRoot"
        }
        $iso = $matchingIso.FullName
    }

    $out = Join-Path $OutputRoot ("{0}.png" -f $app)
    if ($ResumeExisting -and (Test-Path $out)) {
        Write-Host "Skipping existing capture $out"
        $captures += [ordered]@{
            app = $app
            iso = $iso
            path = $out
            resumed = $true
        }
        continue
    }

    $stats = Capture-XemuIso -Iso $iso -OutPath $out
    $captures += [ordered]@{
        app = $app
        iso = $iso
        path = $out
        mean_brightness = $stats.mean_brightness
        non_dark_ratio = $stats.non_dark_ratio
    }
}

$manifest = [ordered]@{
    generated_at = (Get-Date).ToString("o")
    delay_seconds = $DelaySeconds
    launch_attempts = $LaunchAttempts
    no_snapshot = [bool]$NoSnapshot
    iso_root = $IsoRoot
    output_root = $OutputRoot
    captures = $captures
}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $OutputRoot "manifest.json") -Encoding ASCII
Write-Host ("Captured {0} Lazy Foo lesson(s) to {1}" -f $captures.Count, $OutputRoot)
