<p>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset=".github/night.svg">
    <img alt="Throwback Loader" src=".github/day.svg" height="42">
  </picture> <br>
  It launches any older season of Rainbow Six Siege. This is a further development of https://github.com/lungu19/ThrowbackLoader. Not affiliated with, endorsed by, or sponsored by Ubisoft.
</p>

**Installation**

- Download the latest [release](https://github.com/xeralin/ThrowbackLoader/releases/latest) and extract it
- Move all the files into your game folder
- Run `LaunchR6.exe`

**Editing `Config.toml`**

- `username` - your in-game name, up to 16 characters
- `user_id` - derived from the game folder when empty
- `args` - launch arguments passed to the game
- `autorun` - extra programs to run alongside the game

**Building**

You need CMake, Ninja, zip and the mingw-w64 cross-compiler.

```sh
git clone https://github.com/xeralin/ThrowbackLoader.git
cd ThrowbackLoader
bash build.sh
```

<p>
  <a href="https://github.com/xeralin/ThrowbackLoader/releases/latest"><img alt="latest release" src="https://img.shields.io/github/v/release/xeralin/ThrowbackLoader?style=flat&color=c0152a" /></a>
  <a href="https://github.com/xeralin/ThrowbackLoader/releases"><img alt="downloads" src="https://img.shields.io/github/downloads/xeralin/ThrowbackLoader/total?style=flat&color=c0152a" /></a>
  <a href="https://discord.gg/r6s-operation-throwback-2-0-1092820800203141130"><img alt="discord" src="https://img.shields.io/discord/1092820800203141130?style=flat&label=discord&color=e8e0d5" /></a>
</p>
