#!/bin/bash

echo "🎮 R-Type Multiplayer - Quick Test Script"
echo "=========================================="
echo ""

# Couleurs
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

BUILD_DIR="./build"

# Vérifier que les binaires existent
if [ ! -f "$BUILD_DIR/server/r-type_server" ]; then
    echo -e "${RED}❌ Server not found. Please run: cd build && cmake .. && make${NC}"
    exit 1
fi

if [ ! -f "$BUILD_DIR/game/r-type_game" ]; then
    echo -e "${RED}❌ Game not found. Please run: cd build && cmake .. && make${NC}"
    exit 1
fi

echo -e "${GREEN}✅ Binaries found!${NC}"
echo ""
echo "Choose a mode:"
echo "  1) Start SERVER only"
echo "  2) Start LOCAL game (no network)"
echo "  3) Start NETWORK game (connects to localhost:12345)"
echo "  4) Full test: Server + 2 clients in separate terminals"
echo ""
read -p "Enter choice (1-4): " choice

case $choice in
    1)
        echo -e "${BLUE}🚀 Starting server on port 12345...${NC}"
        $BUILD_DIR/server/r-type_server
        ;;
    2)
        echo -e "${BLUE}🎮 Starting local game...${NC}"
        $BUILD_DIR/game/r-type_game
        ;;
    3)
        echo -e "${BLUE}🌐 Starting network game (connecting to localhost:12345)...${NC}"
        echo -e "${RED}⚠️  Make sure the server is running in another terminal!${NC}"
        sleep 2
        $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345
        ;;
    4)
        echo -e "${BLUE}🚀 Starting full multiplayer test...${NC}"
        echo ""
        echo "This will open 3 terminals:"
        echo "  - Terminal 1: Server"
        echo "  - Terminal 2: Player 1"
        echo "  - Terminal 3: Player 2"
        echo ""
        
        # Détecter le terminal émulateur
        if command -v gnome-terminal &> /dev/null; then
            TERM_CMD="gnome-terminal --"
        elif command -v xterm &> /dev/null; then
            TERM_CMD="xterm -e"
        elif command -v konsole &> /dev/null; then
            TERM_CMD="konsole -e"
        else
            echo -e "${RED}❌ No terminal emulator found (gnome-terminal, xterm, konsole)${NC}"
            echo "Please run the server and clients manually:"
            echo "  Terminal 1: $BUILD_DIR/server/r-type_server"
            echo "  Terminal 2: $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345"
            echo "  Terminal 3: $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345"
            exit 1
        fi
        
        # Lancer le serveur
        echo -e "${GREEN}Starting server...${NC}"
        $TERM_CMD bash -c "cd $(pwd) && echo '🚀 SERVER' && $BUILD_DIR/server/r-type_server 2>&1 | tee server.log; read -p 'Press Enter to close...'" &
        sleep 2
        
        # Lancer le premier client
        echo -e "${GREEN}Starting Player 1...${NC}"
        $TERM_CMD bash -c "cd $(pwd) && echo '🎮 PLAYER 1' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345 2>&1 | tee client1.log; read -p 'Press Enter to close...'" &
        sleep 1
        
        # Lancer le deuxième client
        echo -e "${GREEN}Starting Player 2...${NC}"
        $TERM_CMD bash -c "cd $(pwd) && echo '🎮 PLAYER 2' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345 2>&1 | tee client2.log; read -p 'Press Enter to close...'" &
        
        echo ""
        echo -e "${GREEN}✅ All terminals launched!${NC}"
        echo "You should see 3 windows:"
        echo "  - The server showing connections and game state"
        echo "  - Two game clients that can see each other"
        echo ""
        echo "Press Ctrl+C in each window to stop."
        ;;
    *)
        echo -e "${RED}Invalid choice${NC}"
        exit 1
        ;;
esac
