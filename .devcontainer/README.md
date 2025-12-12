# :package: FIMS devcontainer cheat sheet

This cheat sheet outlines the steps for setting up a consistent FIMS environment.
The approach bypasses manual dependency installation and keeps everyone's setup the same.
Please follow the section that matches how you plan to open the container.

- :globe_with_meridians: Open the container using a Codescpace from the browser
- :technologist: Open the container on a Windows machine using Windows Subsystem
  for Linux 2 (WSL2), Docker Engine, and Visual Studio Code (VS Code)

## :globe_with_meridians: Open the container using a Codescpace from the browser

- Click the green `Code` button on the FIMS repo.
- Click the `...` (three dots) next to the `+` icon in the Codespaces tab.
- Select `New with options ...`.
- A dropdown labeled `Dev container configuration` will appear.
- Select `FIMS User` or `FIMS Developer`, depending on your role.
- If you plan to run a FIMS model, change the `Machine type` from the default `2-core` to `16-core`.
- Click the green `Create codespace` button. 

## :technologist: Open the container on a Windows machine using WSL2, Docker Engine, and VS Code

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
- :warning: Important: **A menu will appear at the top. Select `FIMS User` if you
  want to run stock assessments using FIMS. Select `FIMS Developer` if you are
  contributing to the source code or maintaining the FIMS repository.**
- Docker will now build the container. The first build can take several minutes.

### :door: Close the WSL Dev Container connection

When you're finished, open the Command Palette (`Ctrl + Shift +P`) and type `Remote: Close Remote Connection`.

## :raising_hand: Need help?

If any of the instructions above do not work as expected, please
[report an issue on GitHub](https://github.com/NOAA-FIMS/FIMS/issues).
