/*
 * \brief
 * \author Daniel Götz
 */

#include "catch.hpp"
#include "../ECSTestUtils.h"
#include <ecs/EntityManager.h>
#include "ecs/Prefab.h"

SCENARIO("Prefabs can be created by explicitly passing the components of the prefab") {

    GIVEN("A component manager") {
        auto componentManager = CreateComponentManagerPtr();

        WHEN("A prefab is created with the alpha and beta component") {
            auto prefab = Prefab::Create(
                ref(componentManager),
                AlphaTag(),
                BetaTag()
            );

            THEN("Only the alpha and beta components are present on it") {
                REQUIRE(prefab->Has<AlphaTag>());
                REQUIRE(prefab->Has<BetaTag>());
                REQUIRE_FALSE(prefab->Has<GammaTag>());

                REQUIRE(prefab->Get<AlphaTag>() != nullptr);
                REQUIRE(prefab->Get<BetaTag>() != nullptr);
                REQUIRE(prefab->Get<GammaTag>() == nullptr);
            }
        }

        WHEN("A prefab is created with a number component with some data") {

            auto prefab = Prefab::Create(
                ref(componentManager),
                NumberData(42)
            );

            THEN("That data can be retrieved with .Get") {
                REQUIRE(prefab->Has<NumberData>());
                REQUIRE(prefab->Get<NumberData>()->Number == 42);
            }
        }
    }
}

SCENARIO("Prefabs can be created from existing entities") {

    GIVEN("An entity manager and an entity with some components") {
        auto componentManager = ref(CreateComponentManagerPtr());
        auto manager = ref(new EntityManager(componentManager));

        auto e = manager->CreateEntityWith(
            TestTag(),
            StringData{"Test"},
            NumberData(666)
        );

        WHEN("A prefab is created form the entity") {
            auto prefab = Prefab::CreateFromEntity(componentManager, manager, e);

            THEN("it has the same components with the same data as the entity") {
                REQUIRE(prefab->Has<TestTag>());
                REQUIRE(prefab->Has<StringData>());
                REQUIRE(prefab->Has<NumberData>());

                REQUIRE(prefab->Get<StringData>()->Name == "Test");
                REQUIRE(prefab->Get<NumberData>()->Number == 666);
            }

            THEN("The buffer of the prefab is the same size as the entity's chunk minus the size of entities") {
                REQUIRE(prefab->GetSize() == manager->GetChunk(e)->GetEntitySize() - sizeof(Entity));
            }
        }
    }
}

SCENARIO("Created prefabs can be instantiated to create entities from them") {
    GIVEN("A component manager, entity manager and a precreated prefab") {
        auto componentManager = CreateComponentManagerPtr();

        auto prefab = Prefab::Create(
            ref(componentManager),
            TestTag(),
            NumberData(666),
            StringData{"This is a test"}
        );

        auto entityManager = ref(new EntityManager(ref(componentManager)));

        WHEN("A prefab is instantiated") {
            auto entity = prefab->InstantiateIn(entityManager);

            THEN("The entity has the same components and data as the prefab") {
                REQUIRE(entityManager->IsAlive(entity));

                REQUIRE(entity.Has<TestTag>(entityManager));
                REQUIRE(entity.Has<NumberData>(entityManager));
                REQUIRE(entity.Has<StringData>(entityManager));

                REQUIRE(entity.Get<NumberData>(entityManager)->Number == 666);
                REQUIRE(entity.Get<StringData>(entityManager)->Name == "This is a test");
            }
        }
    }
}

SCENARIO("Prefabs correctly use RAII when dealing with resources") {
    GIVEN("A component manager") {
        auto componentManager = CreateComponentManagerPtr();

        WHEN("A prefab is created with a shared resource") {

            auto sharedResource = std::make_shared<int>(666);

            REQUIRE(sharedResource.use_count() == 1);

            {
                auto prefab = Prefab::Create(
                    ref(componentManager),
                    FirstSharedResourceData(sharedResource)
                );

                THEN("The resource count is increased after the prefab has been created") {
                    REQUIRE(sharedResource.use_count() == 2);
                }
            }

            AND_THEN("After the prefab has been destroyed, the resource count is reduced again") {
                REQUIRE(sharedResource.use_count() == 1);
            }
        }

        WHEN("A prefab is created with a shared resource") {

            auto sharedResource = std::make_shared<int>(666);

            REQUIRE(sharedResource.use_count() == 1);


            auto prefab = Prefab::Create(
                ref(componentManager),
                FirstSharedResourceData(sharedResource)
            );

            THEN("The resource count is increased after the prefab has been created") {
                REQUIRE(sharedResource.use_count() == 2);
            }


            AND_WHEN("The prefab is instantiated") {

                {
                    auto entityManager = ref(new EntityManager(ref(componentManager)));

                    auto entity = prefab->InstantiateIn(entityManager);

                    THEN("The resource count is increased again") {
                        REQUIRE(sharedResource.use_count() == 3);
                    }
                }
            }
        }

    }

    GIVEN("An entity manager and enitity with a shared resource") {
        auto componentManager = CreateComponentManagerPtr();
        auto manager = ref(new EntityManager(ref(componentManager)));

        auto sharedResource = std::make_shared<int>(666);

        REQUIRE(sharedResource.use_count() == 1);

        auto e = manager->CreateEntityWith(
            FirstSharedResourceData(sharedResource)
        );

        REQUIRE(sharedResource.use_count() == 2);

        WHEN("A prefab is created from the entity") {

            {
                auto prefab = Prefab::CreateFromEntity(ref(componentManager), manager, e);

                THEN("The use count is increased") {
                    REQUIRE(sharedResource.use_count() == 3);
                }
            }

            AND_WHEN("The prefab is deconstructed"){
                THEN("The use count back to the value it had before"){
                    REQUIRE(sharedResource.use_count() == 2);
                }
            }
        }
    }
}