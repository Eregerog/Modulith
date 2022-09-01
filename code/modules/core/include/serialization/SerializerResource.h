/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include "resources/Resource.h"
#include "SerializationContext.h"

namespace modulith{
    /**
     * A resource that registers serialization capabilities for a single type while the registering code is loaded
     * @tparam T The type that can be serialized
     * @tparam TSerializer An object that contains the statis methods for serializing that type. See Serializer and TrivialSerializer
     */
    template<class T, class TSerializer = Serializer<T>, std::enable_if_t<
        std::is_same_v<decltype(TSerializer::Construct()), T>
        && std::is_same_v<decltype(TSerializer::Serialize(std::declval<const T&>())), SerializedObject>
        && std::is_same_v<decltype(TSerializer::TryDeserialize(std::declval<const SerializedObject&>())), std::optional<T>>,
        int> = 0
    >
    class SerializerResource : public Resource {
    public:

        void OnLoad(std::string description) override {
            Context::GetInstance<SerializationContext>()->RegisterSerializableFor<T>(
                /* construct   */ [](                         ) { return TSerializer::Construct();                   },
                /* serialize   */ [](const auto& toSerialize  ) { return TSerializer::Serialize(toSerialize);        },
                /* deserialize */ [](const auto& toDeserialize) { return TSerializer::TryDeserialize(toDeserialize); }
            );
        }

        void OnInitializeAfterLoad(std::string description) override {
            // unused
        }

        void OnShutdownBeforeUnload(std::string description) override {
            // unused
        }

        void OnUnload(std::string description) override {
            Context::GetInstance<SerializationContext>()->DeregisterSerializableFor<T>();
        }

        int GetPriority() override {
            return 2;
        }
    };
}