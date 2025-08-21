# :package: FIMS devcontainer cheat sheet

This cheat sheet outlines the steps for setting up a consistent FIMS development environment on a Windows machine using Windows Subsystem for Linux 2 (WSL2), Docker Engine, and Visual Studio Code (VS Code). By using a Docker image, this approach bypasses manual dependency installation and keeps everyone's setup the same.

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

- In VS Code and press `Ctrl + O` to open the FIMS Folder you just cloned. 
- Press `Ctrl + Shift + P` to open the command palette.
- Type and select `Dev Containers: Reopen in Container`.
- Docker will now build the container. The first build can take several minutes.

## :door: Close the WSL Dev Container connection

When you're finished, open the Command Palette (`Ctrl + Shift +P`) and type `Remote: Close Remote Connection`.

## :raising_hand: Need help?

If any of the instructions above do not work as expected, please
[report an issue on GitHub](https://github.com/NOAA-FIMS/FIMS/issues).
