/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include "Entity.h"

namespace modulith{

    /**
     * The "each" restriction used in the EntityManager queries
     * @see EntityManager.QueryAll
     * @see EntityManager.QueryActive
     * @tparam ... All of the component types that must be present on an entity to be included in the query
     */
    template<class...>
    struct Each{
    };


    /**
     * The "any" restriction used in the EntityManager queries
     * @see EntityManager.QueryAll
     * @see EntityManager.QueryActive
     * @tparam ... One of these component types must be present on an entity to be included in the query
     */
    template<class...>
    struct Any{
    };

    /**
     * The "none" restriction used in the EntityManager queries
     * @see EntityManager.QueryAll
     * @see EntityManager.QueryActive
     * @tparam ... None of these component types must be present on an entity to be included in the query
     */
    template<class...>
    struct None{
    };


    /**
     * The "has" restriction used in the EntityManager queries
     * @see EntityManager.QueryAll
     * @see EntityManager.QueryActive
     * @tparam ... The query function will be called with an additional boolean parameter for each
     * of these component types. This boolean signals if the given component is present on the current entity.
     */
    template<class...>
    struct Has{
    };

    /**
     * Utility. When used with std::declval it converts any type into a boolean.
     * There is no implementation. This should only be used inside std::declval!
     */
    template<class>
    bool HasComponentUtility();

    struct ComponentTypeHasher;
    struct ComponentTypeEqualTo;

    /**
     * How many component types may be registered in total
     */
    const int MaximumComponentTypes = 512;

    /**
     * The in-engine alias for a reference to a std::type_info.
     * Used to identify a component type
     */
    using ComponentIdentifier = std::reference_wrapper<const std::type_info>;

    /**
     * Maps a component identifier to values of the given type
     */
    template<class TValue>
    using ComponentMap = std::unordered_map<ComponentIdentifier, TValue, ComponentTypeHasher, ComponentTypeEqualTo>;
    /**
     * A set of component identifiers
     */
    using ComponentSet = std::unordered_set<ComponentIdentifier, ComponentTypeHasher, ComponentTypeEqualTo>;

    /**
     * The signature identifier is a set of distinct component types.
     * It is for example used to describe which components are attached to an entity
     */
    using SignatureIdentifier = ComponentSet;

    /**
     * Similar the the signature identifier. It is used when fast bitwise operations are necessary
     * @see SignatureIdentifier
     */
    using Signature = std::bitset<MaximumComponentTypes>;


    /**
     * This struct allows the ComponentIdentifier to be hashable
     */
    struct ComponentTypeHasher {
        std::size_t operator()(ComponentIdentifier code) const {
            return code.get().hash_code();
        }
    };

    /**
     * This struct allows the ComponentIdentifier to be compared for equality
     */
    struct ComponentTypeEqualTo {
        bool operator()(ComponentIdentifier lhs, ComponentIdentifier rhs) const {
            return lhs.get() == rhs.get();
        }
    };
}
