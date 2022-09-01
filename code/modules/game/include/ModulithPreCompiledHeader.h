#pragma once

// Core
#include <Core.h>
#include <Context.h>

// Resources
#include <modules/resources/ModuleResources.h>
#include <resources/Resource.h>
#include <resources/SubcontextResource.h>

#include <ecs/resources/SystemResource.h>
#include <ecs/resources/SystemsGroupResource.h>
#include <ecs/resources/ComponentResource.h>

#include <serialization/SerializerResource.h>

// ECS
#include <ecs/ECSContext.h>
#include <ecs/EntityManager.h>
#include <ecs/ECSUtils.h>
#include <ecs/Prefab.h>

#include <ecs/ComponentManager.h>
#include <ecs/StandardComponents.h>
#include <ecs/transform/TransformComponents.h>
#include <ecs/transform/TransformUtils.h>

#include <ecs/systems/System.h>
#include <ecs/systems/SystemsGroup.h>
#include <ecs/systems/DefaultSystemsGroups.h>

// Serialization
#include <serialization/SerializationContext.h>
#include <serialization/Serializer.h>
#include <serialization/SerializedObject.h>

// Window & Input
#include <window/Window.h>
#include <input/InputState.h>

// Modules
#include <modules/ModuleContext.h>
#include <modules/Module.h>
#include <modules/ModuleInfo.h>
#include <modules/ModuleUtils.h>
#include <modules/ModulePathUtils.h>

// Rendering
#include <renderer/Mesh.h>
#include <renderer/StandardMaterial.h>
#include <renderer/Material.h>
#include <renderer/primitives/Shader.h>
#include <renderer/primitives/Texture.h>
#include <renderer/ModelLoaderUtils.h>

// Utils
#include <utils/TypeUtils.h>
#include <utils/YamlUtils.h>
#include <utils/PreferencesContext.h>
#include <utils/FileDialog.h>

#include <imgui.h>