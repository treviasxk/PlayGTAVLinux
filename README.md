# Play GTA V Linux

A lightweight executable wrapper designed to improve the experience of running Rockstar Games titles through the Epic Games Launcher on Linux.

PlayGTAVLinux provides a compatibility layer that intercepts the game's launch process, initializes the Rockstar Games Launcher, and starts the original game with the required Epic Games integration parameters.

![Preview](Screenshots/banner.png)

## Supported Games

- **Grand Theft Auto V** — Legacy and Enhanced editions.
- **[Red Dead Redemption 2](https://github.com/treviasxk/PlayRDR2Linux)** — See the dedicated PlayRDR2Linux project.

## Features

- Integrates with the Epic Games Launcher on Linux.
- Supports Epic Games Launcher Flatpak and Heroic Games Launcher.
- Initializes the Rockstar Games Launcher before starting GTA V Enhanced.
- Passes the original Epic Games launch arguments to the game.
- Optional startup console and diagnostic logs.

## Installation

### 1. Download

Go to the [Releases page](https://github.com/treviasxk/PlayGTAVLinux/releases) and download the `PlayGTAV.zip` archive.

### 2. Install the wrapper

Extract the ZIP archive into the root directory of your GTA V installation.

The files should look like this:

```text
GTAV/
├── PlayGTAVLinux.exe
├── EpicGamesLauncher.exe
├── PlayGTAV.exe
├── GTA5_Enhanced_BE.exe
└── ...
```

Make sure both wrapper executables are in the same directory as the original game files.


### 3. Configure your launcher

Choose the launcher you use and follow the corresponding instructions.

#### Epic Games Launcher (Flatpak)

The Epic Games Launcher Flatpak stores game installation information in:

```text
~/.var/app/com.epicgames.launcher/data/com.epicgames.eglv2/installations.jsonl
```

1. Completely close the Epic Games Launcher.
2. Open `installations.jsonl` with a text editor.
3. Find the GTA V installation entry. Check its `dir` field to identify the correct game directory.
4. In the corresponding installation entry, change the `launchExecutable` property to:

```json
"launchExecutable": "PlayGTAVLinux.exe"
```

5. Save the file and reopen the Epic Games Launcher.

**Important:** `installations.jsonl` uses JSON Lines format, with one JSON object per line. Preserve the existing entries and fields. If multiple records exist for the same installation, make sure the most recent record also has `PlayGTAVLinux.exe` as its launch executable.

#### Heroic Games Launcher

1. Open Heroic Games Launcher.
2. Select Grand Theft Auto V.
3. Open **Settings → Advanced**.
4. Find **Select Alternative EXE**.
5. Select `PlayGTAVLinux.exe` inside your GTA V installation directory.

### 4. Launch the game

Start GTA V normally through your configured launcher.

The wrapper will automatically:

## Command Line

### Show logs

By default, the wrapper runs without displaying a console window.

To enable the console and display diagnostic information, add:

```bash
-logs
```

## Build from Source

If you want to compile the project yourself instead of downloading a pre-built release, follow these steps.

### 1. Install dependencies

On Arch Linux:

```bash
sudo pacman -S git cmake mingw-w64-gcc
```

### 2. Clone the repository

```bash
git clone https://github.com/treviasxk/PlayGTAVLinux.git
cd PlayGTAVLinux
```

### 3. Build

Configure the Windows cross-compilation build using the project's CMake toolchain file:

```bash
cmake -S play -B build -DCMAKE_TOOLCHAIN_FILE=windows.cmake
```

Compile the project:

```bash
cmake --build build --config Release
```

The generated Windows executables can then be used in the corresponding game directories.

## Requirements

- GE-Proton version 11-X
- Epic Games Launcher (Flatpak) or Heroic Games Launcher
- A working GTA V installation

## Contributing

Contributions, bug reports, and suggestions are welcome.

Feel free to open an issue or submit a pull request.

## Donate

If you enjoy this project or find it useful, consider supporting its development.

All donation methods are available on the official donation page:

**[Support this project](https://treviasxk.devs.surf/donate)**

Every contribution is appreciated and helps support continued development.