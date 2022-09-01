#include <Engine.h>
#include <modules/ModuleUtils.h>
#include <resources/SubcontextResource.h>
#include <utils/PreferencesContext.h>

#include <utility>

namespace modulith{

    Context* _context = nullptr;

    Context& Context::Instance() {
        return *_context;
    }

    Engine::Engine(std::string  applicationName) : _applicationName(std::move(applicationName)) {
        _context = new Context(std::make_unique<modulith::Profiler>());

        Register<SubcontextResource<ModuleContext>>(6 /* Override Priority: Always shutdown / destruct the module ctx after all other ctx */ );
        Register<SubcontextResource<PreferencesContext>>();
    }

    Engine::~Engine(){
        delete _context;
    }

    void Engine::Run() {

        Log::Init();
        Log::CreateDefaultLoggers();

        _isInitialized = true;

        // Initialization

        _context->onInitialize();
        OnInitialize();

        Resource::InvokeLoadCallbacksOn(_engineResources, "Engine");

        // We load the default modules after initialization, to ensure that everything is initialized even IF we could load them earlier...
        auto moduleNames = modulith::ModuleUtils::ParseModlist();

        auto moduleCtx = _context->Get<ModuleContext>();
        for (auto& moduleName : moduleNames) {
            auto module = _context->Get<ModuleContext>()->ModuleFromName(moduleName);
            CoreAssert(module, "No module with the name {} could be found, could not load the modlist!", moduleName)

            if(moduleCtx->ModuleIsStillLoadedNextFrame(module.value())){
                CoreLogWarn("The module {} was found more than once in the modlist, ignoring...", moduleName)
                continue;
            }

            _context->Get<ModuleContext>()->LoadModuleAtBeginOfFrame(module.value());
        }

        _previousFrameTime = std::chrono::high_resolution_clock::now();

        // Game Loop

        while (_isRunning) {
            auto currentFrameTime = std::chrono::high_resolution_clock::now();
            auto deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(
                currentFrameTime - _previousFrameTime
            ).count();

            _previousFrameTime = currentFrameTime;

            _context->onPreUpdate();

            OnUpdate(deltaTime);
            _context->onUpdate(deltaTime);

            _context->onImGui(deltaTime);

            _context->onPostUpdate();

            _isRunning = _context->_isRunning;
        }

        // Shutdown
        Resource::InvokeUnloadCallbacksOn(_engineResources, "Engine");
        OnShutdown();
        _context->onShutdown();

    }
}
