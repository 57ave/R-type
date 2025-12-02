/*

🎯 Rôle du fichier
Gérer toute la communication UDP serveur ↔ clients.

📌 Contenu attendu
Bind sur le port UDP
Réception non bloquante des packets clients :
    inputs
    demandes de connexion
    keep-alive
Envoi des WorldUpdate (snapshots) vers tous les clients
Stockage des adresses/ports des clients actifs

Fonctionnalités internes
File thread-safe pour push les messages entrants (vers TickThread)
File thread-safe pour push les snapshots à envoyer (depuis TickThread)
Gestion du timeout des clients inactifs

🚫 Ce fichier NE doit pas faire
Pas de logique du jeu
Pas d’accès direct aux entités
Pas de parsing complexe : juste sérialisation/désérialisation
*/