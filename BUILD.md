# Building

## Easiest: GitHub Actions (builds Windows, macOS, iOS, Android)
1. Create a new GitHub repo and push this folder to it.
2. Open the repo's **Actions** tab and wait for "Build Binaries" to finish (or run it manually).
3. Download the **Build Output** artifact. It contains the `.geode` file.

## Locally (Windows example)
1. Install the Geode CLI and SDK: https://docs.geode-sdk.org/getting-started/
2. `geode build` in this folder. The `.geode` file lands in `build/`.

Before releasing, edit `developer` and `id` in `mod.json` (the id must be `yourname.modname`, lowercase).
Disable/remove the original "Better Percentage" mod so the two don't both edit the label.
