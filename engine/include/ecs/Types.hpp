#ifndef RTYPE_ENGINE_ECS_TYPES_HPP
#define RTYPE_ENGINE_ECS_TYPES_HPP

#include <cstdint>
#include <bitset>

namespace ECS {

// Entity is just a unique ID (local to this process)
using Entity = std::uint32_t;

// NetworkId is used to identify entities across the network
// Server assigns these IDs, clients use them to map to local entities
using NetworkId = std::uint32_t;

// ComponentType is used to identify component types
using ComponentType = std::uint8_t;

// Maximum number of entities in the system
constexpr Entity MAX_ENTITIES = 5000;

// Maximum number of component types
constexpr ComponentType MAX_COMPONENTS = 64;

// Invalid network ID (used for local-only entities)
constexpr NetworkId INVALID_NETWORK_ID = 0;

// Signature is a bitset that indicates which components an entity has
// Each bit corresponds to a component type
// Example: 0b00000101 means the entity has components 0 and 2
using Signature = std::bitset<MAX_COMPONENTS>;

} // namespace ECS

#endif // RTYPE_ENGINE_ECS_TYPES_HPP
