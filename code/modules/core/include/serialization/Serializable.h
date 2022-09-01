/**
 * \brief
 * \author Daniel Götz
 */

#pragma once
#include "CoreModule.h"
#include "SerializedObject.h"

namespace modulith{

    /**
     * Classes inheriting from dynamic serializable should provide functions for a specific type
     * to trivially construct, serialize and deserialize
     */
    class CORE_API DynamicSerializable {
    public:
        /**
         * This method should create an instance of the supported type.
         * It is given no parameters and should therefore use reasonable defaults.
         * @return A std::any object in which an instance of the supported type is contained
         */
        [[nodiscard]] virtual std::any Construct() const = 0;

        /**
         * This method should take an instance of the supported and create a (hierarchy of) serialized object(s)
         * from it and its fields to be serialized.
         * @param toSerialize The value to serialize
         * @return A (hierarchy of) SerializedObject(s) that contains all the fields to serialize
         */
        [[nodiscard]] virtual SerializedObject Serialize(const std::any& toSerialize) const = 0;

        /**
         * This method should take a serialized object (hierarchy) and try to create an instance of the supported type from it
         * @param toDeserialize The serialized object to deserialize
         * @return Either an instance of the supported type or std::nullopt if the operation was not successful
         */
        [[nodiscard]] virtual std::optional<std::any> TryDeserialize(const SerializedObject& toDeserialize) const = 0;
    };

    /**
     * A class that can serialize, deserialize and trivially construct for a given type
     * @tparam T The type of this Serializable
     */
    template<class T>
    class Serializable : public DynamicSerializable {
    public:
        /**
         * Creates a serializable for the given type that contains functions for trivially constructing, serializing and deserializing the type
         * @param construct The strongly-typed version of { @link DynamicSerializable.Construct() }
         * @param serialize The strongly-typed version of { @link DynamicSerializable.Serialize() }
         * @param tryDeserialize The strongly-typed version of { @link DynamicSerializable.TryDeserialize() }
         */
        Serializable(
            std::function<T()> construct,
            std::function<SerializedObject(const T&)> serialize,
            std::function<std::optional<T>(const SerializedObject&)> tryDeserialize
        ) : _construct(std::move(construct)), _serialize(std::move(serialize)), _tryDeserialize(std::move(tryDeserialize)) {}

        [[nodiscard]] std::any Construct() const override {
            return std::make_any<T>(_construct());
        }

        [[nodiscard]] SerializedObject Serialize(const std::any& toSerialize) const override {
            if(auto* typedVal = std::any_cast<T>(&toSerialize)){
                return _serialize(*typedVal);
            }
            CoreAssert(false, "Type mismatch: Cannot serialize any of type {} with Serializable of type", toSerialize.type().name(), typeid(T).name())
            return SerializedObject();
        }

        [[nodiscard]] std::optional<std::any> TryDeserialize(const SerializedObject& toDeserialize) const override {
            if(std::optional<T> res = _tryDeserialize(toDeserialize)){
                return std::make_any<T>(res.value());
            }
            return std::nullopt;
        }

    private:
        std::function<T()> _construct;
        std::function<SerializedObject(const T&)> _serialize;
        std::function<std::optional<T>(const SerializedObject&)> _tryDeserialize;
    };
}