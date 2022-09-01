/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include "SerializedObject.h"

namespace modulith{
    /**
     * Specializations of this template class are supposed to contain the following methods
     * to be able to fully describe a serializable type (and to be able to create a Serializable for it):
     * @code static T Construct() // a function that creates an instance of the type using reasonable default values
     * @code static SerializedObject Serialize(const T&) // a function that creates a SerializedObject from an instance of the type and its fields
     * @code static std::optional<T> TryDeserialize(const SerializedObject&) // a function that tries to create an instance from the types from a SerializedObject
     * @tparam T The type the Serializer describes
     */
    template<class T>
    struct Serializer;

    /**
     * A Serializer for types that are trivially constructable and have no fields that need to be serialized
     * Candidates for such types include tag components (because they have no fields).
     */
    template<class TComponent>
    struct TrivialSerializer {
        [[nodiscard]] static TComponent Construct() { return TComponent(); }

        [[nodiscard]] static SerializedObject Serialize(const TComponent& component) {
            return { "Root", SerializedSubObjects() };
        }

        [[nodiscard]] static std::optional<TComponent> TryDeserialize(const SerializedObject& so) {
            if(auto value = so.TryGet<SerializedSubObjects>()){
                if(value->Count() == 0){
                    return TComponent();
                }
            }
            return std::nullopt;
        }
    };
}