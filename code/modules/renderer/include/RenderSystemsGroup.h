/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "RendererModule.h"

namespace modulith::renderer {

    /**
     * The render systems group is executed after the LogicSystemsGroup,
     * making rendering the last step in each frame
     */
    class RENDERER_API RenderSystemsGroup : public modulith::SystemsGroup {
    public:
        std::string GetName() override { return "RenderSystemsGroup"; }
    };
}
