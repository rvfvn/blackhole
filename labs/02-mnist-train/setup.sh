#!/usr/bin/env sh
# Sets up this lab: a Python 3.14 venv in .venv with tinygrad installed from a local clone in tinygrad/.
# Same steps as the "get tinygrad" slide. Safe to re-run.

set -eu

cd "$(dirname "$0")"

missing=""
for tool in git clang uv; do
  command -v "$tool" >/dev/null 2>&1 || missing="$missing $tool"
done
if [ -n "$missing" ]; then
  echo "missing:$missing (see README.md, step 1; if you just installed uv, open a new terminal)" >&2
  exit 1
fi

if [ -d .venv ]; then
  echo "== .venv exists, keeping it"
else
  echo "== creating .venv (python 3.14)"
  uv venv --python 3.14
fi

if [ -d tinygrad ]; then
  echo "== tinygrad/ exists, keeping it"
else
  echo "== cloning tinygrad"
  git clone https://github.com/tinygrad/tinygrad.git
fi

# compat: mnist.py sits next to the tinygrad/ folder, and only this mode makes it import the real package
echo "== installing tinygrad into .venv"
uv pip install --python .venv/bin/python -e ./tinygrad -C editable_mode=compat

echo "== checking"
.venv/bin/python check_setup.py "$@"

echo
echo "done. now run:  source .venv/bin/activate"
