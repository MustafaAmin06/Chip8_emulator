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
lib.lib_get_registers.argtypes = [ctypes.POINTER(ctypes.c_uint8)]

ACTIONS = [
    [],
    [0x1],
    [0x4],
]

WIN_SCORE = 3  # First to this many points ends the episode

class Chip8Pongenv(gym.Env):
    def __init__(self, rompath=b"Pong (alt).ch8", cycles_per_step=10, max_episode_steps=2000):
        super().__init__()
        self.rompath = rompath
        self.cycles_per_step = cycles_per_step
        self.max_episode_steps = max_episode_steps
        self.display_buf = (ctypes.c_uint8 * 2048)()
        self._reg_buf = (ctypes.c_uint8 * 16)()
        self.observation_space = spaces.Box(
            low=0, high=1, shape=(32, 64), dtype=np.uint8
        )
        self.action_space = spaces.Discrete(3)
        lib.lib_init(self.rompath)
        self.prev_ve = 0
        self.my_score = 0
        self.opp_score = 0
        self.episode_steps = 0
        self.prev_frame = self._get_frame()

    def step(self, action):
        # Release all keys, then press the chosen one
        for key in [0x1, 0x4]:
            lib.lib_set_key(key, 0)
        for key in ACTIONS[action]:
            lib.lib_set_key(key, 1)

        lib.lib_step(self.cycles_per_step)
        self.episode_steps += 1

        frame = self._get_frame()
        reward, terminated = self._get_reward()
        truncated = self.episode_steps >= self.max_episode_steps
        self.prev_frame = frame

        return frame, reward, terminated, truncated, {
            "my_score": self.my_score,
            "opp_score": self.opp_score,
        }

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        lib.lib_reset()
        self.prev_ve = 0
        self.my_score = 0
        self.opp_score = 0
        self.episode_steps = 0
        self.prev_frame = self._get_frame()
        return self.prev_frame, {}

    def _get_frame(self):
        lib.lib_get_display(self.display_buf)
        return np.array(self.display_buf, dtype=np.uint8).reshape(32, 64)

    def _get_ve(self):
        lib.lib_get_registers(self._reg_buf)
        return int(self._reg_buf[0xE])

    def _get_reward(self):
        ve = self._get_ve()
        delta = (ve - self.prev_ve) % 256  # safe uint8 wrap
        self.prev_ve = ve

        reward = 0.0
        if delta == 10:       # left paddle (us) scored
            self.my_score += 1
            reward = 1.0
        elif delta == 1:      # right paddle (opponent) scored
            self.opp_score += 1
            reward = -1.0

        terminated = self.my_score >= WIN_SCORE or self.opp_score >= WIN_SCORE
        return reward, terminated

if __name__ == "__main__":
    env = Chip8Pongenv(cycles_per_step=500)
    obs, _ = env.reset()
    print("Obs shape:", obs.shape)

    total_steps = 0
    episodes = 0
    while episodes < 3:
        action = env.action_space.sample()
        obs, reward, terminated, truncated, info = env.step(action)
        total_steps += 1
        if reward != 0:
            print(f"  step {total_steps}: reward={reward:+.0f}  score {info['my_score']}-{info['opp_score']}")
        if terminated:
            episodes += 1
            print(f"Episode {episodes} done in {total_steps} steps — final {info['my_score']}-{info['opp_score']}")
            obs, _ = env.reset()
            total_steps = 0