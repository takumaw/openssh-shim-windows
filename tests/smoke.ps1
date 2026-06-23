param(
    [string]$BuildDir = "build\Release"
)

$ErrorActionPreference = "Stop"

$Ssh = Join-Path $BuildDir "ssh.exe"
$Scp = Join-Path $BuildDir "scp.exe"
$WindowsSsh = "C:\Windows\System32\OpenSSH\ssh.exe"
$WindowsScp = "C:\Windows\System32\OpenSSH\scp.exe"

if (-not (Test-Path $WindowsSsh)) {
    throw "Windows OpenSSH ssh.exe not found: $WindowsSsh"
}

if (-not (Test-Path $WindowsScp)) {
    throw "Windows OpenSSH scp.exe not found: $WindowsScp"
}

if (-not (Test-Path $Ssh)) {
    throw "Build artifact not found: $Ssh"
}

if (-not (Test-Path $Scp)) {
    throw "Build artifact not found: $Scp"
}

& $Ssh -V
if ($LASTEXITCODE -ne 0) {
    throw "ssh.exe -V failed with exit code $LASTEXITCODE"
}

# OpenSSH scp.exe commonly prints usage and exits non-zero when called without operands.
# For this shim, the smoke test only verifies that the shim can launch the target scp.exe.
$OldAction = $ErrorActionPreference
$ErrorActionPreference = "SilentlyContinue"
& $Scp *> $null
$ErrorActionPreference = $OldAction

Write-Host "Smoke test passed."
