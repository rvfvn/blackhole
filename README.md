# blackhole

Repository for the Club for AI Research's (CAIR) Project Blackhole and its labs.

## Setup (once per computer)

Every lab runs in a terminal on **macOS**, **Linux**, or **Windows through WSL** (Ubuntu). Native Windows terminals (PowerShell, Command Prompt) are not supported.

### macOS

```sh
xcode-select --install
```

Click **Install** in the window that pops up. This gives you `git` and the C compilers (`cc`, `clang`). If it says they're already installed, you're done.

### Windows (WSL)

Open **PowerShell as Administrator** and run:

```powershell
wsl --install -d Ubuntu
```

Restart if Windows asks you to. Then open **Ubuntu** from the Start menu and create a Linux username and password. The password doesn't show while you type; that's normal.

`Ubuntu` installs the latest LTS release (26.04). From here on, run every command in the Ubuntu terminal, not PowerShell, and keep the repo in your Ubuntu home folder (`~`). Then continue with the Linux steps below.

### Linux (and WSL)

Use the latest Ubuntu LTS (26.04):

```sh
sudo apt update
sudo apt install -y build-essential clang git curl
```

On other distributions, install the same tools with your package manager.

### Everyone: uv

uv installs Python and Python packages for the ML labs:

```sh
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Then close the terminal and open a new one.

### Check

```sh
git --version
cc --version
clang --version
uv --version
```

Each command should print a version.

### Editor

Install [VS Code](https://code.visualstudio.com/). On Windows, also install its **WSL** extension, then open the repo from the Ubuntu terminal with `code .`. The bottom-left corner should show `WSL: Ubuntu`.

## Get the repo

1. Fork [rvfvn/blackhole](https://github.com/rvfvn/blackhole) on GitHub.
2. Set your Git identity once:

   ```sh
   git config --global user.name "Your Name"
   git config --global user.email "you@example.com"
   ```

3. Clone your fork, replacing `YOUR-USERNAME` with your GitHub username:

   ```sh
   git clone https://github.com/YOUR-USERNAME/blackhole.git
   cd blackhole
   ```

4. Before starting each lab, select **Sync fork** on your fork's GitHub page, then:

   ```sh
   git switch main
   git pull origin main
   ```

Each lab in `labs/` has its own README with anything extra it needs.
