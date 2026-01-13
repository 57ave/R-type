#pragma once

// This file is kept for backward compatibility.
// R-Type specific protocol has been moved to server/include/network/RTypeProtocol.hpp
// For generic engine usage, include "Packet.hpp" directly.

#include "Packet.hpp"

// Note: RTypeProtocol.hpp is no longer in the engine.
// Games should include their own protocol definition from their game/server directories.
// Example: #include "network/RTypeProtocol.hpp" (from game or server include paths)
 