#pragma once

// This file is kept for backward compatibility and convenience.
// It includes the concrete R-Type protocol implementation.
// For generic engine usage, include "Packet.hpp" directly.

#include "RTypeProtocol.hpp"

// Typedef for ease of use if you want to keep using "RTypePacket" name in legacy code, 
// though we encourage switching to NetworkPacket + RTypeProtocol helpers.
using RTypePacket = NetworkPacket; 