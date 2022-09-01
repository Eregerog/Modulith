/*
 * \brief
 * \author Daniel Götz
 */

#include "Core.h"
#include "catch/catch.hpp"
#include "../ECSTestUtils.h"
#include <ECS/EntityManager.h>

SCENARIO("For can be used to query components", "[ECS]") {
    auto manager = CreateEntityManager();

    GIVEN("No entities") {
        WHEN("Any component is queried") {
            auto wasCalled = false;

            manager->QueryAll(
                Each<TestTag>(), [&wasCalled](auto entity, auto& tag) {
                    REQUIRE(false);
                    wasCalled = true;
                }
            );

            manager->QueryAll(
                Each<TestTag>(), [](auto entity, TestTag& tag) {
                    REQUIRE(false);
                }
            );

            THEN("the lambda is never called") {
                REQUIRE_FALSE(wasCalled);
            }
        }
    }

    GIVEN("Entities with no components") {
        const int entityCount = 5;
        std::unordered_set<Entity, EntityHasher> remainingEntities;

        for (auto i = 0; i < entityCount; ++i) {
            auto e = manager->CreateEntity();
            manager->AddComponent<TestTag>(e);
            remainingEntities.insert(e);
        }

        WHEN("An For query without constraints is made") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any(), None(), [&calls, &remainingEntities](auto entity) {
                    REQUIRE(remainingEntities.count(entity) == 1);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
//       FOR EACH
// ---------------------------------------------------------------------------------------------------------------------

SCENARIO("The EACH constraint defines a set components that all need to be present on the entity") {
    auto manager = CreateEntityManager();

    GIVEN("Multiple entities with only the TagComponent") {
        const int entityCount = 5;
        std::unordered_set<Entity, EntityHasher> remainingEntities;

        for (auto i = 0; i < entityCount; ++i) {
            auto e = manager->CreateEntity();
            manager->AddComponent<TestTag>(e);
            remainingEntities.insert(e);
        }

        WHEN("Each entity with a TagComponent is queried") {
            int calls = 0;
            manager->QueryAll(
                Each<TestTag>(), [&calls, &remainingEntities](auto entity, auto& tag) {
                    REQUIRE(remainingEntities.count(entity) == 1);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }

        WHEN("Each entity with a TagComponent and NumberData is queried") {
            int calls = 0;
            manager->QueryAll(
                Each<TestTag, NumberData>(),
                [&calls, &remainingEntities](auto entity, auto& tag, auto& number) {
                    REQUIRE(false);
                    ++calls;
                }
            );

            THEN("the lambda is never called") {
                REQUIRE(calls == 0);
            }
        }
    }

    GIVEN("Multiple entities with the TagComponent and NumberData") {
        const int entityCount = 5;
        std::unordered_set<Entity, EntityHasher> remainingEntities;

        for (auto i = 0; i < entityCount; ++i) {
            auto e = manager->CreateEntity();
            manager->AddComponent<TestTag>(e);
            manager->AddComponent<NumberData>(e);
            remainingEntities.insert(e);
        }

        WHEN("Each entity with a TagComponent is queried") {
            int calls = 0;
            manager->QueryAll(
                Each<TestTag>(), [&calls, &remainingEntities](auto entity, auto& tag) {
                    REQUIRE(remainingEntities.count(entity) == 1);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }

        WHEN("Each entity with a TagComponent and NumberData is queried") {
            int calls = 0;
            manager->QueryAll(
                Each<TestTag, NumberData>(),
                [&calls, &remainingEntities](auto entity, auto& tag, auto& number) {
                    REQUIRE(remainingEntities.count(entity) == 1);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
//       FOR ANY
// ---------------------------------------------------------------------------------------------------------------------

SCENARIO("The ANY constraint defines a set components, at least one of which needs to be present on the entity") {
    auto manager = CreateEntityManager();

    GIVEN("Multiple entities with only the TagComponent") {
        const int entityCount = 5;
        std::unordered_set<Entity, EntityHasher> remainingEntities;

        for (auto i = 0; i < entityCount; ++i) {
            auto e = manager->CreateEntity();
            manager->AddComponent<TestTag>(e);
            remainingEntities.insert(e);
        }

        WHEN("Each entity with a TagComponent is queried") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any<TestTag>(), None(), [&calls, &remainingEntities](auto entity, auto* tag) {
                    REQUIRE(tag != nullptr);
                    REQUIRE(remainingEntities.count(entity) == 1);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }

        WHEN("Each entity with a TagComponent and NumberData is queried") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any<TestTag, NumberData>(), None(),
                [&calls, &remainingEntities](auto entity, auto* tag, auto* number) {
                    REQUIRE(tag != nullptr);
                    REQUIRE(number == nullptr);
                    REQUIRE(remainingEntities.count(entity) == 1);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }
    }

    GIVEN("Multiple entities with the TagComponent and NumberData") {
        const int entityCount = 5;
        std::unordered_set<Entity, EntityHasher> remainingEntities;

        for (auto i = 0; i < entityCount; ++i) {
            auto e = manager->CreateEntity();
            manager->AddComponent<TestTag>(e);
            manager->AddComponent<NumberData>(e);
            remainingEntities.insert(e);
        }

        WHEN("Each entity with a TagComponent is queried") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any<TestTag>(), None(), [&calls, &remainingEntities](auto entity, auto* tag) {
                    REQUIRE(tag != nullptr);
                    REQUIRE(remainingEntities.count(entity) == 1);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }

        WHEN("Each entity with a TagComponent and NumberData is queried") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any<TestTag, NumberData>(), None(),
                [&calls, &remainingEntities](auto entity, auto* tag, auto* number) {
                    REQUIRE(tag != nullptr);
                    REQUIRE(number != nullptr);
                    REQUIRE(remainingEntities.count(entity) == 1);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }
    }
}


// ---------------------------------------------------------------------------------------------------------------------
//       FOR NONE
// ---------------------------------------------------------------------------------------------------------------------

SCENARIO("The NONE constraint defines a set components, none of which may to be present on the entity") {
    auto manager = CreateEntityManager();

    GIVEN("Multiple entities with only the TagComponent") {
        const int entityCount = 5;
        std::unordered_set<Entity, EntityHasher> remainingEntities;

        for (auto i = 0; i < entityCount; ++i) {
            auto e = manager->CreateEntity();
            manager->AddComponent<TestTag>(e);
            remainingEntities.insert(e);
        }

        WHEN("Each entity without a TagComponent is queried") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any(), None<TestTag>(), [&calls, &remainingEntities](auto entity) {
                    REQUIRE(false);
                    ++calls;
                }
            );

            THEN("the lambda was never called") {
                REQUIRE(calls == 0);
            }
        }

        WHEN("Each entity without a NumberData is queried") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any(), None<NumberData>(), [&calls, &remainingEntities](auto entity) {
                    REQUIRE(remainingEntities.count(entity) == 1);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }

        WHEN("Each entity without a TagComponent and NumberData is queried") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any(), None<TestTag, NumberData>(),
                [&calls, &remainingEntities](auto entity) {
                    REQUIRE(false);
                    ++calls;
                }
            );

            THEN("the lambda was never called") {
                REQUIRE(calls == 0);
            }
        }
    }

    GIVEN("Multiple entities with the TagComponent and NumberData") {
        const int entityCount = 5;
        std::unordered_set<Entity, EntityHasher> remainingEntities;

        for (auto i = 0; i < entityCount; ++i) {
            auto e = manager->CreateEntity();
            manager->AddComponent<TestTag>(e);
            manager->AddComponent<NumberData>(e);
            remainingEntities.insert(e);
        }

        WHEN("Each entity without a TagComponent is queried") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any(), None<TestTag>(), [&calls, &remainingEntities](auto entity) {
                    REQUIRE(false);
                    ++calls;
                }
            );

            THEN("the lambda was never called") {
                REQUIRE(calls == 0);
            }
        }

        WHEN("Each entity without a TagComponent and NumberData is queried") {
            int calls = 0;
            manager->QueryAll(
                Each(), Any(), None<TestTag, NumberData>(),
                [&calls, &remainingEntities](auto entity) {
                    REQUIRE(false);
                    ++calls;
                }
            );

            THEN("the lambda was never called") {
                REQUIRE(calls == 0);
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
//       FOR HAS
// ---------------------------------------------------------------------------------------------------------------------


SCENARIO("The HAS constraint can be used to check if a component is attached") {
    auto manager = CreateEntityManager();

    GIVEN("A set of entities that all have the alpha and delta component") {
        const int entityCount = 5;
        std::unordered_set<Entity, EntityHasher> remainingEntities;

        for (auto i = 0; i < entityCount; ++i) {
            auto e = manager->CreateEntityWith(AlphaTag(), DeltaTag());
            remainingEntities.insert(e);
        }


        WHEN("Entites with Has<AlphaComponent> are queried, true is returned") {
            int calls = 0;
            manager->QueryAll(
                Each(),
                Any(),
                None(),
                Has<AlphaTag>(),
                [&calls, &remainingEntities](auto entity, bool hasAlpha) {
                    REQUIRE(hasAlpha);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("and the lambda is called for each entity in the first set") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }

        WHEN("Entites with Has<BetaComponent> are queried, false is returned") {
            int calls = 0;
            manager->QueryAll(
                Each(),
                Any(),
                None(),
                Has<BetaTag>(),
                [&calls, &remainingEntities](auto entity, bool hasBeta) {
                    REQUIRE_FALSE(hasBeta);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("and the lambda is called for each entity in the first set") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }

        WHEN("Entites with Has<AlphaComponent, BetaComponent, GammaComponent, DeltaComponent> are queried, (true, false, false, true) is returned") {
            int calls = 0;
            manager->QueryAll(
                Each(),
                Any(),
                None(),
                Has<AlphaTag, BetaTag, GammaTag, DeltaTag>(),
                [&calls, &remainingEntities](auto entity, bool hasAlpha, bool hasBeta, bool hasGamma, bool hasDelta) {
                    REQUIRE(hasAlpha);
                    REQUIRE_FALSE(hasBeta);
                    REQUIRE_FALSE(hasGamma);
                    REQUIRE(hasDelta);
                    remainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("and the lambda is called for each entity in the first set") {
                REQUIRE(calls == entityCount);
                REQUIRE(remainingEntities.empty());
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
//       FOR EACH + ANY + NONE + HAS
// ---------------------------------------------------------------------------------------------------------------------

SCENARIO("The EACH, ANY and NONE constraints can be combined") {
    auto manager = CreateEntityManager();

    GIVEN("Two sets of entities: The first has alpha and beta, the second has alpha, gamma and delta") {
        const int firstSetEntityCount = 5;
        std::unordered_set<Entity, EntityHasher> firstSetRemainingEntities;

        for (auto i = 0; i < firstSetEntityCount; ++i) {
            auto e = manager->CreateEntity();
            manager->AddComponent<AlphaTag>(e);
            manager->AddComponent<BetaTag>(e);
            firstSetRemainingEntities.insert(e);
        }

        const int secondSetEntityCount = 5;
        std::unordered_set<Entity, EntityHasher> secondSetRemainingEntities;

        for (auto i = 0; i < secondSetEntityCount; ++i) {
            auto e = manager->CreateEntity();
            manager->AddComponent<AlphaTag>(e);
            manager->AddComponent<GammaTag>(e);
            manager->AddComponent<DeltaTag>(e);
            secondSetRemainingEntities.insert(e);
        }

        WHEN("Each entity with alpha and either beta or gamma but no delta is queried") {
            int calls = 0;
            manager->QueryAll(
                Each<AlphaTag>(),
                Any<BetaTag, GammaTag>(),
                None<DeltaTag>(),
                [&calls, &firstSetRemainingEntities](auto entity, auto& alpha, auto* beta, auto* gamma) {
                    REQUIRE(beta != nullptr);
                    REQUIRE(gamma == nullptr);
                    REQUIRE(firstSetRemainingEntities.count(entity) == 1);
                    firstSetRemainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity in the first set") {
                REQUIRE(calls == firstSetEntityCount);
                REQUIRE(firstSetRemainingEntities.empty());
            }
        }

        WHEN("Each entity with alpha, gamma and delta but no beta is queried") {
            int calls = 0;
            manager->QueryAll(
                Each<AlphaTag, GammaTag>(),
                Any<DeltaTag>(),
                None<BetaTag>(),
                [&calls, &secondSetRemainingEntities](auto entity, auto& alpha, auto& gamma, auto* delta) {
                    REQUIRE(delta != nullptr);
                    REQUIRE(secondSetRemainingEntities.count(entity) == 1);
                    secondSetRemainingEntities.erase(entity);
                    ++calls;
                }
            );

            THEN("the lambda is called for each entity in the second set") {
                REQUIRE(calls == secondSetEntityCount);
                REQUIRE(secondSetRemainingEntities.empty());
            }
        }

        WHEN("Each entity with alpha and either gamma or delta is queried and test for delta") {
            int calls = 0;
            manager->QueryAll(
                Each<AlphaTag>(),
                Any<BetaTag, GammaTag>(),
                None(),
                Has<DeltaTag>(),
                [&calls, &firstSetRemainingEntities, &secondSetRemainingEntities](
                    auto entity, auto& alpha, auto* beta, auto* gamma, bool hasDelta
                ) {
                    ++calls;
                    if (firstSetRemainingEntities.count(entity) == 1) {
                        REQUIRE_FALSE(hasDelta);
                        firstSetRemainingEntities.erase(entity);
                    } else if (secondSetRemainingEntities.count(entity) == 1) {
                        REQUIRE(hasDelta);
                        secondSetRemainingEntities.erase(entity);
                    } else
                        REQUIRE(false);
                }
            );

            THEN("the lambda is called for each entity in both sets") {
                REQUIRE(calls == firstSetEntityCount + secondSetEntityCount);
                REQUIRE(firstSetRemainingEntities.empty());
                REQUIRE(secondSetRemainingEntities.empty());
            }
        }
    }
}