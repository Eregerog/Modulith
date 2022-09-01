/*
 * \brief
 * \author Daniel Götz
 */


#include "Core.h"
#include "catch/catch.hpp"
#include "../ECSTestUtils.h"
#include <ECS/EntityManager.h>


SCENARIO("Entities can be modified while iterating over them using Defer", "[ECS]") {
    auto manager = CreateEntityManager();
    GIVEN("An entity manager and entity") {

        const int entityCount = 5;

        for (int i = 0; i < entityCount; ++i) {
            auto entity = manager->CreateEntity();
            manager->AddComponent<TestTag>(entity);
        }

        WHEN("Iterating with ForEach and adding a component with defer") {
            int calls = 0;
            REQUIRE_NOTHROW(
                manager->QueryAll(
                    Each<TestTag>(), [manager, &calls](auto entity, auto& tag) {
                        manager->Defer(
                            [entity, &calls](auto manager) {
                                manager->template AddComponent<NumberData>(entity);
                                ++calls;
                            }
                        );
                    }
                )
            );

            THEN("the deferred method was called on each entity") {
                REQUIRE(calls == entityCount);
            }

            THEN("all entities have the new component") {
                manager->QueryAll(
                    Each<TestTag>(), [manager, &calls](auto entity, auto& tag) {
                        REQUIRE (manager->GetComponent<NumberData>(entity) != nullptr);
                    }
                );
            }
        }

        WHEN("Iterating with ForAny and adding a component with defer") {
            int calls = 0;
            REQUIRE_NOTHROW(
                manager->QueryAll(
                    Any<TestTag>(), [manager, &calls](auto entity, auto* tag) {
                        manager->Defer(
                            [entity, &calls](auto manager) {
                                manager->template AddComponent<NumberData>(entity);
                                ++calls;
                            }
                        );
                    }
                )
            );

            THEN("the deferred method was called on each entity") {
                REQUIRE(calls == entityCount);
            }

            THEN("all entities have the new component") {
                manager->QueryAll(
                    Each<TestTag>(), [manager, &calls](auto entity, auto& tag) {
                        REQUIRE (manager->GetComponent<NumberData>(entity) != nullptr);
                    }
                );
            }
        }

        WHEN("Iterating inside an iteration and adding a component with defer") {
            int calls = 0;
            REQUIRE_NOTHROW(
                manager->QueryAll(
                    Each<TestTag>(), [manager, &calls](auto entity, auto& tag) {
                        manager->QueryAll(
                            Each<TestTag>(), [manager, &calls](auto entity, auto& tag) {
                                manager->Defer(
                                    [entity, &calls](auto manager) {
                                        ++calls;
                                        manager->template AddComponent<NumberData>(entity);
                                    }
                                );
                            }
                        );
                    }
                )
            );

            THEN("the deferred method was called the correct number of times") {
                REQUIRE(calls == entityCount * entityCount);
            }

            THEN("all entities have the new component") {
                manager->QueryAll(
                    Each<TestTag>(), [manager, &calls](auto entity, auto& tag) {
                        REQUIRE (manager->GetComponent<NumberData>(entity) != nullptr);
                    }
                );
            }

        }
    }
}