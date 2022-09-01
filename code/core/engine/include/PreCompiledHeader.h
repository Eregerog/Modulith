/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

// Needed because the version of cpp-yaml used is still using deprecated iterators
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

// Data structures

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <stack>
#include <queue>

// Containers

#include <memory> // Smart pointers
#include <optional>
#include <functional>
#include <variant>

// Type utils

#include <typeinfo>
#include <type_traits>
#include <typeindex>
#include <any>

// Filesystem

#include <filesystem>
#include <fstream>

// Glm

#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/ext/matrix_clip_space.hpp>

// String

#include <iostream>
#include <string>
#include <sstream>
#include <regex>

// Boolinq

#include <utils/boolinq/boolinq.h>

// Yaml

#include <yaml-cpp/yaml.h>

// Misc

#include <algorithm>

#include <chrono>
#include <utility>
#include <initializer_list>

#include <cassert>
#include <cstdint>
#include <cinttypes>

#include "profiling/Profiler.h"
#include "Log.h"
#include "spdlog/fmt/ostr.h"