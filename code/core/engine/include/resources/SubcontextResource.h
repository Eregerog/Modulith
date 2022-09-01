/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include "Resource.h"
#include "Context.h"
#include "Subcontext.h"

namespace modulith{

    /**
     * A resource that registers a single subcontext group to the Context while the registering code is loaded
     * @tparam TSubcontext
     */
    template<class TSubcontext, class = std::enable_if_t<std::is_base_of_v<Subcontext, TSubcontext>>>
    class SubcontextResource : public Resource {
    public:
        /**
         * Creates a new subcontext resource
         * @param overridePriority An alternative priority that should be used. (5 if no value is specified)
         * A higher priority will load this resource before others and unload it after others.
         */
        explicit SubcontextResource(const std::optional<int>& overridePriority = std::nullopt) : _overridePriority(overridePriority), _subcontext(std::make_shared<TSubcontext>()) {}

        void OnLoad(std::string description) override {
            Context::Instance().RegisterSubcontext<TSubcontext>(_subcontext);
        }

        void OnInitializeAfterLoad(std::string description) override {
            _subcontext->OnInitialize();
        }

        void OnShutdownBeforeUnload(std::string description) override {
            _subcontext->OnShutdown();
        }

        void OnUnload(std::string description) override {
            Context::Instance().DeregisterSubcontext<TSubcontext>();
            _subcontext.reset();
        }

        int GetPriority() override {
            return _overridePriority.value_or(5);
        }

    private:
        shared<TSubcontext> _subcontext;
        std::optional<int> _overridePriority;
    };

}