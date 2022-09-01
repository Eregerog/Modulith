/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include "utils/TypeUtils.h"
#include "ecs/StandardComponents.h"
#include "SerializedObject.h"
#include "ecs/transform/TransformComponents.h"
#include "Serializable.h"

namespace modulith {

    /**
     * This context associates DynamicSerializable with types, allowing them to be trivially constructed, serialized and deserialized
     */
    class CORE_API SerializationContext : public Subcontext {

    public:
        SerializationContext() : Subcontext("Serialization Context") {}

        /**
         * Registers the functions to create a DynamicSerializable for the given type
         * @tparam T The type to serialize, it mustn't already have a DynamicSerializable registered
         * @param construct The strongly-typed version of { @link DynamicSerializable.Construct() }
         * @param serialize The strongly-typed version of { @link DynamicSerializable.Serialize() }
         * @param tryDeserialize The strongly-typed version of { @link DynamicSerializable.TryDeserialize() }
         */
        template<class T>
        void RegisterSerializableFor(
            std::function<T()> construct,
            std::function<SerializedObject(const T&)> serialize,
            std::function<std::optional<T>(const SerializedObject&)> tryDeserialize
        ) {
            _serializationFunctions.emplace(
                typeid(T), std::make_shared<Serializable<T>>(construct, serialize, tryDeserialize));
        }


        /**
         * Deregisters the functions to create a DynamicSerializable for the given type
         * @tparam T The type to serialize, it must have a DynamicSerializable registered
         */
        template<class T>
        void DeregisterSerializableFor() {
            _serializationFunctions.erase(typeid(T));
        }

        /**
         * For a given type instance tries to return its Serializable
         * @return The serializable if registered, otherwise std::nullopt
         */
        std::optional<shared<DynamicSerializable>> TryGetSerializableFor(TypeInstance type) {
            if (_serializationFunctions.count(type) > 0)
                return _serializationFunctions.at(type);
            return std::nullopt;
        }


        /**
         * @return Returns if the given type has a serializable currently registered
         */
        bool HasSerializable(TypeInstance type) {
             return _serializationFunctions.count(type) > 0;
        }

        /**
         * @return Returns a list of all registered types and their serializables
         */
        std::vector<std::pair<TypeInstance, shared<DynamicSerializable>>> AllRegisteredSerializables(){
            auto res = std::vector<std::pair<TypeInstance, shared<DynamicSerializable>>>();

            for(auto& kvp : _serializationFunctions){
                res.emplace_back(kvp.first, kvp.second);
            }
            return res;
        }

    private:
        InstancedTypeMap<shared<DynamicSerializable>> _serializationFunctions{};
    };


}