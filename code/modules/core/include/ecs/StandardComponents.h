/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include <utility>

#include "CoreModule.h"
#include "serialization/Serializer.h"

namespace modulith {

    /**
     * Entities with this component are disabled
     * and therefore won't be included in most queries
     */
    struct CORE_API DisabledTag {
    };

    /**
     * Entities with this component are disabled because another entity is disabled
     * (such as their parent entity). Just like the DisabledTag, entities with this tag
     * also also not included in most queries
     */
    struct CORE_API IndirectlyDisabledTag {
    };

    /**
     * This component stores data of the display name of an entity
     */
    struct CORE_API NameData {

        NameData() = default;
        explicit NameData(const char* value) : Value(value) {}
        explicit NameData(std::string value) : Value(std::move(value)) {}

        std::string Value;
    };

    /**
     * Allows serialization of NameData
     */
    template<>
    struct CORE_API Serializer<NameData>{
        [[nodiscard]] static NameData Construct() { return NameData(); }

        [[nodiscard]] static SerializedObject Serialize(const NameData& nameData) {
            return { "Root", {{ "Value", nameData.Value }} };
        }

        [[nodiscard]] static std::optional<NameData> TryDeserialize(const SerializedObject& so) {
            if(auto value = so.TryGet("Value")){
                if(auto name = value->TryGet<std::string>()){
                    return NameData(name.value().c_str());
                }
            }
            return std::nullopt;
        }
    };

    /**
     * When a registered component derives from this struct,
     * a suiting Initialized tag for that component is also registered and can be used.
     *
     * This pattern should be used when the data within a component must first be initialized by a system.
     * The component inherits from this struct and the system that does the initialization attaches the suiting
     * InitializedTag to the entity once it is finished.
     *
     * Systems that depend on the data to be initialized should also require the corresponding InitializedTag in their query.
     *
     * @see InitializedTag
     */
    struct CORE_API InitializationTrait {
    };


    /**
     * When this tag is attached to an entity is denotes that the data of the @param TComponent component is initialized
     * @tparam TComponent The type of the component that derived from InitializationTrait that this tag belongs to
     * @see InitializationTrait
     */
    template<class TComponent>
    struct InitializedTag {
    };

}