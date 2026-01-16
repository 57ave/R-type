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
echo "  4) Full test: Server + 2 clients (tmux)"
echo "  5) Full test: Server + 2 clients (separate windows) - RECOMMENDED"
echo ""
read -p "Enter choice (1-5): " choice

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
        echo "This will create 3 panes (Server + 2 Players)"
        echo ""

        # Vérifier si tmux est disponible
        if command -v tmux &> /dev/null; then
            echo -e "${GREEN}Using tmux to create split terminals...${NC}"

            # Créer une session tmux avec 3 panes
            SESSION_NAME="rtype_multiplayer"

            # Tuer la session si elle existe déjà
            tmux kill-session -t $SESSION_NAME 2>/dev/null

            # Créer nouvelle session avec le serveur
            tmux new-session -d -s $SESSION_NAME -n "R-Type"
            tmux send-keys -t $SESSION_NAME "cd $(pwd)" C-m
            tmux send-keys -t $SESSION_NAME "echo '🚀 SERVER - Port 12345'" C-m
            tmux send-keys -t $SESSION_NAME "$BUILD_DIR/server/r-type_server" C-m

            # Split horizontal pour Player 1
            tmux split-window -h -t $SESSION_NAME
            tmux send-keys -t $SESSION_NAME "cd $(pwd)" C-m
            tmux send-keys -t $SESSION_NAME "echo '⏳ Waiting for server to start...'" C-m
            tmux send-keys -t $SESSION_NAME "sleep 3" C-m
            tmux send-keys -t $SESSION_NAME "echo '🎮 PLAYER 1 - Connecting to localhost:12345'" C-m
            tmux send-keys -t $SESSION_NAME "$BUILD_DIR/game/r-type_game --network 127.0.0.1 12345" C-m

            # Split vertical pour Player 2 dans le pane de droite
            tmux split-window -v -t $SESSION_NAME
            tmux send-keys -t $SESSION_NAME "cd $(pwd)" C-m
            tmux send-keys -t $SESSION_NAME "echo '⏳ Waiting for server to start...'" C-m
            tmux send-keys -t $SESSION_NAME "sleep 4" C-m
            tmux send-keys -t $SESSION_NAME "echo '🎮 PLAYER 2 - Connecting to localhost:12345'" C-m
            tmux send-keys -t $SESSION_NAME "$BUILD_DIR/game/r-type_game --network 127.0.0.1 12345" C-m

            # Attacher à la session
            echo ""
            echo -e "${GREEN}✅ Tmux session created!${NC}"
            echo ""
            echo -e "${BLUE}Layout:${NC}"
            echo "  ┌─────────────┬──────────────┐"
            echo "  │             │   Player 1   │"
            echo "  │   SERVER    ├──────────────┤"
            echo "  │             │   Player 2   │"
            echo "  └─────────────┴──────────────┘"
            echo ""
            echo -e "${BLUE}Controls:${NC}"
            echo "  - Ctrl+B then arrow keys: Navigate between panes"
            echo "  - Ctrl+B then D: Detach (keep running in background)"
            echo "  - Ctrl+C in each pane: Stop server/client"
            echo "  - Type 'exit' or Ctrl+D: Close pane"
            echo "  - tmux kill-session -t $SESSION_NAME: Kill all"
            echo ""
            echo -e "${GREEN}Attaching to tmux session in 2 seconds...${NC}"
            sleep 2
            tmux attach-session -t $SESSION_NAME

        # Sinon essayer les émulateurs de terminal graphiques
        elif command -v gnome-terminal &> /dev/null; then
            TERM_CMD="gnome-terminal --"
            echo -e "${GREEN}Using gnome-terminal...${NC}"
            $TERM_CMD bash -c "cd $(pwd) && echo '🚀 SERVER' && $BUILD_DIR/server/r-type_server; read -p 'Press Enter to close...'" &
            sleep 2
            $TERM_CMD bash -c "cd $(pwd) && echo '🎮 PLAYER 1' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; read -p 'Press Enter to close...'" &
            sleep 1
            $TERM_CMD bash -c "cd $(pwd) && echo '🎮 PLAYER 2' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; read -p 'Press Enter to close...'" &
            echo -e "${GREEN}✅ All terminals launched!${NC}"

        elif command -v xterm &> /dev/null; then
            TERM_CMD="xterm -e"
            echo -e "${GREEN}Using xterm...${NC}"
            $TERM_CMD bash -c "cd $(pwd) && echo '🚀 SERVER' && $BUILD_DIR/server/r-type_server; read -p 'Press Enter to close...'" &
            sleep 2
            $TERM_CMD bash -c "cd $(pwd) && echo '🎮 PLAYER 1' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; read -p 'Press Enter to close...'" &
            sleep 1
            $TERM_CMD bash -c "cd $(pwd) && echo '🎮 PLAYER 2' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; read -p 'Press Enter to close...'" &
            echo -e "${GREEN}✅ All terminals launched!${NC}"

        else
            echo -e "${RED}❌ No terminal multiplexer or emulator found${NC}"
            echo ""
            echo -e "${BLUE}📋 Please run manually in 3 separate terminals:${NC}"
            echo ""
            echo -e "${GREEN}Terminal 1 (Server):${NC}"
            echo "  cd $(pwd)"
            echo "  $BUILD_DIR/server/r-type_server"
            echo ""
            echo -e "${GREEN}Terminal 2 (Player 1):${NC}"
            echo "  cd $(pwd)"
            echo "  $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345"
            echo ""
            echo -e "${GREEN}Terminal 3 (Player 2):${NC}"
            echo "  cd $(pwd)"
            echo "  $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345"
            echo ""
            echo -e "${BLUE}💡 Tip: Install tmux for automatic multiplayer testing:${NC}"
            echo "  sudo dnf install tmux     # Fedora/RHEL"
            echo "  sudo apt install tmux     # Debian/Ubuntu"
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
        ;;
    5)
        echo -e "${BLUE}🚀 Starting full multiplayer test in separate windows...${NC}"
        echo ""
        
        # Détecter le terminal disponible
        if command -v gnome-terminal &> /dev/null; then
            TERM_CMD="gnome-terminal"
            TERM_TITLE="--title"
            echo -e "${GREEN}Using gnome-terminal...${NC}"
            
            # Lancer le serveur
            $TERM_CMD $TERM_TITLE "🚀 R-Type SERVER" -- bash -c "cd $(pwd) && echo '🚀 SERVER - Port 12345' && echo '' && $BUILD_DIR/server/r-type_server; echo ''; echo 'Server stopped. Press Enter to close...'; read" &
            sleep 2
            
            # Lancer Player 1
            $TERM_CMD $TERM_TITLE "🎮 R-Type PLAYER 1" -- bash -c "cd $(pwd) && echo '🎮 PLAYER 1 - Connecting to localhost:12345' && echo '' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; echo ''; echo 'Player 1 stopped. Press Enter to close...'; read" &
            sleep 1
            
            # Lancer Player 2
            $TERM_CMD $TERM_TITLE "🎮 R-Type PLAYER 2" -- bash -c "cd $(pwd) && echo '🎮 PLAYER 2 - Connecting to localhost:12345' && echo '' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; echo ''; echo 'Player 2 stopped. Press Enter to close...'; read" &
            
            echo -e "${GREEN}✅ All terminals launched!${NC}"
            echo -e "${BLUE}3 separate windows opened:${NC}"
            echo "  - Server window (port 12345)"
            echo "  - Player 1 window"
            echo "  - Player 2 window"
            echo ""
            echo -e "${BLUE}💡 Tip: Arrange windows side by side to see all logs${NC}"
            
        elif command -v konsole &> /dev/null; then
            TERM_CMD="konsole"
            echo -e "${GREEN}Using konsole...${NC}"
            
            $TERM_CMD --title "🚀 R-Type SERVER" -e bash -c "cd $(pwd) && echo '🚀 SERVER - Port 12345' && echo '' && $BUILD_DIR/server/r-type_server; echo ''; echo 'Server stopped. Press Enter to close...'; read" &
            sleep 2
            $TERM_CMD --title "🎮 R-Type PLAYER 1" -e bash -c "cd $(pwd) && echo '🎮 PLAYER 1' && echo '' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; echo ''; echo 'Player 1 stopped. Press Enter to close...'; read" &
            sleep 1
            $TERM_CMD --title "🎮 R-Type PLAYER 2" -e bash -c "cd $(pwd) && echo '🎮 PLAYER 2' && echo '' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; echo ''; echo 'Player 2 stopped. Press Enter to close...'; read" &
            
            echo -e "${GREEN}✅ All terminals launched!${NC}"
            
        elif command -v xterm &> /dev/null; then
            TERM_CMD="xterm"
            echo -e "${GREEN}Using xterm...${NC}"
            
            $TERM_CMD -title "SERVER" -geometry 100x30+0+0 -e bash -c "cd $(pwd) && echo '🚀 SERVER - Port 12345' && echo '' && $BUILD_DIR/server/r-type_server; echo ''; echo 'Server stopped. Press Enter to close...'; read" &
            sleep 2
            $TERM_CMD -title "PLAYER 1" -geometry 100x30+700+0 -e bash -c "cd $(pwd) && echo '🎮 PLAYER 1' && echo '' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; echo ''; echo 'Player 1 stopped. Press Enter to close...'; read" &
            sleep 1
            $TERM_CMD -title "PLAYER 2" -geometry 100x30+1400+0 -e bash -c "cd $(pwd) && echo '🎮 PLAYER 2' && echo '' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; echo ''; echo 'Player 2 stopped. Press Enter to close...'; read" &
            
            echo -e "${GREEN}✅ All terminals launched!${NC}"
            
        elif command -v alacritty &> /dev/null; then
            TERM_CMD="alacritty"
            echo -e "${GREEN}Using alacritty...${NC}"
            
            $TERM_CMD --title "SERVER" -e bash -c "cd $(pwd) && echo '🚀 SERVER - Port 12345' && echo '' && $BUILD_DIR/server/r-type_server; echo ''; echo 'Server stopped. Press Enter to close...'; read" &
            sleep 2
            $TERM_CMD --title "PLAYER 1" -e bash -c "cd $(pwd) && echo '🎮 PLAYER 1' && echo '' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; echo ''; echo 'Player 1 stopped. Press Enter to close...'; read" &
            sleep 1
            $TERM_CMD --title "PLAYER 2" -e bash -c "cd $(pwd) && echo '🎮 PLAYER 2' && echo '' && $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345; echo ''; echo 'Player 2 stopped. Press Enter to close...'; read" &
            
            echo -e "${GREEN}✅ All terminals launched!${NC}"
            
        else
            echo -e "${RED}❌ No supported terminal emulator found${NC}"
            echo ""
            echo -e "${BLUE}Supported terminals: gnome-terminal, konsole, xterm, alacritty${NC}"
            echo ""
            echo -e "${BLUE}📋 Please run manually in 3 separate terminals:${NC}"
            echo ""
            echo -e "${GREEN}Terminal 1 (Server):${NC}"
            echo "  cd $(pwd)"
            echo "  $BUILD_DIR/server/r-type_server"
            echo ""
            echo -e "${GREEN}Terminal 2 (Player 1):${NC}"
            echo "  cd $(pwd)"
            echo "  $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345"
            echo ""
            echo -e "${GREEN}Terminal 3 (Player 2):${NC}"
            echo "  cd $(pwd)"
            echo "  $BUILD_DIR/game/r-type_game --network 127.0.0.1 12345"
            exit 1
        fi
        ;;
    *)
        echo -e "${RED}Invalid choice${NC}"
        exit 1
        ;;
esac
