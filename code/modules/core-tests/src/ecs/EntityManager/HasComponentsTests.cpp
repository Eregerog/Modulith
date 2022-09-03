/*
 * \brief
 * \author Daniel Götz
 */

#include "Core.h"
#include "catch.hpp"
#include "../ECSTestUtils.h"
#include <ECS/EntityManager.h>

SCENARIO("HasComponents returns if the entity has all of the given components", "[ECS]"){
    auto manager = CreateEntityManager();

    GIVEN("An entity with a number component"){
        auto entity = manager->CreateEntity();
        manager->AddComponent<NumberData>(entity);

        WHEN("The number component is queried"){
            auto hasIt = manager->HasComponents<NumberData>(entity);

            THEN("true is returned"){
                REQUIRE(hasIt);
            }
        }

        WHEN("The string component is queried"){
            auto hasIt = manager->HasComponents<StringData>(entity);

            THEN("false is returned"){
                REQUIRE_FALSE(hasIt);
            }
        }

        WHEN("Both the string component and number component are queried"){
            auto hasThem = manager->HasComponents<StringData, NumberData>(entity);

            THEN("false is returned"){
                REQUIRE_FALSE(hasThem);
            }
        }

        WHEN("The string component is added"){

            manager->AddComponent<StringData>(entity);

            AND_WHEN("Both the string component and number component are queried"){
                auto hasThem = manager->HasComponents<StringData, NumberData>(entity);

                THEN("true is returned"){
                    REQUIRE(hasThem);
                }
            }
        }

        WHEN("The entity is destroyed and cleaned up"){
            manager->DestroyEntity(entity);
            manager->OnEndOfFrame();

            AND_WHEN("Any component is queried"){
                auto hasNumber = manager->HasComponents<NumberData>(entity);
                auto hasString = manager->HasComponents<StringData>(entity);

                auto hasNumberAndString = manager->HasComponents<NumberData, StringData>(entity);

                THEN("all queries return false"){
                    REQUIRE_FALSE(hasNumber);
                    REQUIRE_FALSE(hasString);
                    REQUIRE_FALSE(hasNumberAndString);
                }
            }
        }
    }
}