import ctypes
import numpy as np

lib = ctypes.CDLL("./chip8.so")
lib.lib_init.argtypes = [ctypes.c_char_p]
lib.lib_step.argtypes = [ctypes.c_int]
lib.lib_getmem.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
lib.lib_set_key.argtypes = [ctypes.c_int, ctypes.c_int]

mem_buf = (ctypes.c_uint8 * 4096)()

def lib_getmem():
    lib.lib_getmem(mem_buf)
    return np.array(mem_buf, dtype=np.uint8).copy()

lib.lib_init(b"Pong (alt).ch8")

# Press key to start
lib.lib_set_key(0x1, 1)
lib.lib_step(1000)
lib.lib_set_key(0x1, 0)

# Run long enough for the game to start
lib.lib_step(500000)
snapshot_before = lib_getmem()

# Run more
lib.lib_step(500000)
snapshot_after = lib_getmem()

# Find addresses that changed
changed = np.where(snapshot_before != snapshot_after)[0]
print("Changed addresses:")
for addr in changed:
    print(f"  0x{addr:03X}: {snapshot_before[addr]} -> {snapshot_after[addr]}")
    
#This returned Changed addresses:
 # 0x2F2: 1 -> 2
  #0x2F3: 0 -> 3
  #0x2F4: 9 -> 6   The most likely candidate is the first one. Lets try it