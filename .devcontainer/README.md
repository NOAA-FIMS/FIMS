# :package: FIMS devcontainer cheat sheet

This cheat sheet outlines the steps for setting up a consistent FIMS environment. The approach bypasses manual dependency installation and keeps everyone's setup the same.

Please follow the section below that matches how you plan to open the container, i.e., follow the 🌐[GitHub Codespaces section](#-github-codespaces) if you wish to open the container using Codespaces from your favorite web browser and the 🐧[Windows Subsystem for Linux 2 (WSL2) section](#-wsl2) if you wish to launch the container on your own machine.

## 🌐 GitHub Codespaces

To open the container in a Codespace using the internet browser of your choice please follow the instructions below. You can navigate there directly by clicking on [this link](https://github.com/codespaces/new?hide_repo_select=true&ref=main&repo=452012004&skip_quickstart=true&machine=standardLinux32gb&devcontainer_path=.devcontainer%2Fuser%2Fdevcontainer.json&geo=UsEast) or follow the manual steps below. 

- Click the green `Code` button on the [FIMS repository](https://github.com/NOAA-FIMS/FIMS).
- Click the `...` (three dots) next to the `+` icon in the Codespaces tab.
- Select `New with options ...`.
- Under the dropdown menu labeled `Dev container configuration`, select `FIMS User` or `FIMS Developer`, depending on your role.
- If you plan to run a FIMS model, change the `Machine type` from the default `2-core` to at least `4-core`. The default `2-core` will not allow you to compile {FIMS} and will lead to your R session unexpectedly crashing but it is set as the default because we pay for each minute used in Codespaces whether the session is actively being used or sitting idle and 2 cores is much cheaper.
- Click the green `Create codespace` button. 

## 🐧 WSL2

To open the container on your local Windows machine using Windows Subsystem for Linux 2 (WSL2), Docker Engine, and Visual Studio Code (VS Code) use the following instructions: 

### Connect VS Code to WSL

- Open VS Code and press `Ctrl + Shift + P` to open the command palette.
- Type and select `WSL: Connect to WSL using Distro`.
- Choose `Ubuntu` from the list.

### Clone FIMS repository (first time only)

- Once connected to WSL, open a new terminal in VS Code.
- Clone the project repository inside the WSL2 file system.
```bash
git clone https://github.com/NOAA-FIMS/FIMS.git
```

> **Note:** For other FIMS-related projects, feel free to use the [example](https://github.com/NOAA-FIMS/FIMS/tree/main/.devcontainer/user) in the `FIMS/.devcontainer/user` folder as a template to set up similar devcontainer environments for your own repositories.

### Open FIMS in Dev Container

- In VS Code and press `Ctrl + O` to open the FIMS Folder you just cloned. 
- Press `Ctrl + Shift + P` to open the command palette.
- Type and select `Dev Containers: Reopen in Container`.
- Select `FIMS User` if you want to run stock assessments using FIMS. Select `FIMS Developer` if you are contributing to the source code or maintaining the FIMS repository.
- Docker will now build the container. The first build can take several minutes for a `FIMS developer` setup and about 15 minutes for a `FIMS user` setup.

### Close the container

When you're finished, open the Command Palette (`Ctrl + Shift +P`) and type `Remote: Close Remote Connection`.

## 🙋 Need help?

If any of the instructions above do not work as expected, please [report an issue on GitHub](https://github.com/NOAA-FIMS/FIMS/issues).
