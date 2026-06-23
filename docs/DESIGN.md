# Design

## Goal

`openssh-shim-windows` provides a pair of Windows-native process shims for VS Code Remote-SSH:

```text
ssh.exe -> C:\Windows\System32\OpenSSH\ssh.exe
scp.exe -> C:\Windows\System32\OpenSSH\scp.exe
```

The shims create an extra process boundary between VS Code and Windows OpenSSH while keeping command-line arguments, standard streams, the current directory, the environment, and exit codes transparent.

## Non-goals

This project does not implement SSH or SCP.

This project does not provide key management, SSH agent proxying, SSH config parsing, Git for Windows OpenSSH support, WSL support, WSL2 support, logging, installer behavior, target override configuration, or PATH-based target discovery.

## Why both ssh.exe and scp.exe exist

VS Code Remote-SSH can use `scp.exe` to transfer the VS Code server archive from the local client when remote-side download fails or when a local download path is selected.

When that path is used, Remote-SSH may look for `scp.exe` next to the configured SSH executable. Therefore the installed shim directory must contain both:

```text
ssh.exe
scp.exe
```

## Entry points

The project uses a shared core and separate entry points:

```text
src/
  shim_core.h
  shim_core.c
  ssh_main.c
  scp_main.c
```

`ssh_main.c` calls the shared core with:

```text
C:\Windows\System32\OpenSSH\ssh.exe
```

`scp_main.c` calls the shared core with:

```text
C:\Windows\System32\OpenSSH\scp.exe
```

The separate entry points make future `ssh.exe`-specific and `scp.exe`-specific changes easier to isolate while keeping the process-launching logic shared.

## Command line handling

The shim must not reconstruct the command line from `argv`.

Windows process creation receives a single command-line string. `argv` is already the result of parsing by the process runtime. Splitting and re-quoting `argv` can change the meaning of quotes, backslashes, empty arguments, spaces, and paths.

The shim must:

1. Call `GetCommandLineW()`.
2. Remove only the first token, which is the shim executable path.
3. Preserve the remaining command-line tail unchanged.
4. Build the child command line as:

   ```text
   "C:\Windows\System32\OpenSSH\<tool>.exe" <original tail>
   ```

For `scp.exe`, the same rule applies. The shim must not inject `-S`, rewrite remote paths, or modify SCP arguments.

## Process creation

The shim uses `CreateProcessW` directly.

It must not use a shell. `cmd.exe`, PowerShell, `ShellExecute`, and `system()` are out of scope.

Recommended `CreateProcessW` settings:

```text
lpApplicationName    = C:\Windows\System32\OpenSSH\<tool>.exe
lpCommandLine        = mutable command line buffer
lpEnvironment        = NULL
lpCurrentDirectory   = NULL
bInheritHandles      = TRUE
dwCreationFlags      = CREATE_SUSPENDED
```

`lpEnvironment = NULL` preserves the parent environment.

`lpCurrentDirectory = NULL` preserves the parent current directory.

## Standard streams

The shim must preserve standard input, output, and error.

Remote-SSH may pipe a generated shell script into SSH, for example:

```text
type script.sh | ssh.exe -T -D <port> <host> sh
```

Therefore the shim uses `STARTUPINFOW`, sets `STARTF_USESTDHANDLES`, copies the handles returned by `GetStdHandle`, and starts the child with `bInheritHandles = TRUE`.

## Job Object

The shim uses a Job Object with `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`.

This ensures that if the shim exits unexpectedly, the child OpenSSH process is not left orphaned.

Process flow:

```text
CreateJobObjectW
SetInformationJobObject(JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE)
CreateProcessW(... CREATE_SUSPENDED ...)
AssignProcessToJobObject
ResumeThread
WaitForSingleObject
GetExitCodeProcess
ExitProcess(child_exit_code)
```

If job setup fails before child creation, the shim exits with `255`.

If assigning or resuming fails after child creation, the shim terminates the child and exits with `255`.

## Exit codes

If the target OpenSSH process starts successfully and exits, the shim returns the target process exit code unchanged.

If the shim itself fails before or during launch, it returns `255`.

`255` is used because OpenSSH commonly uses `255` for SSH-level errors.

## Silence

The shim itself must not print to stdout or stderr.

Output from the launched Windows OpenSSH process is preserved. For example, `ssh.exe -V` should print the Windows OpenSSH version because the child `ssh.exe` prints it, not because the shim handles `-V` specially.
