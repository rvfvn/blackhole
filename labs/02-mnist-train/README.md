# Train MNIST Lab: Setup

Slides: [lecture 2 · train mnist](https://boopdotpng.github.io/slide-decks/lec2-mnist/)

Works on macOS, Linux, and Windows through WSL (Ubuntu). Run every command in a terminal: **Terminal** on a Mac, **Ubuntu** on Windows.

## 1. Install the tools (once per computer)

**Mac**

```sh
xcode-select --install
```

To train on the Mac's GPU (Metal), tinygrad also needs LLVM from [Homebrew](https://brew.sh):

```sh
brew install llvm@21
echo "export LLVM_PATH=$(brew --prefix llvm@21)/lib/libLLVM.dylib" >> ~/.zshrc
```

Without it, everything still runs on the CPU, just slower.

**Windows (WSL) and Linux**

```sh
sudo apt update
sudo apt install -y clang git
```

**Both:** install [uv](https://docs.astral.sh/uv/), which installs Python and Python packages:

```sh
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Then close the terminal, open a new one, and check it with `uv --version`.

## 2. Set up the lab

From the root of your fork:

```sh
cd labs/02-mnist-train
sh setup.sh
source .venv/bin/activate
```

`setup.sh` runs the same commands as the slides. It creates a Python 3.14 venv in `.venv`, clones tinygrad into `tinygrad/`, installs it with `uv pip install -e ./tinygrad -C editable_mode=compat`, and then runs the checks. Running it again is safe.

## 3. Check it

```sh
python check_setup.py --data
```

Every line should say `ok`. `--data` also downloads MNIST (about 11 MB) ahead of class.

Opening a new terminal later? Run `cd labs/02-mnist-train`, then `source .venv/bin/activate`. Your prompt shows `(02-mnist-train)` when the venv is active.

`.venv/` and `tinygrad/` are git-ignored, so don't commit them. You'll write `mnist.py` in this folder during the lab.

## If something fails

- **`uv: command not found`:** open a new terminal.
- **`permission denied: .venv/bin/activate`:** put `source` in front: `source .venv/bin/activate`.
- **The check shows the wrong Python** (for example Anaconda's, when your prompt shows `(base)`), **or `cannot import name 'Context'`:** the venv isn't active. Run `source .venv/bin/activate`. If the error is still there, re-run `sh setup.sh`.
- **METAL check fails with `failed to load library llvm`:** do the Mac GPU part of step 1, then open a new terminal.
- **CPU check fails:** clang is missing. Redo step 1.
