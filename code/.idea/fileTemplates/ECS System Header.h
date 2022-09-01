#parse("C File Header.h")

#[[#pragma once]]#
#[[#include "Core.h"]]#
#[[#include "ECS/System.h"]]#

${NAMESPACES_OPEN}

class ${NAME} : public System {
public:
    ${NAME}() : System("${NAME}") {}
};

${NAMESPACES_CLOSE}