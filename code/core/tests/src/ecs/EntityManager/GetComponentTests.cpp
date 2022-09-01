/*
 * \brief
 * \author Daniel Götz
 */

#include "Core.h"
#include "catch/catch.hpp"
#include "../ECSTestUtils.h"
#include <ECS/EntityManager.h>

SCENARIO("Added components can be retrieved using GetComponent", "[ECS]") {
    GIVEN("An entity manager and an entity with a component") {
        auto manager = CreateEntityManager();
        auto entity = manager->CreateEntity();
        auto component = manager->AddComponent<NumberData>(entity);

        component->Number = 50;

        WHEN("GetComponent is called on the entity for an attached component") {
            auto queriedComponent = manager->GetComponent<NumberData>(entity);

            THEN("the returned value is not null"){
                REQUIRE(queriedComponent != nullptr);
            }

            THEN("the component is the same as the added one") {
                REQUIRE(queriedComponent->Number == 50);
            }
        }

        WHEN("GetComponent is called on the entity for an non-attached component") {
            auto queriedComponent = manager->GetComponent<TestTag>(entity);

            THEN("the returned value is null") {
                REQUIRE(queriedComponent == nullptr);
            }
        }
    }
}

SCENARIO("Modifications made to components using GetComponent are persistent") {
    GIVEN("An entity manager and entity with component") {
        auto manager = CreateEntityManager();
        auto entity = manager->CreateEntity();
        manager->AddComponent<NumberData>(entity);
        auto component = manager->GetComponent<NumberData>(entity);

        WHEN("The value of the component is modified") {
            component->Number = 42;

            THEN("Calling GetComponent retrieves a component with that number") {
                auto testComponent = manager->GetComponent<NumberData>(entity);
                REQUIRE(testComponent->Number == 42);
            }
        }
    }
}

