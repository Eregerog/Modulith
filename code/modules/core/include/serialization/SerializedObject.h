/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include <ecs/Entity.h>
#include "CoreModule.h"



namespace modulith{
    class CORE_API SerializedObject;

    /**
     * A possible value that can be stored inside a SerializedProperty.
     * It denotes that the property has no valid value.
     */
    struct CORE_API InvalidSerializedProperty {
        InvalidSerializedProperty() = default;

        bool operator==(const InvalidSerializedProperty& rhs) const {
            return true;
        }

        bool operator!=(const InvalidSerializedProperty& rhs) const {
            return false;
        }
    };

    /**
     * A possible value that can be stored inside a SerializedProperty
     * It denotes that the property is composed of various serialized sub objects.
     * All sub objects must have a unique name
     */
    class CORE_API SerializedSubObjects{

    public:
        /**
         * Creates an instance with no subobjects
         */
        SerializedSubObjects() = default;

        /**
         * Creates an instance with the given subobjects
         */
        SerializedSubObjects(const std::initializer_list<SerializedObject>& values);

        /**
         * Creates an instance with the given subobjects
         */
        explicit SerializedSubObjects(const std::vector<SerializedObject>& values);

        /**
         * Tries to get a subobject with the given name
         * @param identifier The name of the sub object to get
         * @return The subobject if successful, std::nullopt otherwise
         */
        std::optional<SerializedObject> TryGet(const std::string& identifier);

        /**
         * @return Returns the amount of contained subobjects
         */
        [[nodiscard]] size_t Count() const { return _values.size(); }

        /**
         * @return Returns a list with copies of the contained sub objects
         */
        [[nodiscard]] std::vector<SerializedObject> Items() const;

        /**
         * Checks if two instances are equal.
         * It is the case when they have the same amount of SerializedSubobjects (in correct order) that are also equal
         */
        bool operator==(const SerializedSubObjects& rhs) const;

        bool operator!=(const SerializedSubObjects& rhs) const {
            return !(rhs == *this);
        }
    private:
        std::unordered_map<std::string, SerializedObject> _values{};

    };

    /**
     * A SerializedProperty describes any of the values a SerializedObject can hold
     * and thus the values that can be serialized
     */
    using SerializedProperty = std::variant<
        InvalidSerializedProperty,
        int, int2, int3, int4,
        float, float2, float3, float4,
        float3x3, float4x4,
        bool, color3, color4, quat,
        Entity,
        std::string, SerializedSubObjects
    >;

    /**
     * A serialized object is a serializable representation of a struct or its fields.
     * It can recusivly be composed of other serialized objects and can thus serialize any tree-like data structures.
     */
    class CORE_API SerializedObject{
    public:
        /**
         * Constructs an invalid serialized object
         */
        SerializedObject() : _name("Invalid"), _property(InvalidSerializedProperty()) {}

        /**
         * Constructs a serialized object from a name and various subobjects it is composed of
         * @param name The name, which should represents the fields name or "Root" if it is the root of the serialized hierarchy
         * @param subObjects The subobjects it will be composed of
         */
        SerializedObject(std::string name, std::initializer_list<SerializedObject> subObjects) : _name(std::move(name)), _property(SerializedSubObjects(subObjects)) {}

        /**
         * Constructs a serialized object from a name and a value
         * @tparam T The type of the value. It must be one of the SerializedProperty supported values
         * @param name The name of the represented value
         * @param value The value of this serialized object
         */
        template<class T>
        SerializedObject(std::string name, T value) : _name(std::move(name)), _property(value) {}

    public:

        /**
         * Returns the name of the serialized object
         */
        [[nodiscard]] std::string GetName() const {
            return _name;
        }

        /**
         * @return Returns whether this SerializedObject is composed of other sub-objects
         */
        [[nodiscard]] bool IsComposed() const { return std::holds_alternative<SerializedSubObjects>(_property); }

        /**
         * Tries to get a sub object by name
         * @param subObjectName The name of the subobject
         * @return The subobject if one was found, std::nullopt otherwise or if this isn't composed.
         */
        [[nodiscard]] std::optional<SerializedObject> TryGet(const std::string& subObjectName) const;

        /**
         * Tries to get a copy of the underlying value, if it is of the given type
         * @tparam T The type of value to get, should be one of the SerializedProperty supported types
         * @return The value if successful, std::nullopt otherwise (e.g. the underlying value had a different type)
         */
        template<class T>
        [[nodiscard]] std::optional<T> TryGet() const {
            if(std::holds_alternative<T>(_property))
                return std::get<T>(_property);
            return std::nullopt;
        }

        /**
         * Tries to get a subobject by name and then a copy of its underlying value, if it is of the given type
         * @tparam T The type of value to get, should be one of the SerializedProperty supported types
         * @param subObjectName The name of the subObject
         * @return The value if successful, std::nullopt otherwise
         * (e.g. this object is not composed, a subobject of that name could not be found or its underlying value had a different type)
         */
        template<class T>
        [[nodiscard]] std::optional<T> TryGet(std::string subObjectName) const {
            if(auto sub = TryGet(subObjectName))
                return sub->TryGet<T>();
            return std::nullopt;
        }

        /**
         * Tries to get a pointer to the underlying value of the given type
         * @tparam T The type of pointer to get, should be one of the SerializedProperty supported types
         * @return A pointer to the underlying value if it is of the given type, nullptr otherwise
         * @remark A pointer is returned for mutation but ownership is not transferred
         */
        template<class T>
        [[nodiscard]] T* TryGetPtr() {
            return std::get_if<T>(&_property);
        }

        /**
         * @inherit TryGetPtr
         */
        template<class T>
        [[nodiscard]] const T* TryGetPtr() const {
            return std::get_if<T>(&_property);
        }

