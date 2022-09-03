/*
 * \brief
 * \author Daniel Götz
 */


#include "catch.hpp"
#include "../ECSTestUtils.h"
#include <ecs/EntityManager.h>

SCENARIO("Entities can be destroyed") {
    auto manager = CreateEntityManager();

    GIVEN("A created entity") {
        auto entity = manager->CreateEntity();
        auto chunk = manager->GetChunk(entity);
        auto chunkFreeSpace = chunk->GetFree();

        REQUIRE(manager->IsAlive(entity));
        REQUIRE(manager->EntityCount() == 1);


        WHEN("The entity is destroyed") {
            manager->DestroyEntity(entity);

            THEN("the entity is still alive") {
                REQUIRE(manager->IsAlive(entity));
            }

            THEN("The chunk still contains the entity"){
                REQUIRE(chunk->ContainsEntity(entity));
            }

            THEN("The chunk no longer contains the entity as alive"){
                REQUIRE_FALSE(chunk->ContainsEntity(entity, true));
            }

            THEN("The space in the is unchanged"){
                REQUIRE(chunk->GetFree() == chunkFreeSpace);
            }

            THEN("The entity count is the same"){
                REQUIRE(manager->EntityCount() == 1);
            }

            AND_WHEN("Cleanup is performed"){
                manager->OnEndOfFrame();

                THEN("the entity is no longer alive") {
                    REQUIRE_FALSE(manager->IsAlive(entity));
                }

                THEN("The chunk no longer contains the entity"){
                    REQUIRE_FALSE(chunk->ContainsEntity(entity));
                }

                THEN("The space in the chunk has increased"){
                    REQUIRE(chunk->GetFree() > chunkFreeSpace);
                }

                THEN("The entity count is decreased"){
                    REQUIRE(manager->EntityCount() == 0);
                }
            }
        }
    }

    GIVEN("Enough entities to fill a chunk") {
        auto entity = Entity(-1);
        for (int i = 0; i < 10; ++i)
            entity = manager->CreateEntity();

        auto originalChunk = manager->GetChunk(entity);

        WHEN("An entity is destroyed and cleaned up"){
            manager->DestroyEntity(entity);
            manager->OnEndOfFrame();

            AND_WHEN("A new entity is created"){
                auto newEntity = manager->CreateEntity();

                THEN("No new chunk is created and the existing one is re-used"){
                    REQUIRE(manager->ChunkCount() == 1);
                    REQUIRE(originalChunk == manager->GetChunk(newEntity));
                }
            }
        }
    }
}