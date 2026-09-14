# GitHub build

The workflow `.github/workflows/build-windows.yml` uses a GitHub-hosted Windows x64
runner and MSVC, then uploads `SleapyCamera.exe` as an Actions artifact.

The official Ghost Chat project uses the same general idea: its Windows development
setup and production packaging are automated, and its Windows package is generated
from the project rather than requiring the end user to compile it.

For this repository, OBS is deliberately not configured.
