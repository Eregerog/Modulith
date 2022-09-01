/**
 * \brief
 * \author Daniel Götz
 */

#include "assets/AssetContext.h"

using namespace modulith;

void modulith::AssetContext::loadAssetsForModule(const Module& module){
    auto folder = ModulePathUtils::GetModuleFolder(module.Name());
    auto assetsFolder = folder / "assets";
    if(std::filesystem::exists(assetsFolder)){

        auto absoluteAssetsFolder = std::filesystem::absolute(assetsFolder);

        auto assetsPrefix = boolinq::from(absoluteAssetsFolder.begin(), absoluteAssetsFolder.end()).count();

        auto getAddress = [&assetsPrefix](auto& absoluteAssetPath) -> Address {
            return boolinq::from(absoluteAssetPath.begin(), absoluteAssetPath.end())
                .skip(assetsPrefix)
                .template aggregate<Address>(
                    Address(),
                    [](auto accumulate, auto pathElement) { return accumulate / pathElement.string(); }
                );
        };

        for(auto& element : std::filesystem::recursive_directory_iterator(absoluteAssetsFolder)){
            if(element.is_regular_file()){
                auto address = getAddress(element.path());
                LogInfo("Found asset for {} - Address: {}", module.Name(), address.AsString())
                _moduleAssets.emplace(module, address);

                if(_assets.count(address) > 0){
                    _assets.at(address).PushBackModule(module);
                }else{
                    _assets.emplace(address, AssetInfo(module));
                }
            }
        }
    }
}


void AssetContext::OnInitialize() {
    auto moduleCtx = Context::GetInstance<ModuleContext>();
    for(const auto& loadedModule : moduleCtx->GetLoadedModules()){
        loadAssetsForModule(loadedModule);
    }

    loadAssetsForModule(moduleCtx->ModuleFromName("Core").value());

    LogInfo("=========================================================")
    LogInfo("NEW ASSETS AFTER INIT")
    LogInfo("=========================================================")

    for(auto& kvp : _assets){
        LogInfo("{}", ResolveAddress(kvp.first)->string())
    }
}


void modulith::AssetContext::OnBeforeLoadModule(const Module& module) {
    loadAssetsForModule(module);

    LogInfo("=========================================================")
    LogInfo("NEW ASSETS BEFORE MODULE LOAD")
    LogInfo("=========================================================")

    for(auto& kvp : _assets){
        LogInfo("{}", ResolveAddress(kvp.first)->string())
    }
}

void modulith::AssetContext::OnAfterUnloadModule(const Module& module) {
    if(_moduleAssets.count(module) == 0)
        return;

    const auto& range = _moduleAssets.equal_range(module);

    for(auto it = range.first; it != range.second; ++it){
        auto& entry = *it;
        auto& asset = _assets.at(entry.second);
        asset.RemoveModule(entry.first);
        if(!asset.IsValid())
            _assets.erase(entry.second);
    }

    _moduleAssets.erase(range.first, range.second);

    Assert(_moduleAssets.count(module) == 0, "There mustn't be any more entries for the module {} that will be removed", module)
}

std::optional<std::filesystem::path> modulith::AssetContext::ResolveAddress(const Address& address) {
    if(_assets.count(address) == 0)
        return std::nullopt;

    auto assetInfo = _assets.at(address);
    Assert(assetInfo.IsValid(), "The asset info must be valid, otherwise this is an error in the insertion / removal logic")

    auto modulePath = ModulePathUtils::GetModuleFolder(assetInfo.LoadFrom().Name());
    auto res =  modulePath / "assets" /  address.AsRelativePath();

    if(std::filesystem::exists(res)){
        return res;
    }else{
        LogWarn("The asset at {} was registered for module {}, but cannot be found. Was this asset renamed / moved since the module was loaded?", res, assetInfo.LoadFrom().Name());
        return std::nullopt;
    }
}

std::filesystem::path AssetContext::ResolveAddressOrThrow(const Address& address, const std::string& context) {
    auto res = ResolveAddress(address);
    if(res.has_value())
        return res.value();
    else{
        Assert(false, "Tried to resolve the address \"{}\", but no asset was found. Context: {}", address.AsString(), context)
        throw std::runtime_error("The address could not be resolved");
    }
}
