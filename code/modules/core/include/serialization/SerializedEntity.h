/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include <ecs/Entity.h>
#include <ecs/ECSContext.h>
#include <Context.h>
#include "SerializationContext.h"
#include "SerializedObject.h"

namespace modulith {
    /**
     * Serializable representation of an entity and its components
     */
    class CORE_API SerializedEntity {

        friend YAML::convert<SerializedEntity>;

    public:
        /**
         * Creates a serialized entity from the given entity which has to be present in the entity manager of the context
         */
        static SerializedEntity From(Entity e);

        /**
         * Instantiates a copy of the serialized entity in the context's entity manager
         */
        Entity Instantiate(){
            auto ecsCtx = Context::GetInstance<ECSContext>();
            auto serializationCtx = Context::GetInstance<SerializationContext>();
            auto componentManager = ecsCtx->GetComponentManager();

            auto componentsWithTypes = std::vector<std::tuple<SerializedObject, ComponentIdentifier, RegisteredComponent>>();
            auto signature = SignatureIdentifier();
            for(auto& component : _components) {
                if (auto info = componentManager->TryFindByFullName(component.GetName())) {
                    signature.emplace(info->GetIdentifier());
                    componentsWithTypes.emplace_back(component, info->GetIdentifier(), info.value());
                }else{
                    // Error Handling: The full name of the type has changed
                }
            }

            auto [entity, chunk] = ecsCtx->GetEntityManager()->CreateEntityBy(signature);

            for(auto [serializedObject, type, info] : componentsWithTypes){
                if(auto serializable = serializationCtx->TryGetSerializableFor(type)){
                    if(auto deserialized = serializable.value()->TryDeserialize(serializedObject)){
                        if(info.IsSerializable()){
                            CoreLogInfo("Deserializing {}", type.get().name())
                            CoreAssert(deserialized->type() == info.GetIdentifier(), "Type mismatch during deserialization. Expected: {}, but was {}", info.GetIdentifier().get().name(), deserialized->type().name())
                            CoreAssert(deserialized.value().has_value(), "The deserialized object had no value, this should not happen")
                            info.CopyFromAnyToPointer(deserialized.value(), chunk->GetComponentPtr(entity, type));
                        }
                        else{
                            // Error Handling: The type was serializable before, now it is not
                        }
                    }else{
                        // Error Handling: The value could not be deserialized (e.g. corrupt / out-of-date)
                    }
                }else{
                    // Error Handling: The type had a serializable before, now it has not
                }
            }

            return entity;
        }

    private:
        Entity _self = Entity::Invalid();
        std::vector<SerializedObject> _components{};
    };

}

namespace YAML {

    // Allows the serialization of modulith::SerializedEntity

    template<>
    struct convert<SerializedEntity> {
        static Node encode(const SerializedEntity& val) {
            Node res;
            res["Entity"] = val._self;

            Node components;

            for (auto& component : val._components) {
                components.push_back(component);
            }

            res["Components"] = components;

            return res;
        }

        static bool decode(const Node& node, SerializedEntity& res) {

            try{
                res._self = node["Entity"].as<Entity>();
                for(auto& component : node["Components"]){
                    res._components.push_back(component.as<SerializedObject>());
                }
                return true;
            }catch(YAML::InvalidNode&){
                return false;
            }
        }
    };
}