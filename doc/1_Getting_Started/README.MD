# Getting started with EIE

[TOC]

## Introduction

This module describes the software installation required for the EIE Program. All software is freeware or Free Open Source Software.

There are different install instructions for Windows, MacOS and Linux. Follow all instructions _Carefully_ to make sure your setup works.

## Github setup

Start with this before proceeding to the OS setup

1. If you don't already have one create a github account. It is strongly recommended to use a professional, but non school email for this and to link your github account on your resume.
2. Go to the [eie_nRF52840](https://github.com/eiefirmware/eie_nrf52840) repository
3. Above the about information there is a dropdown that says Fork. Click the dropdown and hit create a new fork. This creates a private copy of the repository for you to use
4. Choose a descriptive name for the repository and hit create fork in the bottom right. You can leave the name as the default eie_nrf52840 if you like

## Windows

<details>

<summary>Windows Install Instructions...</summary>

1. Open the windows terminal application. If you do not have windows terminal installed get it from the [microsoft store](https://apps.microsoft.com/detail/9n0dx20hk701)
2. Verify you have winget installed by running the command `winget --help` in the terminal. If installed help text for the Windows Package Manager should show up. If not installed install [winget](https://aka.ms/getwinget)
3. Use Winget to install dependencies `winget install Kitware.CMake Ninja-build.Ninja oss-winget.gperf python Git.Git oss-winget.dtc wget 7zip.7zip nrfutil`.
   a. You may need to add 7zip to the path
   b. type 'python --version' which should say a number >13.0
4. Download the latest [jLink installer](https://www.segger.com/downloads/jlink/)
   a. start the setup process
   b. make sure to check the "Install Legacy USB driver" checkbox
   c. finish the setup process
5. Open Git bash in the windows terminal
6. Run `nrfutil install device`
7. Change the current directory to your user directory: `cd /c/Users/<Your Username>`
8. Create a new directory for zephyr projects: `mkdir zephyr-projects`
9. Change the directory to the zephyr projects directory `cd zephyr-projects`
10. Create a python virtual environment `python -m venv .venv`
11. Activate the venv `source .venv/Scripts/activate`
12. Install west by running `pip install west`
13. Go to the repo you created during the [github setup](#github-setup) and click on the green code button and copy the HTTPS URL. The URL should look something like `https://github.com/<your account>/eie_nrf52840.git` depending on what you named the repository. **DO NOT DOWNLOAD THE CODE AS A ZIP**
14. Clone the EIE source code `west init -m <Your Repo Link Here>` filling in the link from the previous step.
15. If you run `ls` you should see your cloned repo
16. Move into the cloned repo `cd <your repo name>`
17. Update west `west update`
18. Run `west zephyr-export`to create necessary files for cmake to run
19. Install the required python packages for west `west packages pip --install`
20. Install the required toolchain `west sdk install --toolchains arm-zephyr-eabi`
21. Proceed to [Building and Flashing the Board](#building-and-flashing-the-board)
</details>

## MacOS

<details>

<summary>Mac Install Instructions...</summary>

1. Open a terminal
2. Install homebrew by running `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`
3. Add the home brew installation to the path. If you have a Apple silicon mac (2020 or newer) run `(echo; echo 'eval "$(/opt/homebrew/bin/brew shellenv)"') >> ~/.zprofile && source ~/.zprofile`. If you have an intel mac (2020 or older) run `(echo; echo 'eval "$(/usr/local/bin/brew shellenv)"') >> ~/.zprofile && source ~/.zprofile`
4. Use homebrew to install the required dependencies by running `brew install cmake ninja gperf python3 python-tk ccache qemu dtc libmagic wget openocd`
5. Add python to the path `(echo; echo 'export PATH="'$(brew --prefix)'/opt/python/libexec/bin:$PATH"') >> ~/.zprofile && source ~/.zprofile`
6. Download the latest [jLink installer](https://www.segger.com/downloads/jlink/) and run through the setup process.
7. Download [nRF utils](https://www.nordicsemi.com/Products/Development-tools/nRF-Util)
8. Change the directory in your terminal to Downloads `cd ~/Downloads`
9. Make nrfutils executable `chmod +x nrfutil`
10. Move nrfutils to your local bin `mv nrfutil ~/.local/bin/`. If this folder does not exist create it `mkdir -p ~/.local/bin` and add it to your path `echo; echo 'export $PATH=$PATH:$HOME/.local/bin' >> ~/.zprofile && source ~/.zprofile`.
11. Open a new terminal
12. Run `nrfutil install device`
13. Change the current directory to your user directory: `cd ~`
14. Create a new directory for zephyr projects: `mkdir zephyr-projects`
15. Change the directory to the zephyr projects directory `cd zephyr-projects`
16. Create a python virtual environment `python -m venv .venv`
17. Activate the venv `source .venv/bin/activate`
18. Install west by running `pip install west`
19. Go to the repo you created during the [github setup](#github-setup) and click on the green code button and copy the HTTPS URL. The URL should look something like `https://github.com/<your account>/eie_nrf52840.git` depending on what you named the repository. **DO NOT DOWNLOAD THE CODE AS A ZIP**
20. Clone the EIE source code `west init -m <Your Repo Link Here>` filling in the link from the previous step.
21. If you run `ls` you should see your cloned repo
22. Move into the cloned repo `cd <your repo name>`
23. Update west `west update`
24. Run `west zephyr-export`to create necessary files for cmake to run
25. Install the required python packages for west `west packages pip --install`
26. Install the required toolchain `west sdk install --toolchains arm-zephyr-eabi`
27. Proceed to [Building and Flashing the Board](#building-and-flashing-the-board)

</details>

## Linux

<details>

<summary>Linux Install Instructions...</summary>

> Note: This assumes Ubuntu 22.04 but you should be able to adapt this for any linux distribution

1. Run `sudo apt install --no-install-recommends git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget python3-dev python3-venv python3-tk xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1`
2. Download the latest [jLink installer](https://www.segger.com/downloads/jlink/) and run through the setup process.
3. Download [nRF utils](https://www.nordicsemi.com/Products/Development-tools/nRF-Util)
4. Change the directory in your terminal to Downloads `cd ~/Downloads`
5. Make nrfutils executable `chmod +x nrfutil`
6. Move nrfutils to your local bin `mv nrfutil ~/.local/bin/`. If this folder does not exist create it `mkdir -p ~/.local/bin` and add it to your path `echo; echo 'export $PATH=$PATH:$HOME/.local/bin' >> ~/.profile` and restart your computer.
7. Run `nrfutil install device`
8. Change the current directory to your home directory: `cd ~`
9. Create a new directory for zephyr projects: `mkdir zephyr-projects`
10. Change the directory to the zephyr projects directory `cd zephyr-projects`
11. Create a python virtual environment `python -m venv .venv`
12. Activate the venv `source .venv/bin/activate`
13. Install west by running `pip install west`
14. Go to the repo you created during the [github setup](#github-setup) and click on the green code button and copy the HTTPS URL. The URL should look something like `https://github.com/<your account>/eie_nrf52840.git` depending on what you named the repository. **DO NOT DOWNLOAD THE CODE AS A ZIP**
15. Clone the EIE source code `west init -m <Your Repo Link Here>` filling in the link from the previous step.
16. If you run `ls` you should see your cloned repo
17. Move into the cloned repo `cd <your repo name>`
18. Update west `west update`
19. Run `west zephyr-export`to create necessary files for cmake to run
20. Install the required python packages for west `west packages pip --install`
21. Install the required toolchain `west sdk install --toolchains arm-zephyr-eabi`
22. Proceed to [Building and Flashing the Board](#building-and-flashing-the-board)
</details>

## Building and Flashing the Board

1. If you don't already have a terminal open, open a terminal and navigate to the EIE project directory you cloned
2. Activate the virtual environment if not already active (see OS specific instructions for how to do so)
3. Run `west build -b nrf52840dk/nrf52840 app` to build the application
4. Run `west flash` to flash the board
