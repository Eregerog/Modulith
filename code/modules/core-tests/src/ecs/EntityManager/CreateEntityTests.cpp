/*
 * \brief
 * \author Daniel Götz
 */

#include "../ECSTestUtils.h"
#include <ECS/EntityManager.h>
#include "catch.hpp"

SCENARIO("Entities can be created using CreateEntity", "[ECS]") {
    GIVEN("An entity manager") {
        auto manager = CreateEntityManager();

        WHEN("The manager is initialized") {
            THEN("there are no chunks or entities created") {
                REQUIRE(manager->EntityCount() == 0);
                REQUIRE(manager->ChunkCount() == 0);
            }
        }

        WHEN("An entity is created") {
            auto entity = manager->CreateEntity();
            auto chunk = manager->GetChunk(entity);

            THEN("the entity and chunk count are increased") {
                REQUIRE(manager->EntityCount() == 1);
                REQUIRE(manager->ChunkCount() == 1);
                REQUIRE(entity.GetId() == 1);
            }

            THEN("the chunk of the entity contains it") {
                REQUIRE(chunk->ContainsEntity(entity));
            }


            AND_WHEN("More entities than the chunk capacity are created") {
                for (int i = 0; i < chunk->GetCapacity() + 2; i++)
                    manager->CreateEntity();

                THEN("there are more than 1 chunks") {
                    REQUIRE(manager->EntityCount() > 1);
                    REQUIRE(manager->ChunkCount() == 2);
                }

                THEN("the first chunk is full") {
                    REQUIRE(manager->GetChunk(Entity(1))->GetFree() == 0);
                }
            }
        }

    }
}


SCENARIO("Entities can be created while attaching components to them using CreateEntityWith", "[ECS]") {
    GIVEN("An entity manager") {
        auto manager = CreateEntityManager();

        WHEN("An entity is created with a number component") {
            auto entity = manager->CreateEntityWith(NumberData());

            THEN("the number component is attached to the entity with default values") {
                auto hasIt = manager->HasComponents<NumberData>(entity);
                REQUIRE(hasIt);

                auto numberComponent = manager->GetComponent<NumberData>(entity);
                REQUIRE(numberComponent->Number == NumberData().Number);
            }
        }

        WHEN("An entity is created with an explicit number component") {
            auto entity = manager->CreateEntityWith(NumberData(99));

            THEN("the number component is attached to the entity with default values") {
                auto hasIt = manager->HasComponents<NumberData>(entity);
                REQUIRE(hasIt);

                auto numberComponent = manager->GetComponent<NumberData>(entity);
                REQUIRE(numberComponent->Number == 99);
            }
        }
    }
}