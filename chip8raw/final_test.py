import ctypes
import numpy as np

lib = ctypes.CDLL("./chip8.so")
lib.lib_init.argtypes = [ctypes.c_char_p]
lib.lib_step.argtypes = [ctypes.c_int]
lib.lib_set_key.argtypes = [ctypes.c_int, ctypes.c_int]
lib.lib_getmem.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
lib.lib_get_bcd_addresses.argtypes = [ctypes.POINTER(ctypes.c_uint16)]

mem_buf = (ctypes.c_uint8 * 4096)()
addr_buf = (ctypes.c_uint16 * 2)()

def get_mem():
    lib.lib_getmem(mem_buf)
    return np.array(mem_buf, dtype=np.uint8).copy()

lib.lib_init(b"Pong (alt).ch8")
lib.lib_set_key(0x1, 1)
lib.lib_step(1000)
lib.lib_set_key(0x1, 0)

# Let the game run a bit so both players score
lib.lib_step(200000)

lib.lib_get_bcd_addresses(addr_buf)
p1_addr = addr_buf[0]
p2_addr = addr_buf[1]
print(f"BCD address 0: 0x{p1_addr:03X}")
print(f"BCD address 1: 0x{p2_addr:03X}")

print()
for i in range(20):
    lib.lib_step(100000)
    mem = get_mem()
    s1 = mem[p1_addr + 2] if p1_addr != 0xFFFF else -1
    s2 = mem[p2_addr + 2] if p2_addr != 0xFFFF else -1
    print(f"Step {i:02d}: addr0_score={s1}  addr1_score={s2}")