# CONTRIBUTING

This project is intentionally small. Keep changes narrow, transparent, and easy to audit.

## Requirements

Development targets Windows x64 with MSVC.

Required tools:

* Windows 10 x64 or Windows 11 x64
* Visual Studio 2022 Build Tools, including MSVC
* CMake
* PowerShell

## Build

From a Developer PowerShell or a shell where MSVC and CMake are available:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

Expected outputs:

```text
build\Release\ssh.exe
build\Release\scp.exe
```

## Smoke test

```powershell
powershell -ExecutionPolicy Bypass -File tests\smoke.ps1 -BuildDir build\Release
```

The smoke test verifies that:

* `ssh.exe -V` can launch Windows OpenSSH and returns exit code `0`.
* `scp.exe` is launchable. Its exit code is not required to be `0` because OpenSSH `scp.exe` may print usage and exit non-zero when called without operands.

## Source layout

```text
src/
  shim_core.h
  shim_core.c
  ssh_main.c
  scp_main.c
```

`ssh_main.c` and `scp_main.c` are separate entry points. They currently differ only by target path. Shared process behavior belongs in `shim_core.c`.

## Implementation rules

Do not implement SSH or SCP.

Do not parse SSH or SCP options.

Do not handle `-V`, `--version`, or any other option specially.

Do not search `PATH`.

Do not call `cmd.exe`, PowerShell, `ShellExecute`, or `system()`.

Do not use `argv` to reconstruct the command line.

Do not write logs.

Do not print wrapper-specific messages to stdout or stderr.

Do not add environment variables.

Do not add configuration files.

Do not add a target override mechanism.

Do not support Git for Windows OpenSSH in this repository.

Do not support WSL or WSL2 in this repository.

`scp.exe` must pass arguments through unchanged. Do not inject `-S` or any other SCP option unless a future design change explicitly documents that behavior.

## Documentation

Design changes should update [docs/DESIGN.md](docs/DESIGN.md).

Release process changes should update [docs/RELEASE.md](docs/RELEASE.md).
