# FIMS User Setup Guide

Expand All Collapse All

## Overview

This guide helps FIMS users set up a standardized Fisheries Integrated
Modeling System (FIMS) environment. By using [cloud-based
workstations](https://cloud.google.com/workstations?hl=en) and
pre-configured software setups
([containers](https://www.ibm.com/think/topics/containers)), we make
sure the system works the same for everyone and remove the need for you
to manually install the dependencies FIMS requires.

Please follow the section below that matches your preferred setup:

| Option                                                         | Best For                                                                           | Where it Runs | Setup Effort                                        | Internet Needed |
|:---------------------------------------------------------------|:-----------------------------------------------------------------------------------|:--------------|:----------------------------------------------------|:----------------|
| ☁️ **[Google Cloud Workstations](#google-cloud-workstations)** | **NOAA-internal users** interested in using Google Cloud Workstations.             | Cloud Browser | **Moderate** (requires running a one-line command)  | **Always**      |
| 🌐 **[GitHub Codespaces](#github-codespaces)**                 | Anyone with a **GitHub account** who wants a fast, browser-based environment.      | Browser       | **Minimal**                                         | **Always**      |
| 🐧 **[WSL2](#wsl2)**                                           | Windows users who want to run the **FIMS container** in a local Linux environment. | Local Machine | **Moderate** to **Complex** (if WSL2 not installed) | **Setup only**  |
| 🧑‍💻 **[Local](#local)**                                         | **Everyone else** installing packages directly on their own operating system.      | Local Machine | **Moderate** (requires running a one-line command)  | **Setup only**  |

## ☁️ Google Cloud Workstations

NOAA Fisheries Google Cloud Workstations provide a managed RStudio, Code
OSS (the open source version of Visual Studio Code), or Positron
environment. For detailed visual guides (e.g., snapshots) on starting,
stopping, or managing workstations, please refer to the [NOAA-internal
documentation](https://docs.google.com/document/d/1nziPdPULoRWOYQ9WKzISNUgJvANACvfYpFr1z3Ro2Bc/edit?tab=t.0#heading=h.5509095bxzv7).

- **NOAA internal only:** To request access to the NOAA Fisheries Cloud
  Compute Accelerator Program, please follow the steps
  [here](https://docs.google.com/document/d/1nziPdPULoRWOYQ9WKzISNUgJvANACvfYpFr1z3Ro2Bc/edit?tab=t.0#heading=h.adsdqttdaso0)
  to submit a [request
  form](https://docs.google.com/forms/d/e/1FAIpQLSc-RSmPhLV7kBuiiuAzxb2LvWG7Q6XrNbQCbhJZtvaVVtOVZQ/viewform?usp=sharing&ouid=112419493802230683932).

- Navigate to the NOAA Fisheries [Google Cloud Workstations
  Console](https://console.cloud.google.com/workstations/list?project=ggn-nmfs-wsent-prod-1)
  and click `+ create workstation`.

- Provide a name for the workstation, select a configuration, and click
  the `Create` button.

> **Note:** Configurations with low core counts (`*-small`) will lead to
> failed compilation of FIMS due to insufficient memory. Use
> `posit-medium` or `posit-large` for workshops and rapid testing, as
> they have faster startup times and multiple IDE options. The
> `nmfs-rstudio-medium` or `nmfs-rstudio-large` options can also be used
> by users who prefer to work in RStudio.

- Click the `Start` and `Launch` buttons to initialize the workstation.
  It is normal for a workstation to take a minute or two to spin up.

- Add a new session from either `Positron Pro`, `RStudio Pro`, or
  `VS Code` when using `posit-*` configurations, and then `Launch` the
  session.

- Open a [bash
  terminal](https://www.atlassian.com/git/tutorials/git-bash) within
  your session and copy/paste this command below. This will
  automatically set up the workstation by installing all necessary
  system and R dependencies.

``` bash
bash <(curl -sL https://raw.githubusercontent.com/NOAA-FIMS/FIMS/main/setup_fims.sh)
```

> **Note:** The dependency installation process typically takes 10
> minutes. If using `Positron Pro`, a restart of the R session is
> required to initialize the new libraries.
>
> Please run [`library(FIMS)`](https://github.com/noaa-fims/fims) in the
> R session to confirm the successful installation of FIMS. If the
> installation is successful, no messages or errors will appear. If it
> is unsuccessful, you will see an error such as:
> `Error in library(FIMS) : there is no package called 'FIMS'`.
>
> Alternatively, you can run the R script below to perform a full FIMS
> model run.

- To verify that FIMS is ready, run the R code below to download the
  FIMS demo R script:

``` r

knitr::purl(
  input = 'https://raw.githubusercontent.com/NOAA-FIMS/FIMS/refs/heads/main/vignettes/fims-demo.Rmd',
  output = 'fims_demo.R',
  documentation = 0
)

source('fims_demo.R')
```

- To stop a workstation, close the workstation page and go to the
  `Overview` page of the workstations user interface to click the `■`
  (stop) button. Stopping a workstation does not delete it, ensuring
  that files and settings remain intact.

🤿 Deep dive: R code to connect GitHub to a Google Cloud Workstation
(Click to expand)

A Personal Access Token (PAT) can be used to link a workstation to a
GitHub Enterprise Account. Please follow GitHub’s instructions ([PAT
settings](https://github.com/settings/tokens) and
[tutorial](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens))
to generate a PAT. Then, use the R code below from
[`nmfs-opensci/CloudComputingSetup`](https://github.com/nmfs-opensci/CloudComputingSetup)
to link a workstation to a GitHub Enterprise account.

``` bash
# R code to connect github to a Google Cloud Workstation
# Author: Alexandra Norelli

# Go to Github and generate a Personal Access Token (PAT) with the permissions 
# you need:  
# https://github.com/settings/tokens
# Github tutorial here: 
# https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens
# Github Governaance Team Video tutorial here:  
# https://drive.google.com/file/d/1tbbw_xXARK689Zj5tm4lVo18aBaXhdKX/view?t=4

# ctrl+f and replace USER_NAME with your username
# ctrl+f and replace E_MAIL with your email address
# ctrl+f and replace TOKEN with your PAT Token

system('git config --global user.name "USER_NAME"')
system('git config --global user.email "E_MAIL"')
system("git config --list")  #check that the info is correct

cred_line <- "https://USER_NAME:TOKEN@github.com"
writeLines(cred_line, "~/.git-credentials")
system('git config --global credential.helper store')
```

🤿 Deep dive: Terminal code for mounting Google Cloud Storage Buckets
(Click to expand)

To mount a Google Cloud Storage Bucket, which makes the cloud storage
act like a local folder on the Google Cloud Workstation for easy reading
and writing of data, use the R example below from
[`nmfs-opensci/CloudComputingSetup`](https://github.com/nmfs-opensci/CloudComputingSetup).

``` bash
# Terminal code for mounting Google Cloud Buckets to Google Cloud Workstations
# Author: Alexandra Norelli with Gemini assistance for R to BASH translations.  
# Paste all of this code in the terminal and follow the instructions to 
# authenticate your google account.  

# Define your bucket and the mount point.
# replace "nmfs-opensci" with your bucket name
# The `$HOME` symbol is a shortcut for your home directory, so this will
# create the folder at /home/your_username/my_gcs_bucket.
BUCKET_NAME="nmfs-opensci"
MOUNT_POINT="$HOME/my_gcs_bucket"

# --- Authentication ---
# This command authenticates your user account with Google Cloud.
# You'll be prompted to open a browser to complete the login process.
echo "Running gcloud authentication. Please follow the instructions to log in in your browser."
gcloud auth application-default login --no-launch-browser

# --- Installation and Setup (Run only once) ---
# Create the mount point if it doesn't exist.
# Since this directory is in your home folder, you don't need `sudo` to create it.
if [ ! -d "$MOUNT_POINT" ]; then
    mkdir -p "$MOUNT_POINT"
fi

# Add the Google Cloud GPG key to your system's trusted keys.
# This is a critical step to verify the authenticity of the packages.
curl -fsSL https://packages.cloud.google.com/apt/doc/apt-key.gpg | sudo gpg --dearmor -o /etc/apt/trusted.gpg.d/gcsfuse.gpg

# Add the GCS FUSE repository to your system's sources list.
echo "deb [signed-by=/etc/apt/trusted.gpg.d/gcsfuse.gpg] https://packages.cloud.google.com/apt gcsfuse-`lsb_release -c -s` main" | sudo tee /etc/apt/sources.list.d/gcsfuse.list > /dev/null

# Update the package list to include the new repository.
echo "Updating package list..."
sudo apt-get update

# Install gcsfuse.
echo "Installing gcsfuse..."
sudo apt-get install -y gcsfuse

# --- Mounting the Bucket ---  you might only need to run this if restarting a workstation
# Use the gcsfuse tool to mount the bucket to the specified mount point.
# The mount point is in your home directory, so `sudo` is no longer needed.
echo "Mounting the bucket..."
gcsfuse --implicit-dirs "$BUCKET_NAME" "$MOUNT_POINT"

echo "Mounting complete. You can now access the bucket contents at $MOUNT_POINT"

# Optional: List the contents to verify the mount was successful.
ls -l "$MOUNT_POINT"
```

🤿 Deep dive: R code to read/write to Google Cloud Storage Buckets
(Click to expand)

Work through local IT to request a data bucket. To read/write data to a
Google Cloud Storage Bucket, use the R example below:

``` r
# R code to read and write data to Google Cloud Storage Bucket
# - This R script uses a "cloud mount", meaning the Google Bucket behaves like a local
#   folder on your workstation located at "~/my_gcs_bucket".
# - Current code links to a data bucket set up by FIMS located at
#   https://console.cloud.google.com/storage/browser/nsaw-2026-fims-training.
# - Permissions to view, read, and write to this data bucket must be amended by FIMS team.

# Define the root path for the shared Google Cloud Storage bucket
gcs_bucket_root <- "~/my_gcs_bucket"

# List all files and directories in the top level of the bucket
list.files(gcs_bucket_root)

# List contents of the shared training materials folder
training_folder <- file.path(gcs_bucket_root, "training_materials")
list.files(training_folder)

# Create a personal folder and change 'firstname_lastname' to your name
user_folder_name <- "firstname_lastname"
user_folder <- file.path(gcs_bucket_root, user_folder_name)
# Create the directory if it does not already exist
if (!dir.exists(user_folder)) {
  dir.create(user_folder)
}

# Pull the latest FIMS demo and save it directly to the cloud folder
demo_url <- "https://raw.githubusercontent.com/NOAA-FIMS/FIMS/refs/heads/main/vignettes/fims-demo.Rmd"
local_script_path <- file.path(user_folder, "fims_demo.R")

knitr::purl(
  input = demo_url, 
  output = local_script_path,
  documentation = 0
)

## Execute the downloaded demo script
source(local_script_path)

# Define the file path for saving the model fit
fit_output_path <- file.path(user_folder, "fit.RDS")
saveRDS(
  object = fit,
  file = fit_output_path ,
  compress = FALSE
)

# Verify the save by removing the local object and reading it back from the bucket
rm(fit)
fit <- readRDS(file.path(folder_path, "fit.RDS"))
```

🗂️ Cloud Computing Documentation (Click to expand)

- [NOAA Fisheries Cloud
  Program](https://docs.google.com/document/d/1nziPdPULoRWOYQ9WKzISNUgJvANACvfYpFr1z3Ro2Bc/edit?usp=sharing)
  began a Cloud Compute Accelerator Pilot in early 2025: Enhancing NOAA
  Fisheries’ Mission with Google Cloud Workstations.

- Consult the Pilot Participant [Frequently Asked
  Questions](https://docs.google.com/document/d/1U1PzGS7G70xsXtD6F6WxkjTSCw7bwNVyZ-YnFyOLOqU/edit?usp=sharing)
  for support regarding access and billing.

- [NOAA Fisheries Cloud Computing Setup
  repository](https://github.com/nmfs-opensci/CloudComputingSetup/)
  details cloud computing resources at NOAA Fisheries, with a focus on
  Google Cloud Workstations for R users. It includes instructions for
  linking workstations to GitHub and mounting Google Cloud Storage
  buckets.

## 🌐 GitHub Codespaces

[GitHub Codespaces](https://docs.github.com/en/codespaces) offers a
[“container-as-a-service”](https://www.atlassian.com/microservices/cloud-computing/containers-as-a-service)
model that provides a pre-configured FIMS environment directly in your
browser.

To open the container in a Codespace using the internet browser of your
choice please follow the instructions below. You can navigate there
directly by clicking on [this
link](https://github.com/codespaces/new?hide_repo_select=true&ref=main&repo=452012004&skip_quickstart=true&machine=standardLinux32gb&devcontainer_path=.devcontainer%2Fuser%2Fdevcontainer.json&geo=UsEast)
or follow the manual steps below.

- Click the green `Code` button on the [FIMS
  repository](https://github.com/NOAA-FIMS/FIMS).

- Click the `...` (three dots) next to the `+` icon in the Codespaces
  tab.

- Select `New with options ...`.

- Under the dropdown menu labeled `Dev container configuration`, select
  `FIMS User` or `FIMS Developer`, depending on your role.

- If you plan to run a FIMS model, change the `Machine type` from the
  default `2-core` to at least `4-core`. The default `2-core` will not
  allow you to compile {FIMS} and will lead to your R session
  unexpectedly crashing but it is set as the default because we pay for
  each minute used in Codespaces whether the session is actively being
  used or sitting idle and 2 cores is much cheaper.

- Click the green `Create codespace` button.

- To verify that FIMS is ready, run the R code below to download the
  FIMS demo R script:

``` r

knitr::purl(
  input = 'https://raw.githubusercontent.com/NOAA-FIMS/FIMS/refs/heads/main/vignettes/fims-demo.Rmd',
  output = 'fims_demo.R',
  documentation = 0
)

source('fims_demo.R')
```

## 🐧 WSL2

For Windows users who prefer local execution of the FIMS user container,
we recommend using WSL2 in conjunction with Docker Engine.

To open the container on your local Windows machine using Windows
Subsystem for Linux 2 (WSL2), Docker Engine, and Visual Studio Code (VS
Code) use the following instructions:

### Connect VS Code to WSL

- Open VS Code and press `Ctrl + Shift + P` to open the command palette.
- Type and select `WSL: Connect to WSL using Distro`.
- Choose `Ubuntu` from the list.

### Clone FIMS repository (first time only)

- Once connected to WSL, open a new terminal in VS Code.
- Clone the project repository inside the WSL2 file system.

``` bash
git clone https://github.com/NOAA-FIMS/FIMS.git
```

> **Note:** For other FIMS-related projects, feel free to use the
> [example](https://github.com/NOAA-FIMS/FIMS/tree/main/.devcontainer/user)
> in the `FIMS/.devcontainer/user` folder as a template to set up
> similar devcontainer environments for your own repositories.

### Open FIMS in Dev Container

- In VS Code and press `Ctrl + O` to open the FIMS Folder you just
  cloned.
- Press `Ctrl + Shift + P` to open the command palette.
- Type and select `Dev Containers: Reopen in Container`.
- Select `FIMS User` if you want to run stock assessments using FIMS.
  Select `FIMS Developer` if you are contributing to the source code or
  maintaining the FIMS repository.
- Docker will now build the container. The first build can take several
  minutes for a `FIMS Developer` setup and about 15 minutes for a
  `FIMS User` setup.

### Close the container

When you’re finished, open the Command Palette (`Ctrl + Shift +P`) and
type `Remote: Close Remote Connection`.

> **Note:** To verify that FIMS is ready, run the R code below to
> download the FIMS demo R script:

``` r

knitr::purl(
  input = 'https://raw.githubusercontent.com/NOAA-FIMS/FIMS/refs/heads/main/vignettes/fims-demo.Rmd',
  output = 'fims_demo.R',
  documentation = 0
)

source('fims_demo.R')
```

## 🧑‍💻 Local

Users wishing to install FIMS directly on their host operating system
(Windows, Linux, or macOS) can do so using the `setup_fims.sh` bash
script. To install, execute the following command in the bash terminal:

``` bash
bash <(curl -sL https://raw.githubusercontent.com/NOAA-FIMS/FIMS/main/setup_fims.sh)
```

- To verify that FIMS is ready, run the R code below to download the
  FIMS demo R script:

``` r

knitr::purl(
  input = 'https://raw.githubusercontent.com/NOAA-FIMS/FIMS/refs/heads/main/vignettes/fims-demo.Rmd',
  output = 'fims_demo.R',
  documentation = 0
)

source('fims_demo.R')
```

> **Note:** For Windows users, Git Bash is required. If you are using
> RStudio on Windows, make sure the terminal is configured to use Git
> Bash. You can do this by going to `Tools` -\> `Global Options...` -\>
> `Terminal`, and selecting `Git Bash` under `New terminals open with:`.

> **Note:** This setup is designed to avoid overwriting existing FIMS
> installations by isolating the new environment. The script aims to
> create a dedicated library folder and prepend it to the R search path
> ([`.libPaths()`](https://rdrr.io/r/base/libPaths.html)). FIMS is then
> installed into this specific location. The setup attempts to configure
> R to prioritize this custom folder automatically in future R sessions.
> If you wish to return to your previous FIMS environment, you can
> typically do so by removing the custom folder path from your
> [`.libPaths()`](https://rdrr.io/r/base/libPaths.html) or deleting the
> folder; R is designed to then fall back to the next available path.

## 🙋 Need Help?

- If the instructions above do not work, please visit the FIMS
  [discussion page](https://github.com/orgs/NOAA-FIMS/discussions/1073)
  to see known issues and documented solutions. If you encounter a new
  error, feel free to post it there for technical assistance.

- To report errors in this guide, please [open an
  issue](https://github.com/NOAA-FIMS/FIMS/issues) on GitHub.
