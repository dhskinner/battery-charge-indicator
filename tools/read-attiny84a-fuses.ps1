param(
    [string]$Programmer = "usbasp",
    [string]$Part = "t84",
    [string]$AvrDude = "avrdude"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-FuseByte {
    param(
        [string]$FuseName,
        [string]$AvrDudeExe,
        [string]$ProgrammerId,
        [string]$PartId,
        [string]$TempDir
    )

    $outFile = Join-Path $TempDir ("{0}.bin" -f $FuseName)

    & $AvrDudeExe -c $ProgrammerId -p $PartId -U ("{0}:r:{1}:r" -f $FuseName, $outFile) | Out-Null

    if ($LASTEXITCODE -ne 0) {
        throw "avrdude failed while reading $FuseName (exit code: $LASTEXITCODE)."
    }

    if (-not (Test-Path $outFile)) {
        throw ("Expected output file not found for " + $FuseName + ". File: " + $outFile)
    }

    $bytes = [System.IO.File]::ReadAllBytes($outFile)
    if ($bytes.Length -lt 1) {
        throw "No data was read for $FuseName."
    }

    return [byte]$bytes[0]
}

function Show-Bit {
    param(
        [string]$Name,
        [byte]$Value,
        [int]$Bit,
        [bool]$ActiveLow,
        [string]$Meaning
    )

    $isOne = (($Value -band (1 -shl $Bit)) -ne 0)
    $featureOn = if ($ActiveLow) { -not $isOne } else { $isOne }

    $state = if ($featureOn) { "ON" } else { "OFF" }
    $bitValue = if ($isOne) { 1 } else { 0 }
    Write-Host ("  {0,-10} {1,-3} (bit{2}={3}) - {4}" -f $Name, $state, $Bit, $bitValue, $Meaning)
}

function BitsToBinaryString {
    param(
        [byte]$Value,
        [int[]]$BitOrder
    )

    $chars = foreach ($b in $BitOrder) {
        if (($Value -band (1 -shl $b)) -ne 0) { "1" } else { "0" }
    }
    return ($chars -join "")
}

function HexByte {
    param([byte]$Value)
    return ("0x{0:X2}" -f $Value)
}

if (-not (Get-Command $AvrDude -ErrorAction SilentlyContinue)) {
    throw "Could not find '$AvrDude' in PATH. Provide -AvrDude with full path or add it to PATH."
}

$tempDir = Join-Path $env:TEMP ("attiny84a-fuses-{0}" -f ([Guid]::NewGuid().ToString("N")))
New-Item -ItemType Directory -Path $tempDir | Out-Null

try {
    $lfuse = Get-FuseByte -FuseName "lfuse" -AvrDudeExe $AvrDude -ProgrammerId $Programmer -PartId $Part -TempDir $tempDir
    $hfuse = Get-FuseByte -FuseName "hfuse" -AvrDudeExe $AvrDude -ProgrammerId $Programmer -PartId $Part -TempDir $tempDir
    $efuse = Get-FuseByte -FuseName "efuse" -AvrDudeExe $AvrDude -ProgrammerId $Programmer -PartId $Part -TempDir $tempDir

    Write-Host "ATtiny84A fuse report"
    Write-Host "--------------------"
    Write-Host ("LFUSE: {0}" -f (HexByte $lfuse))
    Write-Host ("HFUSE: {0}" -f (HexByte $hfuse))
    Write-Host ("EFUSE: {0}" -f (HexByte $efuse))
    Write-Host ""

    Write-Host "Low fuse (LFUSE)"
    Show-Bit -Name "CKDIV8" -Value $lfuse -Bit 7 -ActiveLow $true -Meaning "Clock divide-by-8 enabled"
    Show-Bit -Name "CKOUT" -Value $lfuse -Bit 6 -ActiveLow $true -Meaning "System clock output on CLKO enabled"
    Show-Bit -Name "SUT1" -Value $lfuse -Bit 5 -ActiveLow $false -Meaning "Start-up time select bit 1"
    Show-Bit -Name "SUT0" -Value $lfuse -Bit 4 -ActiveLow $false -Meaning "Start-up time select bit 0"
    Show-Bit -Name "CKSEL3" -Value $lfuse -Bit 3 -ActiveLow $false -Meaning "Clock source select bit 3"
    Show-Bit -Name "CKSEL2" -Value $lfuse -Bit 2 -ActiveLow $false -Meaning "Clock source select bit 2"
    Show-Bit -Name "CKSEL1" -Value $lfuse -Bit 1 -ActiveLow $false -Meaning "Clock source select bit 1"
    Show-Bit -Name "CKSEL0" -Value $lfuse -Bit 0 -ActiveLow $false -Meaning "Clock source select bit 0"
    Write-Host ("  SUT[1:0]      0b{0}" -f (BitsToBinaryString -Value $lfuse -BitOrder @(5,4)))
    Write-Host ("  CKSEL[3:0]    0b{0}" -f (BitsToBinaryString -Value $lfuse -BitOrder @(3,2,1,0)))
    Write-Host ""

    Write-Host "High fuse (HFUSE)"
    Show-Bit -Name "RSTDISBL" -Value $hfuse -Bit 7 -ActiveLow $true -Meaning "External RESET pin disabled"
    Show-Bit -Name "DWEN" -Value $hfuse -Bit 6 -ActiveLow $true -Meaning "debugWIRE enabled"
    Show-Bit -Name "SPIEN" -Value $hfuse -Bit 5 -ActiveLow $true -Meaning "ISP serial programming enabled"
    Show-Bit -Name "WDTON" -Value $hfuse -Bit 4 -ActiveLow $true -Meaning "Watchdog always on (hardware forced)"
    Show-Bit -Name "EESAVE" -Value $hfuse -Bit 3 -ActiveLow $true -Meaning "EEPROM preserved through chip erase"
    Show-Bit -Name "BODLEVEL2" -Value $hfuse -Bit 2 -ActiveLow $false -Meaning "Brown-out level select bit 2"
    Show-Bit -Name "BODLEVEL1" -Value $hfuse -Bit 1 -ActiveLow $false -Meaning "Brown-out level select bit 1"
    Show-Bit -Name "BODLEVEL0" -Value $hfuse -Bit 0 -ActiveLow $false -Meaning "Brown-out level select bit 0"
    Write-Host ("  BODLEVEL[2:0] 0b{0}" -f (BitsToBinaryString -Value $hfuse -BitOrder @(2,1,0)))
    Write-Host ""

    Write-Host "Extended fuse (EFUSE)"
    Show-Bit -Name "RES7" -Value $efuse -Bit 7 -ActiveLow $false -Meaning "Reserved (typically left as 1)"
    Show-Bit -Name "RES6" -Value $efuse -Bit 6 -ActiveLow $false -Meaning "Reserved (typically left as 1)"
    Show-Bit -Name "RES5" -Value $efuse -Bit 5 -ActiveLow $false -Meaning "Reserved (typically left as 1)"
    Show-Bit -Name "RES4" -Value $efuse -Bit 4 -ActiveLow $false -Meaning "Reserved (typically left as 1)"
    Show-Bit -Name "RES3" -Value $efuse -Bit 3 -ActiveLow $false -Meaning "Reserved (typically left as 1)"
    Show-Bit -Name "RES2" -Value $efuse -Bit 2 -ActiveLow $false -Meaning "Reserved (typically left as 1)"
    Show-Bit -Name "RES1" -Value $efuse -Bit 1 -ActiveLow $false -Meaning "Reserved (typically left as 1)"
    Show-Bit -Name "SELFPRGEN" -Value $efuse -Bit 0 -ActiveLow $true -Meaning "Self-programming (SPM) enabled"
    Write-Host ""

    Write-Host "Note: many AVR fuse features are active-low (bit = 0 means programmed/enabled)."
}
finally {
    if (Test-Path $tempDir) {
        Remove-Item -Path $tempDir -Recurse -Force
    }
}
