/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"

#include <utility>

#include "ECSUtils.h"
#include "StandardComponents.h"

namespace modulith{

    class ComponentManager;
    class RegisteredComponent;

    /**
     * Represents info of a component type that is created when the type is registered with the module
     */
    struct CORE_API ComponentInfo {
        friend RegisteredComponent;
        friend ComponentManager;

    public:
        /**
         * Creates an info object from a component with the initialization trait
         * @see InitializationTrait
         * @tparam TComponent The type of the component that inherits from InitializationTrait
         * @param moduleName The name of the module the component belongs to
         * @param componentName The name of the component
         * @return Two component infos: One for the component itself, another for its initialized  version
         */
        template<class TComponent, std::enable_if_t<std::is_base_of_v<InitializationTrait, TComponent>, int> = 0>
        static std::vector<shared<ComponentInfo>> Create(std::string moduleName, std::string componentName) {
            return {createSingle<TComponent>(moduleName, componentName),
                    createSingle<InitializedTag<TComponent>>(moduleName, componentName + "_Initialized")};
        }

        /**
         * Creates an info object from a component without the initialization trait
         * @tparam TComponent The type of the component
         * @param moduleName The name of the module the component belongs to
         * @param componentName The name of the component
         * @return One component info for the component itself
         */
        template<class TComponent, std::enable_if_t<!std::is_base_of_v<InitializationTrait, TComponent>, int> = 0>
        static std::vector<shared<ComponentInfo>> Create(std::string moduleName, std::string componentName) {
            return {createSingle<TComponent>(moduleName, componentName)};
        }

    private:

        template<class TComponent>
        static shared<ComponentInfo> createSingle(std::string moduleName, std::string componentName) {
            auto res = std::make_shared<ComponentInfo>(
                typeid(TComponent),
                [](void* component) {
                    ((TComponent*) component)->~TComponent();
                },
                getCopyFunctionFor<TComponent>(),
                getCopyIntoAnyFunctionFor<TComponent>(),
                getCopyIntoPointerFunctionFor<TComponent>(),
                sizeof(TComponent)
            );
            res->_moduleName = moduleName;
            res->_componentName = componentName;
            return res;
        }

        template<class TComponent, std::enable_if_t<std::is_copy_constructible_v<TComponent>, int> = 0>
        static std::function<void(void*, void*)> getCopyFunctionFor(){
            return [](void* destPtr, void* srcPtr) {
                // Call the copy constructor intentionally, so resource counter (like shared pointers) are incremented
                auto copy = new TComponent(*(TComponent*) srcPtr);
                memmove(destPtr, copy, sizeof(TComponent));
                // Intentionally not calling delete because we don't want destructors to be called and resource counters to be reduced
                free((TComponent*) copy);
            };
        }

        template<class TComponent, std::enable_if_t<!std::is_copy_constructible_v<TComponent>, int> = 0>
        static std::function<void(void*, void*)> getCopyFunctionFor(){
            return nullptr;
        }

        template<class TComponent, std::enable_if_t<std::is_copy_constructible_v<TComponent>, int> = 0>
        static std::function<std::any(void*)> getCopyIntoAnyFunctionFor(){
            return [](void* component){
                return std::make_any<TComponent>(*(TComponent*) component);
            };
        }

        template<class TComponent, std::enable_if_t<!std::is_copy_constructible_v<TComponent>, int> = 0>
        static std::function<std::any(void*)>getCopyIntoAnyFunctionFor(){
            return nullptr;
        }

        template<class TComponent, std::enable_if_t<std::is_copy_constructible_v<TComponent>, int> = 0>
        static  std::function<void(std::any, void*)> getCopyIntoPointerFunctionFor(){
            return [](std::any source, void* destination){
                ((TComponent*) destination)->~TComponent();
                auto toCopy = std::any_cast<TComponent>(source);
                memmove(destination, &toCopy, sizeof(TComponent));
                // Ensure that the destructors are not called on the component that was just moved into the destination
                memset(&toCopy, 0, sizeof(TComponent));
            };
        }

