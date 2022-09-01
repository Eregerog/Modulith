/**
 * \brief
 * \author Daniel Götz
 */


#pragma once

#include "CoreModule.h"
#include "ecs/systems/SystemsGroup.h"

namespace modulith {

    /**
     * One of the default systems groups.
     * It is executed before the TransformSystemsGroup and LogicSystemsGroup.
     * Systems that read the InputState and mutate game or transform state
     * should be contained in this group.
     * Systems that read transform state should not.
     */
    class CORE_API InputSystemsGroup : public SystemsGroup {
    public:
        std::string GetName() override {
            return "Input Systems Group";
        }
    };

    /**
     * One of the default systems groups.
     * It is executed after the TransformSystemsGroup and before the LogicSystemsGroup.
     * The LocalTransformSystem, ParentSystem and GlobalTransformSystem are contained in this group,
     * which automatically update the LocalTransformData, WithChildrenData and GlobalTransformData
     * for a given frame.
     * Systems that also mutate the transform data or scene graph should be contained in this group.
    */
    class CORE_API TransformSystemsGroup : public SystemsGroup {
    public:
        std::string GetName() override {
            return "Transform Systems Group";
        }
    };

    /**
     * One of the default systems groups, that is the default for any registered System if not specified otherwise.
     * It is executed after the InputSystemsGroup and TransformSystemsGroup.
     * Most gameplay-related systems should be contained in this group.
     */
    class CORE_API LogicSystemsGroup : public SystemsGroup {
    public:
        std::string GetName() override {
            return "Logic Systems Group";
        }
    };
}