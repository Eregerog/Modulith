set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/docs")
set(DOXYGEN_PROJECT_LOGO "${PROJECT_SOURCE_DIR}/docs/logo - light.png")

doxygen_add_docs(ModulithEngineDocs
        "${PROJECT_SOURCE_DIR}/core/engine/include"
        "${PROJECT_SOURCE_DIR}/docs"
        "${PROJECT_SOURCE_DIR}/modules/renderer/include"
        "${PROJECT_SOURCE_DIR}/modules/core/include"
        "${PROJECT_SOURCE_DIR}/modules/physics/include"
        "${PROJECT_SOURCE_DIR}/modules/editor/include"
        "${PROJECT_SOURCE_DIR}/modules/game/include"
        "${PROJECT_SOURCE_DIR}/modules/shooter/include"
        "${PROJECT_SOURCE_DIR}/modules/mario/include"
        WORKING_DIRECTORY
        "${PROJECT_SOURCE_DIR}/core/engine/include"
        )
