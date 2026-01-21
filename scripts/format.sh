#!/bin/bash
set -e

# R-Type Code Formatting Script
# Uses clang-format to format all C++ source files

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}❌ Error: clang-format is not installed!${NC}"
    echo "Install it with: sudo apt install clang-format"
    exit 1
fi

# Check mode
CHECK_MODE=false
if [[ "$1" == "--check" ]]; then
    CHECK_MODE=true
    echo -e "${YELLOW}🔍 Running in CHECK mode (no files will be modified)${NC}"
fi

# Find all C++ files
echo "🔍 Finding C++ files..."
FILES=$(find "$PROJECT_ROOT" \
    -type f \
    \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.cc" \) \
    ! -path "*/build*/*" \
    ! -path "*/_deps/*" \
    ! -path "*/cmake-build-*/*" \
    ! -path "*/third_party/*" \
    ! -path "*/external/*")

FILE_COUNT=$(echo "$FILES" | wc -l)
echo "📁 Found $FILE_COUNT files to format"

# Format or check files
UNFORMATTED=()
FORMATTED_COUNT=0

for file in $FILES; do
    if [ "$CHECK_MODE" = true ]; then
        # Check if file needs formatting
        if ! clang-format --dry-run --Werror "$file" &> /dev/null; then
            UNFORMATTED+=("$file")
        fi
    else
        # Format the file
        echo "  Formatting: $file"
        clang-format -i "$file"
        FORMATTED_COUNT=$((FORMATTED_COUNT + 1))
    fi
done

# Report results
echo ""
if [ "$CHECK_MODE" = true ]; then
    if [ ${#UNFORMATTED[@]} -eq 0 ]; then
        echo -e "${GREEN}✅ All files are properly formatted!${NC}"
        exit 0
    else
        echo -e "${RED}❌ Found ${#UNFORMATTED[@]} unformatted files:${NC}"
        for file in "${UNFORMATTED[@]}"; do
            echo -e "  ${RED}✗${NC} $file"
        done
        echo ""
        echo -e "${YELLOW}Run 'scripts/format.sh' to fix formatting${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}✅ Formatted $FORMATTED_COUNT files successfully!${NC}"
fi
