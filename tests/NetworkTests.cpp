#include <gtest/gtest.h>
#include "network/Serializer.hpp"
#include "network/Compression.hpp"
#include "network/RoomManager.hpp"
#include "network/Prediction.hpp"
#include "network/Packet.hpp"
#include "network/RTypeProtocol.hpp"
#include <limits>



TEST(SerializerTest, BasicTypes) {
    Network::Serializer serializer;
    serializer.write<int>(42);
    serializer.write<float>(3.14f);
    serializer.write<uint8_t>(255);
    serializer.writeString("Hello World");
    
    auto buffer = serializer.getBuffer();
    Network::Deserializer deserializer(buffer);
    
    EXPECT_EQ(deserializer.read<int>(), 42);
    EXPECT_FLOAT_EQ(deserializer.read<float>(), 3.14f);
    EXPECT_EQ(deserializer.read<uint8_t>(), 255);
    EXPECT_EQ(deserializer.readString(), "Hello World");
}

TEST(SerializerTest, BoundaryChecks) {
    Network::Serializer serializer;
    serializer.write<int>(10);
    
    auto buffer = serializer.getBuffer();
    Network::Deserializer deserializer(buffer);
    
    EXPECT_EQ(deserializer.read<int>(), 10);
    
    // Should throw on underflow
    EXPECT_THROW(deserializer.read<int>(), std::runtime_error);
}

TEST(SerializerTest, StringBoundary) {
    Network::Serializer serializer;
    serializer.writeString("Test");
    
    auto buffer = serializer.getBuffer();
    // Corrupt the length to be huge
    buffer[0] = 0xFF; 
    
    Network::Deserializer deserializer(buffer);
    EXPECT_THROW(deserializer.readString(), std::runtime_error);
}



TEST(CompressionTest, RLE_Efficiency) {
    std::string input = "AAAAABBBCCCCCCDD";
    std::vector<char> data(input.begin(), input.end());
    
    auto compressed = Network::Compression::compress(data);
    

    EXPECT_EQ(compressed.size(), 8); 
    
    auto decompressed = Network::Compression::decompress(compressed);
    std::string output(decompressed.begin(), decompressed.end());
    
    EXPECT_EQ(input, output);
}

TEST(CompressionTest, RLE_NoRepetition) {
    std::string input = "ABCDE";
    std::vector<char> data(input.begin(), input.end());
    
    auto compressed = Network::Compression::compress(data);
    

    EXPECT_EQ(compressed.size(), 10);
    
    auto decompressed = Network::Compression::decompress(compressed);
    std::string output(decompressed.begin(), decompressed.end());
    
    EXPECT_EQ(input, output);
}

TEST(CompressionTest, RLE_Empty) {
    std::vector<char> empty;
    auto compressed = Network::Compression::compress(empty);
    EXPECT_TRUE(compressed.empty());
    
    auto decompressed = Network::Compression::decompress(compressed);
    EXPECT_TRUE(decompressed.empty());
}



TEST(RoomManagerTest, CreateAndJoin) {
    RoomManager manager;
    uint32_t roomId1 = manager.createRoom("Room1");
    uint32_t roomId2 = manager.createRoom("Room2");
    
    EXPECT_NE(roomId1, roomId2);
    EXPECT_NE(roomId1, 0);

    auto room1 = manager.getRoom(roomId1);
    ASSERT_NE(room1, nullptr);
    EXPECT_EQ(room1->name, "Room1");
    EXPECT_EQ(room1->maxPlayers, 4);
    
    EXPECT_TRUE(manager.joinRoom(roomId1, 100));
    EXPECT_TRUE(room1->hasPlayer(100));
    EXPECT_EQ(room1->playerIds.size(), 1);
}

TEST(RoomManagerTest, MaxPlayers) {
    RoomManager manager;

    uint32_t roomId = manager.createRoom("FullRoom");
    
    EXPECT_TRUE(manager.joinRoom(roomId, 1));
    EXPECT_TRUE(manager.joinRoom(roomId, 2));
    EXPECT_TRUE(manager.joinRoom(roomId, 3));
    EXPECT_TRUE(manager.joinRoom(roomId, 4));
    

    EXPECT_FALSE(manager.joinRoom(roomId, 5));
}

TEST(RoomManagerTest, JoinInvalidRoom) {
    RoomManager manager;
    EXPECT_FALSE(manager.joinRoom(999, 1));
}

TEST(RoomManagerTest, LeaveAndAutoDestroy) {
    RoomManager manager;
    uint32_t roomId = manager.createRoom("TempRoom");
    
    manager.joinRoom(roomId, 1);
    EXPECT_NE(manager.getRoom(roomId), nullptr);
    
    manager.leaveRoom(roomId, 1);

    EXPECT_EQ(manager.getRoom(roomId), nullptr);
}

