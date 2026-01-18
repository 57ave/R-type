#!/bin/bash

# Script simple pour tester le multiplayer avec logs

echo "🎮 R-Type Multiplayer Test - Avec Logs"
echo "======================================"
echo ""

# Créer le dossier de sortie
mkdir -p output_test

echo "1️⃣  Lancement du serveur..."
./build/server/r-type_server > output_test/server.log 2>&1 &
SERVER_PID=$!
echo "   Serveur lancé (PID: $SERVER_PID)"
sleep 2

echo ""
echo "2️⃣  Lancement du client 1 (Player 1) avec --network..."
./build/game/r-type_game --network 127.0.0.1 12345 > output_test/player1.log 2>&1 &
CLIENT1_PID=$!
echo "   Client 1 lancé (PID: $CLIENT1_PID)"
sleep 2

echo ""
echo "3️⃣  Lancement du client 2 (Player 2) avec --network..."
./build/game/r-type_game --network 127.0.0.1 12345 > output_test/player2.log 2>&1 &
CLIENT2_PID=$!
echo "   Client 2 lancé (PID: $CLIENT2_PID)"

echo ""
echo "✅ Tous les processus sont lancés !"
echo ""
echo "📋 Processus actifs :"
echo "   - Serveur: PID $SERVER_PID"
echo "   - Client 1: PID $CLIENT1_PID"
echo "   - Client 2: PID $CLIENT2_PID"
echo ""
echo "📝 Logs disponibles dans :"
echo "   - output_test/server.log"
echo "   - output_test/player1.log"
echo "   - output_test/player2.log"
echo ""
echo "🎮 Instructions :"
echo "   1. Dans le client 1: allez dans MULTIPLAYER > CREATE ROOM"
echo "   2. Dans le client 2: allez dans MULTIPLAYER > Rejoignez la room"
echo "   3. Dans le client 1 (host): cliquez sur START GAME"
echo ""
echo "⛔ Pour arrêter tous les processus :"
echo "   kill $SERVER_PID $CLIENT1_PID $CLIENT2_PID"
echo ""
echo "   OU utilisez Ctrl+C et ensuite:"
echo "   killall r-type_server r-type_game"
echo ""

# Fonction de nettoyage quand on fait Ctrl+C
cleanup() {
    echo ""
    echo "🛑 Arrêt des processus..."
    kill $SERVER_PID $CLIENT1_PID $CLIENT2_PID 2>/dev/null
    echo "✅ Processus arrêtés"
    exit 0
}

trap cleanup INT TERM

# Attendre que l'utilisateur arrête le script
echo "Appuyez sur Ctrl+C pour arrêter tous les processus..."
wait
