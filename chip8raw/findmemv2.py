import ctypes
import numpy as np

lib = ctypes.CDLL("./chip8.so")
lib.lib_init.argtypes = [ctypes.c_char_p]
lib.lib_step.argtypes = [ctypes.c_int]
lib.lib_getmem.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
lib.lib_set_key.argtypes = [ctypes.c_int, ctypes.c_int]

lib.lib_init(b"Pong (alt).ch8")
lib.lib_set_key(0x1, 1)
lib.lib_step(1000)
lib.lib_set_key(0x1, 0)

mem_buf = (ctypes.c_uint8 * 4096)()
lib.lib_get_registers.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
reg_buf = (ctypes.c_uint8 * 16)()

def get_regs():
    lib.lib_get_registers(reg_buf)
    return np.array(reg_buf, dtype=np.uint8).copy()

lib.lib_init(b"Pong (alt).ch8")
lib.lib_set_key(0x1, 1)
lib.lib_step(1000)
lib.lib_set_key(0x1, 0)

for i in range(20):
    lib.lib_step(100000)
    regs = get_regs()
    print(f"Step {i}: {[int(x) for x in regs]}")