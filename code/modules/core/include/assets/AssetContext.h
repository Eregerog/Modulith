/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <utility>

#include "CoreModule.h"

namespace modulith {

    struct Address {

        Address() = default;

        explicit Address(std::filesystem::path relativePath) : _underlying(std::move(relativePath)) {}

        [[nodiscard]] std::filesystem::path AsRelativePath() const { return _underlying; }

        [[nodiscard]] std::string AsString() const { return _underlying.string(); }

        [[nodiscard]] Address ParentAddress() const { return Address(_underlying.parent_path()); }

        Address operator / (const Address& rhs){
            return Address(_underlying / rhs._underlying);
        }

        Address operator / (const std::string& rhs){
            return Address(_underlying / rhs);
        }

    private:
        std::filesystem::path _underlying;
    };

    struct AssetInfo{
        explicit AssetInfo(Module loadedBy) : _loadedBy({std::move(loadedBy)}){}

        [[nodiscard]] Module LoadFrom() const { return _loadedBy.back(); }

        void PushBackModule(const Module& module) { _loadedBy.push_back(module); }
        void RemoveModule(const Module& module) {
            _loadedBy.erase(std::remove(_loadedBy.begin(), _loadedBy.end(), module), _loadedBy.end());
        }

        bool IsValid() { return !_loadedBy.empty(); }
    private:
        std::list<Module> _loadedBy;
    };

    struct AddressHasher {
        std::size_t operator()(const Address& address) const {
            return std::hash<std::string>{}(address.AsString());
        }
    };

    struct AddressEqualTo {
        bool operator()(const Address& lhs, const Address& rhs) const {
            return lhs.AsString() == rhs.AsString();
        }
    };

    class AssetContext : public Subcontext{
    public:
        AssetContext() : Subcontext("Asset Context") {}

        void OnInitialize() override;

        void OnBeforeLoadModule(const Module& module) override;

        void OnAfterUnloadModule(const Module& module) override;

        std::optional<std::filesystem::path> ResolveAddress(const Address& address);
        std::filesystem::path ResolveAddressOrThrow(const Address& address, const std::string& context);

    private:
        void loadAssetsForModule(const Module& module);

        std::unordered_multimap<Module, Address> _moduleAssets;
        std::unordered_map<Address, AssetInfo, AddressHasher, AddressEqualTo> _assets;
    };
}
