import ctypes
import numpy as np
import gymnasium as gym
from gymnasium import spaces

lib = ctypes.CDLL("./chip8.so")


# Tell Python the argument and return types of each function
lib.lib_init.argtypes = [ctypes.c_char_p]
lib.lib_reset.argtypes = []
lib.lib_step.argtypes = [ctypes.c_int]
lib.lib_set_key.argtypes = [ctypes.c_int, ctypes.c_int]
lib.lib_get_display.argtypes = [ctypes.POINTER(ctypes.c_uint8)]

ACTIONS = [
    [],
    [0x1],
    [0x2],
]

class Chip8Pongenv(gym.Env):
    def __init__(self, rompath=b"Pong (alt).ch8", cycles_per_step = 10):
        super().__init__()
        self.rompath = rompath
        self.cycles_per_step = cycles_per_step
        self.display_buf = (ctypes.c_uint8 * 2048)()
        self.observation_space = spaces.Box(
            low = 0, high = 1, shape=(32,64), dtype=np.uint8
        )
        self.action_space = spaces.Discrete(3)
        lib.lib_init(self.rompath)
        self.prev_frame = self._get_frame()     
    
    def step(self, action):
        # Release all keys, then press the chosen one
        for key in [0x1, 0x4]:
            lib.lib_set_key(key, 0)
        for key in ACTIONS[action]:
            lib.lib_set_key(key, 1)
        
        lib.lib_step(self.cycles_per_step)

        frame = self._get_frame()
        reward = self._get_reward(frame)
        self.prev_frame = frame

        # No natural "done" signal yet — episodes run for a fixed time
        terminated = False
        truncated = False
        
        return frame, reward, terminated, truncated, {}
    
    def _get_frame(self):
        lib.lib_get_display(self.display_buf)
        return np.array(self.display_buf, dtype=np.uint8).reshape(32, 64)

    def _get_reward(self, frame):
        # Placeholder — returns 0 for now
        return 0.0