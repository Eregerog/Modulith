/*
 * \brief
 * \author Daniel Götz
 */


#include "Core.h"
#include "catch/catch.hpp"
#include "../ECSTestUtils.h"
#include <ECS/EntityManager.h>


SCENARIO("Components can be added to entities", "[ECS]") {
    GIVEN("An entity manager and blank entity") {
        auto manager = CreateEntityManager();
        auto entity = manager->CreateEntity();
        auto originalChunk = manager->GetChunk(entity);

        WHEN("A component is added") {
            auto component = manager->AddComponent<NumberData>(entity);
            auto newChunk = manager->GetChunk(entity);

            THEN("the variables are initialized") {
                REQUIRE(component->Number == 1);
            }

            THEN("the chunk of the entity has changed") {
                REQUIRE(originalChunk != newChunk);
            }

            THEN("the new chunk's signature contains the component") {
                REQUIRE(newChunk->GetIdentifier().count(typeid(NumberData)) == 1);
            }

            THEN("the new chunk contains the entity") {
                REQUIRE(newChunk->ContainsEntity(entity));
                REQUIRE(newChunk->GetOccupied() == 1);
            }

            THEN("the old chunk no longer contains the entity") {
                REQUIRE(!originalChunk->ContainsEntity(entity));
                REQUIRE(originalChunk->GetOccupied() == 0);
            }
        }

        WHEN("A component is added that already existed on the entity") {
            auto firstComponent = manager->AddComponent<NumberData>(entity, 55);
            auto secondComponent = manager->AddComponent<NumberData>(entity, 666);
            auto newChunk = manager->GetChunk(entity);

            THEN("the second time it is added the first value is overwritten") {
                REQUIRE(secondComponent->Number == 666);
            }

            THEN("the chunk of the entity didn't change") {
                REQUIRE(originalChunk != newChunk);
            }
        }

        WHEN("A component is added with an explicit constructor") {
            auto component = manager->AddComponent<NumberData>(entity, 42);

            THEN("the added component is initialized correctly") {
                REQUIRE(component->Number == 42);
            }
        }

        WHEN("Multiple components are added to the same entity") {
            auto numberComponent = manager->AddComponent<NumberData>(entity);
            auto intermediateChunk = manager->GetChunk(entity);
            auto stringComponent = manager->AddComponent<StringData>(entity);
            auto newChunk = manager->GetChunk(entity);

            THEN("both components are initialized correctly") {
                REQUIRE(numberComponent->Number == 1);
                REQUIRE(stringComponent->Name == "Foo");
            }

            THEN("the intermediate chunk does not contain the entity"){
                REQUIRE_FALSE(intermediateChunk->ContainsEntity(entity));
            }

            THEN("the new chunk's signature contains both components") {
                REQUIRE(newChunk->GetIdentifier().count(typeid(NumberData)) == 1);
                REQUIRE(newChunk->GetIdentifier().count(typeid(StringData)) == 1);
            }
        }
    }
}


SCENARIO("Resources of added entities are persist through creation", "[ECS]") {
    GIVEN("An entity manager and entity") {
        auto manager = CreateEntityManager();

        auto entity = manager->CreateEntity();

        WHEN("A component with a resource (unique ptr) is added") {
            auto component = manager->AddComponent<OwnedResourceData>(entity, 42);

            THEN("accessing the component's resource is not an access violation and has the proper data") {
                REQUIRE(*component->Resource == 42);
            }
        }
    }
}


SCENARIO("When components are added existing component values are moved in memory", "[ECS]") {
    GIVEN("An entity manager and an entity with a component") {
        auto manager = CreateEntityManager();
        auto entity = manager->CreateEntity();
        auto component = manager->AddComponent<NumberData>(entity);

        component->Number = 50;

        StringData* otherComponent = nullptr;
        WHEN("A new component is added") {
            otherComponent = manager->AddComponent<StringData>(entity);
            otherComponent->Name = "Bar";

            THEN("The original component value has been moved as well") {
                REQUIRE(manager->GetComponent<NumberData>(entity)->Number == 50);
            }
        }

        WHEN("Two more components are added") {
            otherComponent = manager->AddComponent<StringData>(entity);
            otherComponent->Name = "Bar";
            manager->AddComponent<TestTag>(entity);

            THEN("The first and second component values are moved to the new chunk") {
                REQUIRE(manager->GetComponent<NumberData>(entity)->Number == 50);
                REQUIRE(manager->GetComponent<StringData>(entity)->Name == "Bar");
            }
        }
    }
}

SCENARIO("Multiple components can be added using AddComponents", "[ECS]") {
    GIVEN("An entity manager and entity") {
        auto manager = CreateEntityManager();
        auto entity = manager->CreateEntity();

        WHEN("Multiple components are added at once") {
            manager->AddComponents<TestTag, NumberData>(entity);
            THEN("both components have been added") {
                REQUIRE(manager->GetComponent<TestTag>(entity) != nullptr);
                REQUIRE(manager->GetComponent<NumberData>(entity) != nullptr);
            }
        }
    }
}

SCENARIO("Components can be added directly as rvalue references", "[ECS]"){
    GIVEN("An entity manager and entity"){
        auto manager = CreateEntityManager();
        auto entity = manager->CreateEntity();

        WHEN("A components are added as rvalue") {
            manager->AddComponent(entity, NumberData(666));
            THEN("the components has been added with the same value") {
                REQUIRE(manager->GetComponent<NumberData>(entity) != nullptr);
                REQUIRE(manager->GetComponent<NumberData>(entity)->Number == 666);
            }
        }
    }
}

SCENARIO("Multiple components can be added directly as rvalue references", "[ECS]"){
    GIVEN("An entity manager and entity"){
        auto manager = CreateEntityManager();
        auto entity = manager->CreateEntity();

        WHEN("Multiple components are added as rvalues") {
            manager->AddComponents(entity, NumberData(666), StringData{"foo"});
            THEN("the components has been added with the same value") {
                REQUIRE(manager->GetComponent<NumberData>(entity) != nullptr);
                REQUIRE(manager->GetComponent<NumberData>(entity)->Number == 666);
                REQUIRE(manager->GetComponent<StringData>(entity) != nullptr);
                REQUIRE(manager->GetComponent<StringData>(entity)->Name == "foo");
            }
        }
    }
}

SCENARIO("Components can be added using the untyped variants"){
    auto ecs = CreateEntityManager();

    auto e = ecs->CreateEntity();
    auto initialChunk = ecs->GetChunk(e);
    GIVEN("A component with some trivial data"){

        WHEN("A component is added"){
            ecs->AddComponent(e, typeid(NumberData));

            THEN("The data is uninitialized"){
                REQUIRE(e.Has<NumberData>(ecs));
                REQUIRE(e.Get<NumberData>(ecs)->Number == 0);
            }

            THEN("The entity is moved into the according chunk"){
                REQUIRE(ecs->GetChunk(e) != initialChunk);
                REQUIRE(ecs->GetChunk(e)->GetIdentifier().size() == 1);
                REQUIRE(ecs->GetChunk(e)->GetIdentifier().count(typeid(NumberData)) > 0);
            }
        }
    }
}