#!/bin/bash
# Patch sol2 pour corriger le bug avec GCC récent

SOL2_FILE="build/_deps/sol2-src/include/sol/optional_implementation.hpp"

if [ ! -f "$SOL2_FILE" ]; then
    echo "❌ Fichier sol2 non trouvé: $SOL2_FILE"
    exit 1
fi

echo "🔧 Application du patch sol2..."

# Backup
cp "$SOL2_FILE" "$SOL2_FILE.bak"

# Commenter la ligne problématique (ligne 2191)
sed -i '2191s/.*/                        \/\/ this->construct(std::forward<Args>(args)...); \/\/ Commented out for GCC 11+ compatibility/' "$SOL2_FILE"

echo "✅ Patch appliqué avec succès!"
echo "   Fichier: $SOL2_FILE"
echo "   Backup: $SOL2_FILE.bak"
