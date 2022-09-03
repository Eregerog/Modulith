/*
 * \brief
 * \author Daniel Götz
 */


#include "Core.h"
#include "catch.hpp"
#include "ECSTestUtils.h"
#include <ECS/EntityManager.h>

SCENARIO("Entities have alias functions for EntityManager functions", "[ecs]") {
    GIVEN("An entity manager") {
        auto ecs = CreateEntityManager();

        WHEN("A non alive entity is manually constructed") {
            auto entity = Entity(1);

            THEN("It is not alive") {
                REQUIRE_FALSE(entity.IsAlive(ecs));
            }
        }


        WHEN("An entity is created") {

            auto entity = ecs->CreateEntity();

            THEN("It is alive") {
                entity.IsAlive(ecs);
            }

            THEN("It does not have an alpha component"){
                REQUIRE_FALSE(entity.Has<AlphaTag>(ecs));
                REQUIRE(entity.Get<AlphaTag>(ecs) == nullptr);
            }

            AND_WHEN("An alpha component is added"){
                entity.Add(ecs, AlphaTag());

                THEN("It has the component"){
                    REQUIRE(entity.Has<AlphaTag>(ecs));
                    REQUIRE(entity.Get<AlphaTag>(ecs) != nullptr);
                }
            }

            AND_WHEN("The entity is destroyed and cleaned up"){
                entity.Destroy(ecs);
                ecs->OnEndOfFrame();

                THEN("It is no longer alive"){
                    REQUIRE_FALSE(entity.IsAlive(ecs));
                }
            }
        }
    }
}