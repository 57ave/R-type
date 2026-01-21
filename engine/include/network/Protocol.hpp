#pragma once

// This file is kept for backward compatibility.
// Game specific protocol has been moved to server/include/network/GameProtocol.hpp
// For generic engine usage, include "Packet.hpp" directly.

#include "Packet.hpp"

// Note: GameProtocol.hpp is no longer in the engine.
// Games should include their own protocol definition from their game/server directories.
// Example: #include "network/GameProtocol.hpp" (from game or server include paths)
