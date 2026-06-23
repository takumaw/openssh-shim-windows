# Release process

Releases are driven by Git tags.

Release notes are maintained in [HISTORY.md](../HISTORY.md).

## Version format

Git tags use a leading `v`:

```text
v0.1.0
```

`HISTORY.md` headings do not use a leading `v`:

```markdown
## 0.1.0
```

## Release assets

A release contains exactly these assets:

```text
openssh-shim-windows-vX.Y.Z-x86_64-pc-windows-msvc.zip
SHA256SUMS.txt
```

Do not upload standalone `ssh.exe` or `scp.exe` assets.

The zip contains:

```text
ssh.exe
scp.exe
README.md
LICENSE.txt
```

`SHA256SUMS.txt` contains the SHA-256 hash for the zip asset.

## Pre-release checklist

Before creating a tag:

* `README.md` is current.
* `LICENSE.txt` is present.
* `HISTORY.md` contains the release entry.
* The `HISTORY.md` heading is exactly `## X.Y.Z`.
* `src/ssh.rc` and `src/scp.rc` have their version fields updated to `X,Y,Z,0` and `"X.Y.Z.0"`.
* CI passes on `main`.
* The local release build and smoke test pass if run manually.

## Creating a release

To trigger a release (e.g., `v0.1.0`), commit the version updates and push a signed Git tag:

```powershell
# 1. Update files, commit changes
git add HISTORY.md README.md src/ssh.rc src/scp.rc
git commit -m "Prepare v0.1.0 release"

# 2. Create a signed tag
git tag -s v0.1.0 -m "Release v0.1.0"

# 3. Push main branch and the tag
git push origin main
git push origin v0.1.0
```

The release workflow is triggered by pushing the tag.

## Workflow behavior

The release workflow:

1. Checks out the repository.
2. Extracts the version from the tag.
3. Extracts the matching `HISTORY.md` section.
4. Configures CMake for x64 MSVC.
5. Builds `ssh.exe` and `scp.exe`.
6. Runs the smoke test.
7. Creates `openssh-shim-windows-vX.Y.Z-x86_64-pc-windows-msvc.zip`.
8. Generates `SHA256SUMS.txt`.
9. Creates a GitHub Release.
10. Uploads the zip and `SHA256SUMS.txt`.

## Failure cases

The workflow should fail if:

* The tag does not match `vX.Y.Z`.
* The matching `HISTORY.md` section is missing.
* The extracted release notes are empty.
* Build fails.
* Smoke test fails.
* The GitHub Release already exists.

If the workflow fails before creating a release, fix the issue and recreate or move the tag as needed.

If the workflow fails after creating a release, delete the failed release and tag before retrying, or repair the release manually.
