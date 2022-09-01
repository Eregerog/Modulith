/*
 * \brief
 * \author Daniel Götz
 */


#include "Core.h"
#include "catch/catch.hpp"
#include "../ECSTestUtils.h"
#include <ECS/EntityManager.h>

SCENARIO("Components can be removed using RemoveComponent") {
    auto manager = CreateEntityManager();

    GIVEN("An entity with no components") {
        auto entity = manager->CreateEntity();

        WHEN("The tag component is removed from it") {
            auto wasRemoved = manager->RemoveComponent<TestTag>(entity);

            THEN("false is returned, since there was no component to remove") {
                REQUIRE_FALSE(wasRemoved);
            }
        }
    }

    GIVEN("An entity with a component") {
        auto entity = manager->CreateEntity();
        manager->AddComponent(entity, TestTag());
        auto currentChunk = manager->GetChunk(entity);
        auto currentChunkFreeSpace = currentChunk->GetFree();

        WHEN("The tag component is removed") {
            auto wasRemoved = manager->RemoveComponent<TestTag>(entity);
            auto newChunk = manager->GetChunk(entity);

            THEN("true is returned since the component was removed") {
                REQUIRE(wasRemoved);
            }

            THEN("the chunk of the entity has changed") {
                REQUIRE(currentChunk != newChunk);
            }

            THEN("the new chunk does not contain the removed component") {
                REQUIRE_FALSE(newChunk->ContainsComponent(typeid(TestTag)));
            }

            THEN("the free space in the old chunk has increased") {
                REQUIRE(currentChunk->GetFree() == currentChunkFreeSpace + 1);
            }
        }
    }
}

SCENARIO("Remove components can be used to remove multiple components at a time", "[ECS]") {
    auto manager = CreateEntityManager();

    GIVEN("An entity with no components") {
        auto entity = manager->CreateEntity();

        WHEN("Multiple components are removed from it") {
            auto wasRemoved = manager->RemoveComponents<TestTag, NumberData>(entity);

            THEN("false is returned, since there was no component to remove") {
                REQUIRE_FALSE(wasRemoved);
            }
        }
    }


    GIVEN("An entity with multiple components") {
        auto entity = manager->CreateEntity();
        manager->AddComponents(entity,
                              TestTag(),
                              NumberData(),
                              StringData()
        );

        auto currentChunk = manager->GetChunk(entity);
        auto currentChunkFreeSpace = currentChunk->GetFree();

        WHEN("The tag component is removed") {
            auto wasRemoved = manager->RemoveComponents<TestTag, NumberData>(entity);
            auto newChunk = manager->GetChunk(entity);

            THEN("true is returned since a component was removed") {
                REQUIRE(wasRemoved);
            }

            THEN("the chunk of the entity has changed") {
                REQUIRE(currentChunk != newChunk);
            }

            THEN("the new chunk does not contain the removed components") {
                REQUIRE_FALSE(newChunk->ContainsComponent(typeid(TestTag)));
                REQUIRE_FALSE(newChunk->ContainsComponent(typeid(NumberData)));
            }

            THEN("the new chunk still contains the non-removed component") {
                REQUIRE(newChunk->ContainsComponent(typeid(StringData)));
            }

            THEN("the free space in the old chunk has increased") {
                REQUIRE(currentChunk->GetFree() == currentChunkFreeSpace + 1);
            }
        }
    }
}

SCENARIO("Components can be removed using the untyped variant"){
    auto manager = CreateEntityManager();


    GIVEN("An entity with a number component"){
        auto e = manager->CreateEntityWith(NumberData(42));

        auto initialChunk = manager->GetChunk(e);

        WHEN("The component is removed"){
            auto successful = manager->RemoveComponent(e, typeid(NumberData));
            auto newChunk = manager->GetChunk(e);

            THEN("The component removal was successful"){
                REQUIRE(successful == true);
            }

            THEN("The entity is in a different chunk"){
                REQUIRE(initialChunk != newChunk);
                REQUIRE(newChunk->GetComponentPtr(e, typeid(NumberData)) == nullptr);
            }

            AND_WHEN("The component is removed again"){
                auto successfulAgain = manager->RemoveComponent(e, typeid(NumberData));
                auto chunkAfterSecondRemoval = manager->GetChunk(e);

                THEN("That operation is not successful"){
                    REQUIRE_FALSE(successfulAgain);
                }

                THEN("The chunk does not change"){
                    REQUIRE(chunkAfterSecondRemoval == newChunk);
                }
            }
        }
    }

    GIVEN("An entity with a shared resource component"){

        auto sharedResource = std::make_shared<int>(42);

        auto e = manager->CreateEntityWith(FirstSharedResourceData(sharedResource));
        REQUIRE(e.Get<FirstSharedResourceData>(manager)->Resource == sharedResource);
        REQUIRE(sharedResource.use_count() == 2);

        WHEN("The component is removed"){
            auto successful = manager->RemoveComponent(e, typeid(FirstSharedResourceData));
            REQUIRE(successful);

            THEN("The use count is appropriately decreased which means that the component has been destructed"){
                REQUIRE(sharedResource.use_count() == 1);
            }
        }
    }
}