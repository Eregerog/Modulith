/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include <crossguid/guid.hpp>

namespace modulith {

    /**
     * This class is a handle for a single module, that is memory-efficient, cheaply comparable and hashable.
     * It is used by many module-related utilities and classes as an identifier
     * @remark The name field is only contained to make it easier to log the module.
     * It should not be used in most cases, since it could become outdated.
     * Instead, use the {@link ModuleContext}'s methods for getting the info from a Module handle
     */
    class Module {

    public:
        Module(std::string name, const xg::Guid& guid) : _name(std::move(name)), _guid(guid) {}

        Module() = default;

        /**
         * @return The name of the module. This may become outdated and thus should only be used for logging.
         * Otherwise, the handle should be used on the {@link ModuleContext} to get the module's info
         */
        [[nodiscard]] const std::string& Name() const {
            return _name;
        }

        /**
         * @return The guid of the module, that is used for equality comparison and hashing
         */
        [[nodiscard]] const xg::Guid& Guid() const {
            return _guid;
        }

        bool operator==(const Module& rhs) const {
            return _guid == rhs._guid;
        }

        bool operator!=(const Module& rhs) const {
            return !(rhs == *this);
        }

    private:
        std::string _name{};
        xg::Guid _guid{};
    };
}

/**
 * This function allows {@link Module} to be logged to the console
 */
inline std::ostream& operator<<(std::ostream& os, const modulith::Module& module)
{
    return os << module.Name() << " (" << module.Guid().str() << ")";
}

namespace std {

    /**
     * Allows {@link Module} to be hashed with the STL containers
     */
    template<>
    struct hash<modulith::Module>{
        size_t operator()(const modulith::Module& module) const
        {
            return hash<xg::Guid>()(module.Guid());
        }
    };

    /**
     * Allows {@link Module} to be equality compared with the STL containers
     */
    template<>
    struct equal_to<modulith::Module>{
        bool operator()(const modulith::Module& lhs, const modulith::Module& rhs) const{
            return lhs == rhs;
        }
    };
};
