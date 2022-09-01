/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"

namespace modulith{

    /**
     * An alias for a copyable / moveable unique identifier of a type.
     * TypeInstances referring to the same type have are equals and produce the same hash code,
     * even across different module's processes.
     *
     * However, each process has their own instance of every type in static memory and storing
     * such an instance when the module has been unloaded will lead to access violations.
     *
     * Therefore it is recommended to use the TypeHash wherever possible
     */
    using TypeInstance = std::reference_wrapper<const std::type_info>;

    /**
     * A unique identifier for a type that is the same for equal types, even across different module's processes.
     * It is trivially comparable and hashable and an ideal candidate for any collection.
     *
     * Unlike the TypeInstance, it is not a reference to static memory of a process and can safely be used
     * when the module that produced it is no longer loaded (as long as the type still exists).
     */
    using TypeHash = std::size_t;

    /**
     * Allows hashing of TypeInstance
     */
    struct TypeHasher {
        std::size_t operator()(TypeInstance code) const {
            return code.get().hash_code();
        }
    };

    /**
     * Allows equality comparison of TypeInstance
     */
    struct TypeEqualTo {
        bool operator()(TypeInstance lhs, TypeInstance rhs) const {
            return lhs.get() == rhs.get();
        }
    };

    /**
     * Alias for an unordered map of a TypeInstance. Due to the reasons given in TypeInstance, use a PersistentTypeMap wherever possible!
     */
    template<class TValue>
    using InstancedTypeMap = std::unordered_map<TypeInstance, TValue, TypeHasher, TypeEqualTo>;

    /**
     * Alias for an unordered set of a TypeInstance. Due to the reasons given in TypeInstance, use a PersistentTypeSet wherever possible!
     */
    using InstancedTypeSet = std::unordered_set<TypeInstance, TypeHasher, TypeEqualTo>;

    /**
     * Alias for an unordered map of TypeHash
     */
    template<class TValue>
    using PersistentTypeMap = std::unordered_map<TypeHash, TValue>;

    /**
     * Alias for an unordered set of TypeHash
     */
    using PersistentTypeSet = std::unordered_set<TypeHash>;
}
