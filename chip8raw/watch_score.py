import ctypes
import numpy as np

lib = ctypes.CDLL("./chip8.so")
lib.lib_init.argtypes = [ctypes.c_char_p]
lib.lib_step.argtypes = [ctypes.c_int]
lib.lib_set_key.argtypes = [ctypes.c_int, ctypes.c_int]
lib.lib_getmem.argtypes = [ctypes.POINTER(ctypes.c_uint8)]

mem_buf = (ctypes.c_uint8 * 4096)()

def get_mem():
    lib.lib_getmem(mem_buf)
    return np.array(mem_buf, dtype=np.uint8).copy()

lib.lib_init(b"Pong (alt).ch8")
lib.lib_set_key(0x1, 1)
lib.lib_step(1000)
lib.lib_set_key(0x1, 0)

# Sample the three addresses repeatedly
for i in range(20):
    lib.lib_step(100000)
    mem = get_mem()
    print(f"Step {i}: 0x2F2={mem[0x2F2]}  0x2F3={mem[0x2F3]}  0x2F4={mem[0x2F4]}")