# 🐦 Flappy Bird Multiplayer - Quick Start Guide

## Build the Project

```bash
cd /Users/gustavedelecroix/Desktop/epitech/tek3/r-cpp/R-type
cmake -B build
cmake --build build -j4
```

## Run the Game

### Option 1: Using Helper Scripts (RECOMMENDED)

**Terminal 1 - Server:**
```bash
./run_server.sh
```

**Terminal 2 - Client 1:**
```bash
./run_client.sh
```

**Terminal 3 - Client 2:**
```bash
./run_client.sh
```

### Option 2: Manual (if scripts don't work)

**Terminal 1 - Server:**
```bash
cd /Users/gustavedelecroix/Desktop/epitech/tek3/r-cpp/R-type
./build/server/flappy_server
```

**Terminal 2 - Client 1:**
```bash
cd /Users/gustavedelecroix/Desktop/epitech/tek3/r-cpp/R-type/build/bin
./flappy_bird
```

**Terminal 3 - Client 2:**
```bash
cd /Users/gustavedelecroix/Desktop/epitech/tek3/r-cpp/R-type/build/bin
./flappy_bird
```

## How to Play

1. **Start the server first**
2. **Launch client 1** - You'll see "Waiting for opponent..."
3. **Launch client 2** - Countdown will begin automatically
4. **Press M or 2** on the menu to start multiplayer
5. **Press SPACE** to flap
6. **Avoid pipes** and stay alive
7. **Last bird flying wins!**

## Important Notes

⚠️ **The client MUST be run from `build/bin/` directory**, otherwise it can't find the assets!

✅ **Use the helper scripts** - they automatically cd to the right directory

🎮 **Server:** `/build/server/flappy_server` (the one we fixed with physics!)
🐦 **Client:** `/build/bin/flappy_bird`

## What We Fixed

✅ Non-blocking countdown (both clients sync properly)
✅ Birds only created when game starts (not before second player joins)
✅ Server-side physics (gravity, collisions, bounds checking)
✅ Proper GAME_STATE packet format (floats instead of uint16)
✅ Network float parsing helper (bytesToFloat)
✅ Pipe spawning and movement
✅ Death detection and game over logic

## Troubleshooting

**"Cannot open game2/assets/scripts/init.lua"**
- You're running the client from the wrong directory
- Use `./run_client.sh` or manually `cd build/bin` first

**"[Room] Error creating room"**
- Ignore this - it's just the engine trying to deserialize unknown packets
- The game works despite this error

**"No pipes/gravity/input"**
- Make sure you're running `./build/server/flappy_server`
- NOT the old simple_flappy_server
