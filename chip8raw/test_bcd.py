import ctypes


lib = ctypes.CDLL("./chip8.so")
lib.lib_init.argtypes = [ctypes.c_char_p]
lib.lib_step.argtypes = [ctypes.c_int]
lib.lib_set_key.argtypes = [ctypes.c_int, ctypes.c_int]
lib.lib_get_bcd_address.argtypes = []
lib.lib_get_bcd_address.restype = ctypes.c_uint16


lib.lib_init(b"Pong (alt).ch8")

# Send a brief key press so the game gets past its start state.
lib.lib_set_key(0x1, 1)
lib.lib_step(1000)
lib.lib_set_key(0x1, 0)

for step_index in range(20):
    lib.lib_step(5000)
    bcd_address = lib.lib_get_bcd_address()
    if bcd_address == 0xFFFF:
        print(f"Step {step_index:02d}: no BCD write yet")
    else:
        print(f"Step {step_index:02d}: lib_get_bcd_address() -> 0x{bcd_address:03X}")