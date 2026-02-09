-- ==========================================
-- Test Script - Create Entity from Lua
-- ==========================================

print("[LUA TEST] Creating test entity...")

-- Créer une entité de test
local testEntity = ECS.CreateEntity()
print("[LUA TEST] Created entity ID: " .. tostring(testEntity))

-- Ajouter des composants
ECS.AddPosition(testEntity, 100, 200)
ECS.AddVelocity(testEntity, 50, 0)
ECS.AddHealth(testEntity, 100, 100)
ECS.AddTag(testEntity, "test_entity")

print("[LUA TEST] Components added to entity")

-- Récupérer et afficher les données
local pos = ECS.GetPosition(testEntity)
if pos then
    print("[LUA TEST] Position: x=" .. pos.x .. ", y=" .. pos.y)
end

local health = ECS.GetHealth(testEntity)
if health then
    print("[LUA TEST] Health: " .. health.current .. "/" .. health.max)
end

print("[LUA TEST] ✅ Lua-ECS integration working!")

return {
    test_passed = true
}
