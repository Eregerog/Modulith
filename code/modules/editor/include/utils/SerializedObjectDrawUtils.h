/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <EditorModule.h>

namespace modulith::editor {

    class SerializedObjectDrawUtils {

    public:
        static std::optional<SerializedObject> DrawSerializedObjectAndReturnIfChanged(SerializedObject& obj);

    private:

        static std::optional<SerializedObject> drawRecursiveAndReturnChanged(SerializedObject& obj, bool autoExpandIfComposed = false);

    };

}