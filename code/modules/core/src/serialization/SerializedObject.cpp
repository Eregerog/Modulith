/**
 * \brief
 * \author Daniel Götz
 */

#include "serialization/SerializedObject.h"

namespace modulith{

    bool SerializedSubObjects::operator==(const SerializedSubObjects& rhs) const {
        auto ownItems = Items();
        auto otherItems = rhs.Items();
        if(ownItems.size() == otherItems.size()){
            for(int idx = 0; idx < ownItems.size(); ++idx){
                if(ownItems[idx] != otherItems[idx])
                    return false;
            }
            return true;
        } else {
            return false;
        }
    }

    SerializedSubObjects::SerializedSubObjects(const std::initializer_list<SerializedObject>& values) : SerializedSubObjects(std::vector(values)) {}

    SerializedSubObjects::SerializedSubObjects(const std::vector<SerializedObject>& values) {
        for(auto& item : values){
            auto name = item.GetName();
            CoreAssert(_values.count(name) == 0, "Cannot construct SerializedSubObjects: A SerializedObject with name {} already exists in this sub-object", name)
            _values.emplace(item.GetName(), item);
        }
    }

    std::optional<SerializedObject> SerializedSubObjects::TryGet(const std::string& identifier) {
        if(_values.count(identifier) > 0)
            return _values.at(identifier);
        return std::nullopt;
    }

    std::vector<SerializedObject> SerializedSubObjects::Items() const {
        auto res = std::vector<SerializedObject>();
        for(auto& kvp : _values)
            res.push_back(kvp.second);
        return res;
    }


    std::optional<SerializedObject> SerializedObject::TryGet(const std::string& subObjectName) const {
        if(auto subObjects = TryGet<SerializedSubObjects>()){
            return subObjects->TryGet(subObjectName);
        }
        return std::nullopt;
    }
}