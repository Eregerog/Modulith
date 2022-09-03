/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include <ecs/ComponentManager.h>
#include <ecs/EntityManager.h>
#include <ecs/transform/TransformComponents.h>

using namespace modulith;

struct AlphaTag {

};

struct BetaTag {

};

struct GammaTag {

};

struct DeltaTag {

};


struct NumberData {
    NumberData() = default;

    explicit NumberData(int number) : Number(number) {}

    NumberData(const NumberData& nb) : Number(nb.Number){}

    int Number = 1;
};

struct StringData {
    std::string Name = "Foo";
};

struct TestTag {
};


struct OwnedResourceData {
    explicit OwnedResourceData(int value) : Resource(std::make_unique<int>(value)) {}
    std::unique_ptr<int> Resource;
};

struct FirstSharedResourceData{
    explicit FirstSharedResourceData(const std::shared_ptr<int>& resource) : Resource(resource) {}

    std::shared_ptr<int> Resource;
};


struct SecondSharedResourceData{
    explicit SecondSharedResourceData(const std::shared_ptr<int>& resource) : Resource(resource) {}

    std::shared_ptr<int> Resource;
};

inline void registerTestingComponentsOn(ComponentManager* componentManager){
    componentManager->RegisterComponents(ComponentInfo::Create<AlphaTag>("Tests", "Alpha"));
    componentManager->RegisterComponents(ComponentInfo::Create<BetaTag>("Tests", "Beta"));
    componentManager->RegisterComponents(ComponentInfo::Create<GammaTag>("Tests", "Gamma"));
    componentManager->RegisterComponents(ComponentInfo::Create<DeltaTag>("Tests", "Delta"));

    componentManager->RegisterComponents(ComponentInfo::Create<NumberData>("Tests", "Number"));
    componentManager->RegisterComponents(ComponentInfo::Create<StringData>("Tests", "String"));
    componentManager->RegisterComponents(ComponentInfo::Create<TestTag>("Tests", "Test"));

    componentManager->RegisterComponents(ComponentInfo::Create<OwnedResourceData>("Tests", "OwnedResource"));
    componentManager->RegisterComponents(ComponentInfo::Create<FirstSharedResourceData>("Tests", "FirstSharedResource"));
    componentManager->RegisterComponents(
        ComponentInfo::Create<SecondSharedResourceData>("Tests", "SecondSharedResource"));

    // Needed because EntityManager.DestroyEntity checks for it
    componentManager->RegisterComponents(
        ComponentInfo::Create<WithChildrenData>("Tests", "WithChildrenData"));
}

inline ComponentManager* CreateComponentManagerPtr(){
    auto componentManager = new ComponentManager(); // this leaks, but that is fine because these are tests
    registerTestingComponentsOn(componentManager);
    return componentManager;
}

inline shared<ComponentManager> CreateComponentManager(){
    auto componentManager = std::make_shared<ComponentManager>();
    registerTestingComponentsOn(componentManager.get());
    return componentManager;
}

inline EntityManager* CreateEntityManagerPtr(){
    return new EntityManager(ref(CreateComponentManagerPtr()));
}

inline ref<EntityManager> CreateEntityManager(){
    return ref(CreateEntityManagerPtr()); // this leaks, but that is fine because these are tests
}
