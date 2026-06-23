# openssh-shim-windows

Windows OpenSSH shims for VS Code Remote-SSH.

This project provides Windows-native `ssh.exe` and `scp.exe` shims that launch Windows OpenSSH while preserving command-line arguments, standard streams, the current directory, the environment, and exit codes.

The primary use case is VS Code Remote-SSH on Windows environments where launching Windows OpenSSH directly from VS Code can be affected by compatibility issues with file-protection products such as NEC InfoCage FileShell.

## What it installs

Install both files in the same directory:

```text
ssh.exe
scp.exe
```

VS Code Remote-SSH may look for `scp.exe` next to the configured SSH executable when it needs to transfer the VS Code server archive from the local client.

## Manual installation

1. Open the latest release:

   <https://github.com/takumaw/openssh-shim-windows/releases/latest>

2. Download the release asset named like this:

   ```text
   openssh-shim-windows-vX.Y.Z-x86_64-pc-windows-msvc.zip
   ```

3. Extract the zip file to:

   ```text
   %LOCALAPPDATA%\Programs\openssh-shim-windows\
   ```

   For example:

   ```text
   C:\Users\<you>\AppData\Local\Programs\openssh-shim-windows\
   ```

4. Confirm that both files exist:

   ```text
   C:\Users\<you>\AppData\Local\Programs\openssh-shim-windows\ssh.exe
   C:\Users\<you>\AppData\Local\Programs\openssh-shim-windows\scp.exe
   ```

## Script installation

This PowerShell script downloads the latest versioned zip asset from GitHub Releases and extracts it to the recommended per-user install directory.

```powershell
$InstallDir = Join-Path $env:LOCALAPPDATA "Programs\openssh-shim-windows"
$TempZip = Join-Path $env:TEMP "openssh-shim-windows.zip"
$ApiUrl = "https://api.github.com/repos/takumaw/openssh-shim-windows/releases/latest"

New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null

$Release = Invoke-RestMethod -Uri $ApiUrl
$Asset = $Release.assets |
    Where-Object { $_.name -like "openssh-shim-windows-v*-x86_64-pc-windows-msvc.zip" } |
    Select-Object -First 1

if (-not $Asset) {
    throw "Release asset not found."
}

Invoke-WebRequest -Uri $Asset.browser_download_url -OutFile $TempZip
Expand-Archive -Path $TempZip -DestinationPath $InstallDir -Force
Remove-Item $TempZip -Force
```

## VS Code configuration

Set `remote.SSH.path` to the installed `ssh.exe` shim.

```json
{
  "remote.SSH.path": "C:\\Users\\<you>\\AppData\\Local\\Programs\\openssh-shim-windows\\ssh.exe"
}
```

Do not set it to `scp.exe`. VS Code Remote-SSH discovers `scp.exe` automatically from the directory containing the configured SSH executable.

## Behavior

`ssh.exe` launches:

```text
C:\Windows\System32\OpenSSH\ssh.exe
```

`scp.exe` launches:

```text
C:\Windows\System32\OpenSSH\scp.exe
```

Both shims pass the original command-line tail through unchanged. They do not parse SSH or SCP options, do not rewrite arguments, do not search `PATH`, do not write logs, and do not print wrapper-specific messages.

## Development

See [CONTRIBUTING.md](CONTRIBUTING.md), [docs/DESIGN.md](docs/DESIGN.md), and [docs/RELEASE.md](docs/RELEASE.md).

## License

Apache-2.0. See [LICENSE.txt](LICENSE.txt).

(C) 2026 Takuma WATANABE