        template<class TComponent, std::enable_if_t<!std::is_copy_constructible_v<TComponent>, int> = 0>
        static std::function<void(std::any, void*)> getCopyIntoPointerFunctionFor(){
            return nullptr;
        }


    public:

        /**
         * @warning Use ComponentInfo.Create instead
         */
        ComponentInfo() = default;

        /**
         * @warning Use ComponentInfo.Create instead
         */
        ComponentInfo(
            const ComponentIdentifier& identifier,
            std::function<void(void*)> destruct,
            std::function<void(void*, void*)> createCopyIn,
            std::function<std::any(void*)> copyPointerIntoAny,
            std::function<void(std::any, void*)> copyAnyIntoPointer,
            size_t size
        )
            : _identifier(identifier), _destruct(std::move(destruct)), _createCopyIn(std::move(createCopyIn)),
            _copyPointerIntoAny(std::move(copyPointerIntoAny)), _copyAnyIntoPointer(std::move(copyAnyIntoPointer)),
              _size(size) {}

    private:
        std::optional<ComponentIdentifier> _identifier{};
        std::string _moduleName{};
        std::string _componentName{};
        std::function<void(void*)> _destruct = nullptr;
        std::function<void(void*, void*)> _createCopyIn = nullptr;
        std::function<std::any(void*)> _copyPointerIntoAny = nullptr;
        std::function<void(std::any, void*)> _copyAnyIntoPointer = nullptr;
        size_t _size = -1;
    };


    /**
     * Represents info of a component type that is created when it is registered in the component manager
     */
    class CORE_API RegisteredComponent {
    public:
        /**
         * @warning Do not use this constructor, it is public solely to offer
         * trivial construction for unordered maps
         */
        RegisteredComponent() = default;

        RegisteredComponent(
            shared<ComponentInfo> info, uint32_t index
        ) : _info(info),
            _index(index) {
            CoreAssert(_info->_identifier.has_value(), "Cannot create a registered component from an invalid component info!")
        }

        /**
         * @return Returns the component identifier / component type info of this registered component
         */
        [[nodiscard]] ComponentIdentifier GetIdentifier() const { return _info->_identifier.value(); }

        /**
         * @return Returns the full name of the component, which includes the module name and its name
         */
        [[nodiscard]] std::string GetFullName() const { return _info->_moduleName + "::" + _info->_componentName; }

        /**
         * @return Returns the module this component was registered by
         */
        [[nodiscard]] std::string GetModuleName() const { return _info->_moduleName; }

        /**
         * @return Returns the name of the component without the name of the moduel
         */
        [[nodiscard]] std::string GetComponentName() const { return _info->_componentName; }

        /**
         * @return Returns a unique index of the component
         */
        [[nodiscard]] uint32_t GetIndex() const { return _index; }

        /**
         * @return Returns the size (in bytes) of the component
         */
        [[nodiscard]] size_t GetSize() const { return _info->_size; }

        /**
         * Manually calls the component's destructor on the given pointer
         * @param component A pointer that points to memory with a component of this registered component's type
         */
        void Destruct(void* component) { _info->_destruct(component); }

        /**
         * @return Returns whether the component type supports trivial copy construction
         */
        [[nodiscard]] bool IsCopyable() const { return _info->_createCopyIn != nullptr; }

        /**
         * Creates a copy of the data from srcPtr at destPtr
         * @param destPtr A pointer to memory with the component of this registered component's type
         * @param srcPtr A pointer to memory with the component of this registered component's type
         */
        void CreateCopyIn(void* destPtr, void* srcPtr) {
            CoreAssert(IsCopyable(),
                "Attempted to call CreateCopyIn for a component that is not copyable. This is due to a deleted copy constructor!")
            _info->_createCopyIn(destPtr, srcPtr);
        }

