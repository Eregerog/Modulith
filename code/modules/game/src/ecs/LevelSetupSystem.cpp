/*
 * \brief
 * \author Daniel Götz
 */

#include "ecs/LevelSetupSystem.h"
#include "RenderUtils.h"
#include "PhysicsComponents.h"
#include "ecs/commands/CommandComponents.h"
#include "ecs/Components.h"
#include "renderer/RenderContext.h"

using namespace modulith;
using namespace modulith::renderer;
using namespace modulith::physics;

void LevelSetupSystem::OnInitialize() {

    auto& ctx = Context::Instance();

    auto renderCtx = ctx.Get<RenderContext>();
    auto standardShader = renderCtx->Shaders()->Load(Address() / "shaders" / "PhongShader.glsl");

    auto ecsCtx = ctx.Get<modulith::ECSContext>();
    auto ecs = ecsCtx->GetEntityManager();
    auto buildingSystem = ctx.Get<BuildingContext>();

    // Models

    _gravestoneModels = ModelLoaderUtils::LoadSceneFromFile(Address() / "gravestone" / "gravestone.obj");
    _deadTree = ModelLoaderUtils::LoadSceneFromFile(Address() / "dead tree" / "tree.obj");

    // Clocktower

    auto clocktower = ecs->CreateEntityWith(
        NameData("Clocktower"),
        PositionData(),
        LureEffectData{ 30 },
        BoxColliderData(float3(4.6f,20,4.6), float3(0, 10, 0))
    );

    auto clocktowerModel = ecs->CreateEntityWith(
        NameData("Clocktower Model"),
        PositionData(-1.4, 0, -1.4f),
        WithParentData(clocktower),
        RotationData(-90, float3(1,0,0)),
        ScaleData(float3(0.2f, 0.2f, 0.2f))
    );

    RenderUtils::InstantiateModelsUnder(ecs, clocktowerModel, ModelLoaderUtils::LoadSceneFromFile(Address() / "clocktower" / "clocktower.obj"));

    auto clocktowerLight = ecs->CreateEntityWith(NameData("Clocktower Light"), PositionData(0, 10, 0), PointLightData(float3(1.000f, 1.000f, 0.157f), 14.0f));


    // Floor

    auto floorEntity = ecs->CreateEntityWith(NameData("Floor"), GroundTag());
    auto floorRenderMesh = ecs->AddComponent<RenderMeshData>(floorEntity, modulith::Mesh::CreateQuad());
    floorRenderMesh->Material = std::make_shared<modulith::StandardMaterial>(
        standardShader, modulith::float4(0.082f, 0.145f, 0.196f, 1.000f), 0.2f, 32
    );


    ecs->AddComponents(
        floorEntity,
        modulith::PositionData(0.0f, 0.0f, 0.0f),
        modulith::ScaleData(200.0f, 0.01f, 200.0f),
        BoxColliderData(),
        GroundTag()
    );

    // Moon

    ecs->CreateEntityWith(
        NameData("Moon"),
        modulith::RotationData(60.0f, modulith::float3{1.0f, 0.0f, 0.0f}),
        DirectionalLightData(modulith::float3(0.10f, 0.250f, 0.330f), 0.3f)
    );

    SpawnGravestoneCluster(float3(45, 0, 45), float3(0, 0 , 0));
    SpawnGravestoneCluster(float3(-45, 0, 45), float3(0, 0 , 0));
    SpawnGravestoneCluster(float3(-45, 0, -45), float3(0, 0 , 0));
    SpawnGravestoneCluster(float3(45, 0, -45), float3(0, 0 , 0));
}


Entity LevelSetupSystem::CreateDeadTree(float3 position, float rotation) {
    auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();
    auto tree = ecs->CreateEntityWith(
        NameData("Dead Tree"),
        PositionData(float3(position.x, position.y - 0.1f, position.z)),
        RotationData( rotation, float3(0,1,0)),
        ScaleData(float3(1.6f)),
        BoxColliderData(float3(0.8f, 3.0f, 0.8f), float3(0, 1.5f, 0.0f))
    );
    RenderUtils::InstantiateModelsUnder(ecs, tree, _deadTree);
    return tree;
}


Entity LevelSetupSystem::CreateGravestone(float3 position, float3 lookAt) {
    auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();
    auto rotationData = RotationData();
    rotationData.SetLookAt(lookAt - position);
    auto gravestone = ecs->CreateEntityWith(
        NameData("Gravestone"),
        PositionData(float3(position.x, position.y - 0.1f, position.z)),
        std::move(rotationData),
        ScaleData(float3(0.9f)),
        BoxColliderData(float3(1.0f, 1.2f, 0.4f), float3(0,0.6f,0))
    );
    RenderUtils::InstantiateModelsUnder(ecs, gravestone, _gravestoneModels);
    return gravestone;
}


modulith::Entity LevelSetupSystem::SpawnGravestoneCluster(float3 position, float3 lookAt) {
    auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();
    auto rotationData = RotationData();
    rotationData.SetLookAt(lookAt - position);
    auto cluster = ecs->CreateEntityWith(NameData("Gravestone Cluster"), PositionData(position), std::move(rotationData), SpawnEnemiesTag());

    const auto startingAngle = 60.0f;
    const auto angleIncrement = 60.0f;
    const auto radius = 3.5f;

    for(int graves = 0; graves < 5; ++graves){
        auto currentAngle = startingAngle + (angleIncrement * graves);
        auto gravestone = CreateGravestone(
            float3(-std::sin(glm::radians(currentAngle)) * radius, 0.0f, -std::cos(glm::radians(currentAngle)) * radius),
            float3(0.0f)
        );
        gravestone.Add(ecs, WithParentData(cluster));
    }

    auto deadTree = CreateDeadTree(float3(0,0,0), 15.0f);
    deadTree.Add(ecs, WithParentData(cluster));

    return cluster;
}