TEST(RoomManagerTest, ListRooms) {
    RoomManager manager;
    manager.createRoom("A");
    manager.createRoom("B");
    
    auto games = manager.getRooms();
    EXPECT_EQ(games.size(), 2);
}



TEST(ProtocolTest, CreateRoomPayload) {
    CreateRoomPayload original;
    original.name = "My Awesome Room";
    
    std::vector<char> buffer = original.serialize();
    CreateRoomPayload result = CreateRoomPayload::deserialize(buffer);
    
    EXPECT_EQ(result.name, original.name);
}

TEST(ProtocolTest, JoinRoomPayload) {
    JoinRoomPayload original;
    original.roomId = 12345;
    
    std::vector<char> buffer = original.serialize();
    JoinRoomPayload result = JoinRoomPayload::deserialize(buffer);
    
    EXPECT_EQ(result.roomId, original.roomId);
}

TEST(ProtocolTest, ClientInput) {
    ClientInput original;
    original.playerId = 10;
    original.inputMask = 0x05;
    original.inputSeq = 42;

    auto packet = RTypeProtocol::createClientInputPacket(original);

    ClientInput result = RTypeProtocol::getClientInput(packet);
    EXPECT_EQ(result.playerId, original.playerId);
    EXPECT_EQ(result.inputMask, original.inputMask);
    EXPECT_EQ(result.inputSeq, 42);
}

TEST(ProtocolTest, WorldSnapshotWithAcks) {
    SnapshotHeader header;
    header.entityCount = 2;
    header.snapshotSeq = 7;
    header.playerAckCount = 1;

    std::vector<PlayerInputAck> acks(1);
    acks[0].playerId = 1;
    acks[0].lastProcessedInputSeq = 99;

    std::vector<EntityState> entities(2);

    entities[0].id = 1;
    entities[0].type = EntityType::ENTITY_PLAYER;
    entities[0].x = 100;
    entities[0].y = 200;
    entities[0].vx = 10;
    entities[0].vy = -10;

    entities[1].id = 2;
    entities[1].type = EntityType::ENTITY_MONSTER;
    entities[1].x = 500;
    entities[1].y = 600;

    auto packet = RTypeProtocol::createWorldSnapshotPacket(header, acks, entities);

    // Deserialize
    auto data = RTypeProtocol::getWorldSnapshot(packet);

    EXPECT_EQ(data.header.entityCount, 2);
    EXPECT_EQ(data.header.snapshotSeq, 7);
    EXPECT_EQ(data.header.playerAckCount, 1);
    ASSERT_EQ(data.acks.size(), 1);
    EXPECT_EQ(data.acks[0].playerId, 1);
    EXPECT_EQ(data.acks[0].lastProcessedInputSeq, 99);
    ASSERT_EQ(data.entities.size(), 2);

    EXPECT_EQ(data.entities[0].id, 1);
    EXPECT_EQ(data.entities[0].x, 100);
    EXPECT_EQ(data.entities[1].id, 2);
    EXPECT_EQ(data.entities[1].type, EntityType::ENTITY_MONSTER);
}

TEST(ProtocolTest, QuantizationLimits) {
    EntityState e;
    e.x = 32767;

    std::vector<char> buf = e.serialize();
    EntityState res = EntityState::deserialize(buf.data());
    EXPECT_EQ(res.x, 32767);
}



TEST(PredictionTest, InterpolationBounds) {
    EntityState start; start.x = 0; start.y = 0;
    EntityState end; end.x = 100; end.y = 100;
    

    auto res = Network::Prediction::interpolate(start, end, 0.0f);
    EXPECT_EQ(res.x, 0);
    

    res = Network::Prediction::interpolate(start, end, 1.0f);
    EXPECT_EQ(res.x, 100);
    

    res = Network::Prediction::interpolate(start, end, 0.5f);
    EXPECT_EQ(res.x, 50);
}

TEST(PredictionTest, InterpolationClamping) {
    EntityState start; start.x = 0;
    EntityState end; end.x = 100;
    

    auto res = Network::Prediction::interpolate(start, end, 1.5f);
    EXPECT_EQ(res.x, 100);
    

    res = Network::Prediction::interpolate(start, end, -0.5f);
    EXPECT_EQ(res.x, 0);
}

TEST(PredictionTest, PredictMovement) {
    EntityState state;
    state.x = 100;
    state.y = 100;
    state.vx = 50;
    state.vy = -50; 
    

    Network::Prediction::predict(state, 1.0f);
    EXPECT_EQ(state.x, 150);
    EXPECT_EQ(state.y, 50);
    

    Network::Prediction::predict(state, 0.5f);
    EXPECT_EQ(state.x, 175);
    EXPECT_EQ(state.y, 25);
}
