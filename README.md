# Overview

This project is a command line tool for applying video effects.
The tool is built using C and utilizes the FFmpeg library for video processing.

# Quick Start

1. Install the [build prerequisites](#build-prerequisites) on your system
2. Clone this repository
3. `cd video-effects/`
4. `autoreconf -if`
5. `./configure`
6. `make`
    - Run `make` again to rebuild the project after making changes to the source code
    - If you want to clean the project see [Cleaning the Project](#cleaning-the-project)
7. `cd src/`
    - This is where the executable is located
    - If you are using CLion, the executable will be located in the `build/src` folder
8. `./video_effects --help` to check if the build was successful
    - If you see the help message, the build was successful

## Usage

#### General Usage
```sh
./video_effects -i <input_file> -o <output_file> -f <effect_id>
```

#### Example
```sh
./video_effects -i input.mp4 -o output.mp4 -f 1
```
This would apply the effect with ID 1 to the input file `input.mp4` and save the output to `output.mp4`.

## Build prerequisites

General requirements

1. GCC (GNU Compiler Collection)
2. GNU Autotools
3. pkgconf
4. FFmpeg
5. Argp
    - Preinstalled on Linux systems, for macOS installation see [macOS](#macos)

### Linux

#### Ubuntu / Debian
```sh
sudo apt update
sudo apt upgrade
sudo apt install git build-essential autoconf automake pkgconf ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
```
```

### macOS

#### Install 

1. Install the **Command Line Developer Tools** for **Xcode**
```sh
xcode-select --install
```

2. Install **Homebrew**
```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
3. Update, Upgrade and Cleanup **Homebrew**
```sh
brew update && brew upgrade && brew cleanup
```
4. Install the **build dependencies**
```sh
brew install git gcc autotools autoconf pkgconf argp-standalone ffmpeg
```

5. Create a symbolic link for Homebrew
    - This is required for the `./configure` script to find the libraries
    - Use `ls -l /usr/local` to check if the symbolic link already exists or if it was successfully created
```sh
sudo mkdir -p /usr/local/include
sudo mkdir -p /usr/local/lib
sudo ln -s /opt/homebrew/include /usr/local/include
sudo ln -s /opt/homebrew/lib /usr/local/lib
```

### Windows

Windows is currently **not** supported.

## Cleaning the Project

- `make mostlyclean`
  - Removes all `.o` object files in `src/`

- `make clean`
  - Removes all generated files including:
    - Object files (`.o`)
    - The executable
    - Generated makefiles
    - Configuration files
