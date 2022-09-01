/*
 * \brief
 * \author Daniel Götz
 */


#include "Core.h"
#include "catch/catch.hpp"
#include "../ECSTestUtils.h"
#include <ecs/EntityManager.h>


SCENARIO("Removed components destructors are called", "[ECS]") {
    auto manager = CreateEntityManager();


    GIVEN("A shared resource") {
        auto firstResource = std::make_shared<int>(42);
        auto secondResource = std::make_shared<int>(666);

        auto compareResource = std::make_shared<int>(69);

        REQUIRE(firstResource.use_count() == 1);
        REQUIRE(secondResource.use_count() == 1);

        WHEN("The resources are attached to components on an entity") {
            auto entityToDestroy = manager->CreateEntity();
            auto entityToLife = manager->CreateEntityWith(FirstSharedResourceData(compareResource), SecondSharedResourceData(compareResource));

            manager->AddComponents(
                entityToDestroy, FirstSharedResourceData(firstResource),
                SecondSharedResourceData(secondResource));

            REQUIRE(manager->GetChunk(entityToDestroy) == manager->GetChunk(entityToLife));

            THEN("The resource use count is increased") {
                REQUIRE(firstResource.use_count() == 2);
                REQUIRE(secondResource.use_count() == 2);

                REQUIRE(compareResource.use_count() == 3);
            }

            AND_WHEN("the component is removed using RemoveComponent") {
                manager->RemoveComponent<FirstSharedResourceData>(entityToDestroy);

                THEN("the first resource's use count is decreased again") {
                    REQUIRE(firstResource.use_count() == 1);
                }

                THEN("the second resource's use count stays the same") {
                    REQUIRE(secondResource.use_count() == 2);
                }
            }

            AND_WHEN("multiple components are removed using RemoveComponents") {
                manager->RemoveComponents<FirstSharedResourceData, SecondSharedResourceData>(entityToDestroy);

                THEN("the resource use count of both is decreased again") {
                    REQUIRE(firstResource.use_count() == 1);
                    REQUIRE(secondResource.use_count() == 1);
                }
            }

            AND_WHEN("the entity with both shared resource components is destroyed") {
                manager->DestroyEntity(entityToDestroy);

                THEN("the resource use count of both is not yet decreased again") {
                    REQUIRE(firstResource.use_count() == 2);
                    REQUIRE(secondResource.use_count() == 2);
                }

                THEN("the resource use count of the non-destroyed entity does not change"){
                    REQUIRE(compareResource.use_count() == 3);
                }

                AND_WHEN("OnEndOfFrame is called on the entity manager"){

                    manager->OnEndOfFrame();

                    THEN("the resource use count of both is decreased again") {
                        REQUIRE(firstResource.use_count() == 1);
                        REQUIRE(secondResource.use_count() == 1);
                    }

                    THEN("the resource use count of the non-destroyed entity does not change again"){
                        REQUIRE(compareResource.use_count() == 3);
                    }
                }
            }
        }
    }
}

SCENARIO("Destorying entity chunks calls destructors on all its entitie's components"){
    auto manager = CreateEntityManagerPtr();
    GIVEN("A shared resource") {
        auto firstResource = std::make_shared<int>(42);
        auto secondResource = std::make_shared<int>(666);

        WHEN("The resources are attached to components on an entity") {
            auto entityToDestroy = manager->CreateEntity();

            manager->AddComponents(
                entityToDestroy, FirstSharedResourceData(firstResource),
                SecondSharedResourceData(secondResource));

            THEN("The resource use count is increased") {
                REQUIRE(firstResource.use_count() == 2);
                REQUIRE(secondResource.use_count() == 2);
            }

            AND_WHEN("The manager is destroyed (and it's chunks subsequently)"){
                delete manager;

                THEN("The resource count is decreased"){
                    REQUIRE(firstResource.use_count() == 1);
                    REQUIRE(secondResource.use_count() == 1);
                }
            }
        }
    }
}