        /**
         * @return Returns whether the value of this instance is the same of the given property
         */
        [[nodiscard]] bool HasSameUnderlyingValueAs(const SerializedProperty& other) const { return _property == other; }

        /**
         * @return Returns a copy of the underlying value
         */
        [[nodiscard]] SerializedProperty GetUnderlyingValue() const { return _property; }

        /**
         * @return Returns the index of the propertie's type
         */
        [[nodiscard]] size_t GetPropertyIndex() const { return _property.index(); }

        /**
         * @return Returns if two SerializedObjects are equal.
         * This is the case if they have the same name, property type and value
         */
        bool operator==(const SerializedObject& rhs) const {
            return _name == rhs._name && _property == rhs._property;
        }

        bool operator!=(const SerializedObject& rhs) const {
            return !(rhs == *this);
        }

    private:

        std::string _name{};
        SerializedProperty _property{};
    };

}

namespace YAML {
    using namespace modulith;

    template<>
    struct convert<SerializedSubObjects> {
        static Node encode(const SerializedSubObjects& val) {
            Node node;
            for(auto& item : val.Items())
                node.push_back(item);
            return node;
        }

        static bool decode(const Node& node, SerializedSubObjects& res) {
            try{
                if(node.IsSequence()){
                    auto subObjects = std::vector<SerializedObject>();
                    for(auto& subnode : node){
                        subObjects.push_back(subnode.as<SerializedObject>());
                    }
                    res = SerializedSubObjects(subObjects);
                    return true;
                }
            }catch(YAML::InvalidNode&){
                return false;
            }

            return false;
        }
    };

    template<>
    struct convert<SerializedObject> {
        static Node encode(const SerializedObject& val) {
            Node node;

            node["name"] = val.GetName();
            node["type"] = val.GetPropertyIndex();

            if(auto* asInt = val.TryGetPtr<int>())        node["value"] = *asInt;
            else if(auto* asInt2 = val.TryGetPtr<int2>()) node["value"] = *asInt2;
            else if(auto* asInt3 = val.TryGetPtr<int3>()) node["value"] = *asInt3;
            else if(auto* asInt4 = val.TryGetPtr<int4>()) node["value"] = *asInt4;

            else if(auto* asFloat = val.TryGetPtr<float>())   node["value"] = *asFloat;
            else if(auto* asFloat2 = val.TryGetPtr<float2>()) node["value"] = *asFloat2;
            else if(auto* asFloat3 = val.TryGetPtr<float3>()) node["value"] = *asFloat3;
            else if(auto* asFloat4 = val.TryGetPtr<float4>()) node["value"] = *asFloat4;

            else if(auto* asFloat3x3 = val.TryGetPtr<float3x3>()) node["value"] = *asFloat3x3;
            else if(auto* asFloat4x4 = val.TryGetPtr<float4x4>()) node["value"] = *asFloat4x4;

            else if(auto* asBool = val.TryGetPtr<bool>()) node["value"] = *asBool;

            else if(auto* asColor3 = val.TryGetPtr<color3>()) node["value"] = *asColor3;
            else if(auto* asColor4 = val.TryGetPtr<color4>()) node["value"] = *asColor4;

            else if(auto* asQuat = val.TryGetPtr<quat>()) node["value"] = *asQuat;

            else if(auto* asEntity = val.TryGetPtr<Entity>()) node["value"] = *asEntity;

            else if(auto* asString = val.TryGetPtr<std::string>()) node["value"] = *asString;

            else if(auto* subObjects = val.TryGetPtr<SerializedSubObjects>()) node["value"] = *subObjects;

            return node;
        }

        static bool decode(const Node& node, SerializedObject& res) {

            try {
                auto name = node["name"].as<std::string>();
                auto typeIndex = node["type"].as<size_t>();

                switch (typeIndex) {
                    case 1: // int
                        res = SerializedObject(name, node["value"].as<int>());
                        break;
                    case 2: // int2
                        res = SerializedObject(name, node["value"].as<int2>());
                        break;
                    case 3: // int3
                        res = SerializedObject(name, node["value"].as<int3>());
                        break;
                    case 4: // int4
                        res = SerializedObject(name, node["value"].as<int4>());
                        break;

                    case 5: // float
                        res = SerializedObject(name, node["value"].as<float>());
                        break;
                    case 6: // float2
                        res = SerializedObject(name, node["value"].as<float2>());
                        break;
                    case 7: // float3
                        res = SerializedObject(name, node["value"].as<float3>());
                        break;
                    case 8: // float4
                        res = SerializedObject(name, node["value"].as<float4>());
                        break;

                    case 9: // float3x3
                        res = SerializedObject(name, node["value"].as<float3x3>());
                        break;
                    case 10: // float4x4
                        res = SerializedObject(name, node["value"].as<float4x4>());
                        break;

                    case 11: // bool
                        res = SerializedObject(name, node["value"].as<bool>());
                        break;

                    case 12: // color3
                        res = SerializedObject(name, node["value"].as<color3>());
                        break;
                    case 13: // color4
                        res = SerializedObject(name, node["value"].as<color4>());
                        break;

                    case 14: // quat
                        res = SerializedObject(name, node["value"].as<quat>());
                        break;
                    case 15: // Entity
                        res = SerializedObject(name, node["value"].as<Entity>());
                        break;

                    case 16: // string
                        res = SerializedObject(name, node["value"].as<std::string>());
                        break;

                    case 17: // SerializedSubObjects
                        res = SerializedObject(name, node["value"].as<SerializedSubObjects>());
                        break;

                    default:
                        return false;
                }

                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }

    };
}