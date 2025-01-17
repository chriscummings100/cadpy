import sgl
import numpy as np
from pathlib import Path

SLANG_DIR = Path(__file__).parent / "slang"

device: sgl.Device = None

def init():
    global device
    device = sgl.Device(
        enable_debug_layers=True,
        compiler_options={"include_paths": [SLANG_DIR]},
    )