        /**
         * @return Returns whether the component can be serialized, which mandates that it can be copied into and from a std::any object
         */
        [[nodiscard]] bool IsSerializable() const { return _info->_copyAnyIntoPointer && _info->_copyPointerIntoAny; }

        /**
         * Creates a copy of the component fround at source and returns it wrapped in a std::any object
         * @param source A pointer to memory with the component of this registered component's type
         * @return A copy of the component found at source
         */
        std::any CopyFromPointerToAny(void* source) {
            CoreAssert(IsSerializable(), "Attempted to call CopyFromPointerToAny for a component that is not serializable. This is because it isn't trivially constructable.")
            return _info->_copyPointerIntoAny(source);
        }

        /**
         * Creates a copy of the component inside any at the destination pointer
         * @param source An any object that must contain an instance of the registered component's type
         * @param destination A pointer to memory with the component of this registered component's type
         */
        void CopyFromAnyToPointer(std::any source, void* destination) {
            CoreAssert(IsSerializable(), "Attempted to call CopyFromPointerToAny for a component that is not serializable. This is because it isn't trivially constructable.")
                _info->_copyAnyIntoPointer(std::move(source), destination);
        }

    private:
        shared<ComponentInfo> _info{};
        uint32_t _index = -1;
    };


    /**
     * Contains information about all the registered components.
     * Each registered component is assigned a unique id (its index).
     * While registered, a component of the given type may be attached to entities.
     */
    class CORE_API ComponentManager {

    public:

        ComponentManager() = default;

        /**
         * Registers all provided component infos.
         * Instances of their underlying component types may then be attached to entities
         * @param componentInfos The component infos to register
         */
        void RegisterComponents(const std::vector<shared<ComponentInfo>>& componentInfos);

        /**
         * Registers the provided component info.
         * Instances of its underlying component type may then be attached to entities
         * @param componentInfo The component info to register
         */
        void RegisterComponent(const shared<ComponentInfo>& componentInfo);

        /**
         * Deregisters the provided component info
         * Before doing so, components of that type may no longer be present on any entity
         * @param componentInfo The component info to deregister
         */
        void DeregisterComponent(const shared<ComponentInfo>& componentInfo);

        /**
         * @return Returns whether the component of the given type is registered
         */
        template<class TComponent>
        bool IsRegistered() {
            return IsRegistered(typeid(TComponent));
        }

        /**
         * @return Returns whether the component of the given type is registered
         */
        bool IsRegistered(ComponentIdentifier identifier){
            return _registeredComponents.count(identifier) > 0;
        }

        /**
         * Tries to find the registered component by its full name
         * @param fullName The name of the component. Should be of format <ModuleName>::<ComponentName>.
         * @return Returns the registered component if successful, std::nullopt otherwise
         */
        std::optional<RegisteredComponent> TryFindByFullName(const std::string& fullName){
            for(auto& kvp : _registeredComponents){
                if(kvp.second.GetFullName() == fullName){
                    return kvp.second;
                }
            }
            return std::nullopt;
        }

        /**
         * Creates a signature identifier from the given component types
         * @tparam TComponents Types of registered components
         * @return Their signature identifier
         */
        template<class... TComponents>
        SignatureIdentifier ToIdentifier();

        /**
         * Creates a signature of the given signature identifiers
         * @param identifier A signature identifier of registered components
         * @return The identifiers signature
         */
        Signature ToSignature(const SignatureIdentifier& identifier);

        /**
         * @return Returns the registered component info of the provided component
         */
        RegisteredComponent GetInfoOf(ComponentIdentifier identifier);

        /**
         * @return Returns how many components are currently registered
         */
        [[nodiscard]] size_t RegisteredComponentCount() const;

    private:

        uint32_t _componentCount = 0;
        ComponentMap<RegisteredComponent> _registeredComponents{};
    };

    template<class... TComponents>
    SignatureIdentifier ComponentManager::ToIdentifier() {
        auto result = SignatureIdentifier();
        result.insert({typeid(TComponents)...});
        return result;
    }
}
