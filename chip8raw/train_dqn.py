import argparse

import numpy as np
import torch as th
import torch.nn as nn
import gymnasium as gym
from gymnasium import spaces

from stable_baselines3 import DQN
from stable_baselines3.common.vec_env import DummyVecEnv, VecFrameStack
from stable_baselines3.common.callbacks import EvalCallback, CheckpointCallback
from stable_baselines3.common.torch_layers import BaseFeaturesExtractor

from chip8_env import Chip8Pongenv



# Observation wrapper: reshape (32,64) -> (1,32,64) and scale {0,1} -> {0,255}
# so SB3's CnnPolicy recognises it as an image and auto-normalises to float.

class ScaleAndReshapeObs(gym.ObservationWrapper):
    def __init__(self, env):
        super().__init__(env)
        self.observation_space = spaces.Box(
            low=0, high=255, shape=(1, 32, 64), dtype=np.uint8
        )

    def observation(self, obs):
        return (obs * 255).reshape(1, 32, 64)



# Custom CNN feature extractor for 32x64 input.
# SB3's NatureCNN uses kernels too large for this resolution (height -> 0).

class SmallCNN(BaseFeaturesExtractor):
    def __init__(self, observation_space, features_dim=256):
        super().__init__(observation_space, features_dim)
        n_input_channels = observation_space.shape[0]  # 4 after frame stack
        self.cnn = nn.Sequential(
            nn.Conv2d(n_input_channels, 32, kernel_size=3, stride=2, padding=1),
            nn.ReLU(),
            nn.Conv2d(32, 64, kernel_size=3, stride=2, padding=1),
            nn.ReLU(),
            nn.Conv2d(64, 64, kernel_size=3, stride=2, padding=1),
            nn.ReLU(),
            nn.Flatten(),
        )
        # Compute flattened size by doing a dummy forward pass
        with th.no_grad():
            sample = th.zeros(1, *observation_space.shape)
            n_flat = self.cnn(sample).shape[1]
        self.linear = nn.Sequential(nn.Linear(n_flat, features_dim), nn.ReLU())

    def forward(self, observations):
        return self.linear(self.cnn(observations))



# Environment factories

def make_env(cycles_per_step=10):
    def _init():
        env = Chip8Pongenv(cycles_per_step=cycles_per_step)
        env = ScaleAndReshapeObs(env)
        return env
    return _init



# Train

def train(total_timesteps=500_000, cycles_per_step=10, seed=42):
    train_venv = DummyVecEnv([make_env(cycles_per_step)])
    train_venv = VecFrameStack(train_venv, n_stack=4, channels_order="first")

    eval_venv = DummyVecEnv([make_env(cycles_per_step)])
    eval_venv = VecFrameStack(eval_venv, n_stack=4, channels_order="first")

    model = DQN(
        policy="CnnPolicy",
        env=train_venv,
        learning_rate=1e-4,
        buffer_size=50_000,
        learning_starts=1_000,
        batch_size=64,
        gamma=0.99,
        train_freq=4,
        gradient_steps=1,
        target_update_interval=1_000,
        exploration_fraction=0.2,
        exploration_initial_eps=1.0,
        exploration_final_eps=0.02,
        max_grad_norm=10,
        tau=1.0,
        policy_kwargs=dict(
            features_extractor_class=SmallCNN,
            features_extractor_kwargs=dict(features_dim=256),
            net_arch=[],
        ),
        tensorboard_log="./tb_logs/",
        verbose=1,
        seed=seed,
        device="auto",
    )

    eval_cb = EvalCallback(
        eval_venv,
        best_model_save_path="./models/best/",
        log_path="./models/eval_logs/",
        eval_freq=5_000,
        n_eval_episodes=10,
        deterministic=True,
    )
    ckpt_cb = CheckpointCallback(
        save_freq=25_000,
        save_path="./models/checkpoints/",
        name_prefix="dqn_pong",
    )

    model.learn(
        total_timesteps=total_timesteps,
        callback=[eval_cb, ckpt_cb],
        log_interval=10,
        progress_bar=True,
    )
    model.save("./models/dqn_pong_final")
    print("Training complete. Model saved to ./models/dqn_pong_final.zip")

    train_venv.close()
    eval_venv.close()



# Evaluate

def evaluate(model_path, cycles_per_step=10, n_episodes=10):
    venv = DummyVecEnv([make_env(cycles_per_step)])
    venv = VecFrameStack(venv, n_stack=4, channels_order="first")

    model = DQN.load(model_path, env=venv)
    wins, losses = 0, 0

    for ep in range(n_episodes):
        obs = venv.reset()
        done = False
        while not done:
            action, _ = model.predict(obs, deterministic=True)
            obs, reward, dones, infos = venv.step(action)
            done = dones[0]

        info = infos[0]
        my, opp = info["my_score"], info["opp_score"]
        result = "WIN" if my > opp else "LOSS"
        if my > opp:
            wins += 1
        else:
            losses += 1
        print(f"Episode {ep+1}: {my}-{opp} ({result})")

    print(f"\nResults: {wins}W / {losses}L out of {n_episodes} games")
    venv.close()



# CLI

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Train/evaluate DQN on Chip8 Pong")
    parser.add_argument("--mode", choices=["train", "eval"], default="train")
    parser.add_argument("--model-path", type=str, default="./models/dqn_pong_final")
    parser.add_argument("--total-timesteps", type=int, default=500_000)
    parser.add_argument("--cycles-per-step", type=int, default=10)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--n-episodes", type=int, default=10)
    args = parser.parse_args()

    if args.mode == "train":
        train(args.total_timesteps, args.cycles_per_step, args.seed)
    else:
        evaluate(args.model_path, args.cycles_per_step, args.n_episodes)
