# Contributing

## Branches

| Branch | Purpose |
|---|---|
| `main` | Stable, released code only |
| `dev` | Integration branch — all features merge here first |
| `feat/windows-setup` | Windows build support |

New work goes on a feature branch cut from `dev`, then merged back via pull request.

### Branch naming

```
feat/<short-description>     # new feature
fix/<short-description>      # bug fix
docs/<short-description>     # documentation only
refactor/<short-description> # code cleanup with no behavior change
```

Examples: `feat/enemy-ai`, `fix/network-timeout`, `docs/protocol`.

---

## Workflow

1. Branch off `dev`:
   ```bash
   git checkout dev
   git pull origin dev
   git checkout -b feat/my-feature
   ```

2. Make your changes. Keep commits focused — one logical change per commit.

3. Push and open a PR targeting `dev`:
   ```bash
   git push origin feat/my-feature
   gh pr create --base dev --head feat/my-feature
   ```

4. Get at least one review before merging.

---

## Commit Messages

Use the imperative form, one line summary under 72 characters:

```
feat: add zigzag movement pattern for enemies
fix: prevent server crash on malformed CLIENT_HELLO
docs: update protocol packet type table
refactor: split NetworkManager into send and receive paths
```

Prefix: `feat`, `fix`, `docs`, `refactor`, `test`, `chore`.

---

## Code Style

- C++17, no exceptions in hot paths.
- All gameplay values go in Lua config files, not in C++ source.
- New components and systems must be registered through the ECS Coordinator — no direct manager calls from game code.
- New engine features (components, systems) that are game-specific belong in `game/`, not `engine/`.
- No magic numbers. Use named constants or Lua config values.
- Comment non-obvious logic. Headers need a brief description of the class/struct purpose.

---

## Adding a New Enemy Type

1. Add an entry to `game/assets/scripts/config/enemies_config.lua`.
2. Add the sprite to `game/assets/enemies/`.
3. If the movement pattern does not exist, implement it in the `EnemyAISystem` and expose it to Lua.
4. Reference the new enemy type in a wave file under `game/assets/scripts/levels/`.

No C++ recompilation required for new enemy configurations.

---

## Adding a New System

1. Create the header in `engine/include/systems/` or `game/include/systems/` depending on scope.
2. Implement in the corresponding `src/` directory.
3. Define the required component `Signature` and register the system in the appropriate `Game` or state initialization code.
4. Add the system to `engine/CMakeLists.txt` or `game/CMakeLists.txt`.
