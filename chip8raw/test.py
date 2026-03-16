import ctypes
import numpy as np

lib = ctypes.CDLL("./chip8.so")


lib = ctypes.CDLL("./chip8.so")

# Tell Python the argument and return types of each function
lib.lib_init.argtypes = [ctypes.c_char_p]
lib.lib_reset.argtypes = []
lib.lib_step.argtypes = [ctypes.c_int]
lib.lib_set_key.argtypes = [ctypes.c_int, ctypes.c_int]
lib.lib_get_display.argtypes = [ctypes.POINTER(ctypes.c_uint8)]

# Create a buffer for the display (2048 bytes)
display_buf = (ctypes.c_uint8 * 2048)()

# Init and run 100 cycles
lib.lib_init(b"Pong (alt).ch8")
lib.lib_step(100)

# Read the display
lib.lib_get_display(display_buf)
frame = np.array(display_buf, dtype=np.uint8).reshape(32, 64)

print("Display shape:", frame.shape)
print("Unique values:", np.unique(frame))  # should be [0] or [0, 1]
print("Pixels on:", frame.sum())    