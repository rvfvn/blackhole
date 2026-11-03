"""Checks that this lab is ready to run: python 3.14 from .venv, tinygrad from ./tinygrad, and a working device.

  python check_setup.py           # the checks
  python check_setup.py --data    # also download mnist (about 11 MB) so class doesn't wait on wifi
"""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
failed = False

def report(good: bool, msg: str, fix: str = ""):
  global failed
  print(("  ok    " if good else "  FAIL  ") + msg)
  if not good:
    failed = True
    if fix: print("        -> " + fix)

# 1. the right python
v = sys.version_info
report(v >= (3, 14), f"python {v.major}.{v.minor}.{v.micro}", "run sh setup.sh, then: source .venv/bin/activate")
venv = (HERE / ".venv").resolve()
report(Path(sys.prefix).resolve() == venv, f"python is {sys.executable}", "activate this lab's venv: source .venv/bin/activate")

# 2. tinygrad, from the clone next to this file
try:
  import tinygrad
  from tinygrad import Context, Device, Tensor  # noqa: F401  (Context fails if the install mode is wrong)
except Exception as e:
  report(False, f"import tinygrad: {e}", "source .venv/bin/activate; if it's already active, re-run sh setup.sh")
  sys.exit(1)
src = Path(tinygrad.__file__).resolve().parent
report(src == HERE / "tinygrad" / "tinygrad", f"tinygrad from {src}", "re-run sh setup.sh from this folder")

# 3. it computes: on the default device (Metal / CUDA / ...), and on the cpu, which needs clang
def compute(device: str):
  try:
    out = (Tensor([1, 2, 3], device=device) * 2).tolist()
    report(out == [2, 4, 6], f"(Tensor([1, 2, 3]) * 2) on {device} -> {out}")
  except Exception as e:
    if device == "CPU": fix = "install clang (README.md, step 1)"
    elif "llvm" in str(e).lower(): fix = "the Mac GPU needs Homebrew LLVM (README.md, step 1); the CPU works without it"
    else: fix = "the lab still runs on the CPU: DEV=CPU python mnist.py"
    report(False, f"compute on {device}: {type(e).__name__}: {e}", fix)

compute(Device.DEFAULT)
if Device.DEFAULT != "CPU": compute("CPU")

# 4. optional: fetch the dataset now
if "--data" in sys.argv:
  try:
    from tinygrad.nn.datasets import mnist
    xtrain, ytrain, xtest, ytest = mnist()
    good = xtrain.shape == (60000, 1, 28, 28) and xtest.shape == (10000, 1, 28, 28)
    report(good, f"mnist: xtrain {xtrain.shape}, ytrain {ytrain.shape}, xtest {xtest.shape}, ytest {ytest.shape}")
  except Exception as e:
    report(False, f"mnist download: {e}", "check your internet connection and try again")

print("\nnot ready yet: fix the FAIL lines above" if failed else "\nready for the lab")
sys.exit(1 if failed else 0)
