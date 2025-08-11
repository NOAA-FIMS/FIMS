# :package: FIMS devcontainer cheat sheet

This cheat sheet outlines the steps for setting up a consistent FIMS development
environment on a Windows machine using Windows Subsystem for Linux 2 (WSL2),
Docker Engine, and Visual Studio Code (VS Code). This approach bypasses manual 
dependency installation and keeps everyone's setup the same.

## :hammer: Install WSL2 and Docker Engine

- Install WSL2 and Docker Engine on the Windows machine. Both tools are on the
[aprroved software list](https://docs.google.com/spreadsheets/d/1oPaTegdBGEmkjrmkbOVjGAJpPFg755p3Wws50ddLwCI/edit?usp=sharing).
  - For installation details, see the official
  [WSL documentation](https://learn.microsoft.com/en-us/windows/wsl/install)
  and [Docker documentation](https://docs.docker.com/engine/install/).

## :hammer: Install VS Code and extensions

Open VS Code and install the following extensions from the Extensions view
(`Ctrl + Shift + X`):
- WSL extension (ID: ms-vscode-remote.remote-wsl): allows VS Code to
connect to the WLS2 envionment.
- Dev Containers extention (ID: ms-vscode-remote.remote-containers): builds and
manages development containers from the `.devcontainers/devcontainer.json` file.

## :hammer: Install Ubuntu

- Open a Windows Command Prompt and check if `Ubuntu` is listed as a WSL
distribution:
```bash
wsl --list --verbose
```

If `Ubuntu` is not listed under the `NAME` column, run the following command
to install it:
Ubuntu: 
```Bash
wsl --install -d Ubuntu
```

## :rocket: Launch the FIMS environment

### Connect VS Code to WSL

- Open VS Code and press `Ctrl + Shift + P` to open the command palette.
- Type and select `WSL: Connect to WSL using Distro`.
- Choose `Ubuntu` from the list.

### Clone FIMS repo (first time only)

- Once connected to WSL, open a new terminal in VS Code.
- Clone the project repository:
```bash
git clone https://github.com/NOAA-FIMS/FIMS.git
```

### Open FIMS in Dev Container

- In VS Code, go to the `File > Open Folder...` and open the FIMS Folder you just
cloned. 
- Press `Ctrl + Shift + P` to open the command palette.
- Type and select `Dev Containers: Reopen in Container`.
- Docker will now build the container. The first build can take several minutes.

## :door: Close the WSL Dev Container connection

When you're finished, open the Command Palette (`Ctrl + Shift +P`) and
type `Remote: Close Remote Connection`.

## :raising_hand: Need help?

If any of the instructions above do not work as expected, please
[report an issue on GitHub](https://github.com/NOAA-FIMS/FIMS/issues).
