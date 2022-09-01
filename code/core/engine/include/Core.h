/**
 * \brief
 * \author Daniel Götz
 */

#pragma once
#pragma warning (disable: 4251)

#ifdef EXPORT_ENGINE
#define ENGINE_API  __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#include "PreCompiledHeader.h"


namespace modulith {

    /**
     * The in-engine alias for a unique pointer
     */
    template<class T>
    using owned = std::unique_ptr<T>;

    /**
     * The in-engine alias for a shared pointer
     */
    template<class T>
    using shared = std::shared_ptr<T>;


    class Context;

    /**
     * This class wraps a variable and allows the value of that variable to be accessed.
     * It is implemented similar to a "point of a pointer", allowing it to work even if the variable's value changes.
     *
     * Data wrapped in this class is often returned by the context and can therefore
     * safely be copied and preserved across multiple frames, without accessing data that may no longer exist.
     * @tparam T The type of the value
     */
    template<class T>
    class ref {

        friend Context;

    public:
        /**
         * Creates a ref from a unique pointer of a subtype of T
         * @tparam V A type that is a subclass of T
         * @param underlying The unique ptr to create a reference to
         */
        template<class V, class = typename std::enable_if<!std::is_same_v<T, V> && std::is_base_of_v<T, V>>::type>
        explicit ref(owned<V>* underlying) : _getter([underlying]() { return static_cast<T*>(underlying->get()); }) {}

        /**
         * Create a reference to the value of the given unique ptr
         */
        explicit ref(owned<T>* underlying) : _getter([underlying]() { return underlying->get(); }) {}

        /**
         * Creates a reference to the given pointer
         * @warning This should only be used when underlying is a self-reference using this
         */
        explicit ref(T* underlying) : _getter([underlying]() { return underlying; }) {}

        /**
         * Access the underlying value
         * @return A pointer to the underlying value
         */
        T* operator->() const { return _getter(); }

        /**
         * Access the underlying value
         * @return A reference to the underlying value
         */
        T& operator*() const { return _getter(); }

    private:
        std::function<T*(void)> _getter;
    };

    /**
     * Logs a message with priority "trace" on behalf of the engine
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define CoreLogTrace(...) {modulith::Log::GetCoreLogger()->trace(__VA_ARGS__);}

    /**
     * Logs a message with priority "info" on behalf of the engine
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define CoreLogInfo(...) {modulith::Log::GetCoreLogger()->info(__VA_ARGS__);}

    /**
     * Logs a message with priority "warning" on behalf of the engine
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define CoreLogWarn(...) {modulith::Log::GetCoreLogger()->warn(__VA_ARGS__);}

    /**
     * Logs a message with priority "fatal" on behalf of the engine
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define CoreLogFatal(...) {modulith::Log::GetCoreLogger()->fatal(__VA_ARGS__);}

    /**
     * Logs a message with priority "error" on behalf of the engine
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define CoreLogError(...) {modulith::Log::GetCoreLogger()->error(__VA_ARGS__);}

    /**
     * Logs a message with priority "trace" on behalf of the current module or executable
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define LogTrace(...) {modulith::Log::GetClientLogger()->trace(__VA_ARGS__);}

    /**
     * Logs a message with priority "info" on behalf of the current module or executable
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define LogInfo(...) {modulith::Log::GetClientLogger()->info(__VA_ARGS__);}

    /**
     * Logs a message with priority "warning" on behalf of the current module or executable
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define LogWarn(...) {modulith::Log::GetClientLogger()->warn(__VA_ARGS__);}

    /**
     * Logs a message with priority "fatal" on behalf of the current module or executable
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define LogFatal(...) {modulith::Log::GetClientLogger()->fatal(__VA_ARGS__);}

    /**
     * Logs a message with priority "error" on behalf of the current module or executable
     * The arguments given must be the following:
     * First, a string with the text to be displayed
     * After that, all the variables to be inserted into the text
     * Example: ("Test {}, {}", 12, 15) prints "Test 12, 15"
     */
#define LogError(...) {modulith::Log::GetClientLogger()->error(__VA_ARGS__);}


#ifndef MODU_DISABLE_ASSERTS

#ifdef MODU_THROW_ON_ASSERTS

#include <stdexcept>
#define Assert(predicate, ...) { if(!(predicate)) { throw std::logic_error("Assertion failed on expression: " + #predicate); }}
#define CoreAssert(predicate, ...) { if(!(predicate)) { throw std::logic_error("Core Assertion failed on expression: " + #predicate); }}

#else
    /**
     * Checks a given condition. If it is false, print to the console on behalf of the current module or executable and interrupt the execution
     * The arguments given must be the following:
     * First, the condition to be checked
     * Second, a string with the error text to be displayed on failure
     * After that, all the variables to be inserted into the text
     * Example: (2 != 3, "This was supposed to fail all along. Error {}", 1) prints "This was supposed to fail all along. Error 1"
     */
#define Assert(predicate, ...) { if(!(predicate)) { LogError("Assertion failed on expression: {0}", #predicate); LogError(__VA_ARGS__); __debugbreak(); }}

    /**
    * Checks a given condition. If it is false, print to the console on behalf of the engine and interrupt the execution
    * The arguments given must be the following:
    * First, the condition to be checked
    * Second, a string with the error text to be displayed on failure
    * After that, all the variables to be inserted into the text
    * Example: (2 != 3, "This was supposed to fail all along. Error {}", 1) prints "This was supposed to fail all along. Error 1"
    */
#define CoreAssert(predicate, ...) { if(!(predicate)) { CoreLogError("Assertion failed on expression: {0}", #predicate); CoreLogError(__VA_ARGS__); __debugbreak(); }}

#endif

#else

#define Assert(predicate, ...)
#define CoreAssert(predicate, ...)

#endif

    /**
     * The in-engine alias for a float
     */
    using float1 = float;

    /**
     * The in-engine alias for a 2D vector of floats
     */
    using float2 = glm::vec2;

    /**
     * The in-engine alias for a 3D vector of floats
     */
    using float3 = glm::vec3;

    /**
     * The in-engine alias for a 4D vector of floats
     */
    using float4 = glm::vec4;

    /**
     * The in-engine alias for a 3x3 matrix of floats
     */
    using float3x3 = glm::mat3;

    /**
     * The in-engine alias for a 4x4 matrix of floats
     */
    using float4x4 = glm::mat4;

    /**
     * The in-engine alias for an int
     */
    using int1 = int;

    /**
     * The in-engine alias for a 2D vector of ints
     */
    using int2 = glm::vec<2, int1>;

    /**
     * The in-engine alias for a 3D vector of ints
     */
    using int3 = glm::vec<3, int1>;

    /**
     * The in-engine alias for a 4D vector of ints
     */
    using int4 = glm::vec<4, int1>;

    /**
     * The in-engine alias for a quaternion, used to represent rotations
     */
    using quat = glm::quat;

    /**
     * A data object that represents a ray in 3D-space
     */
    struct ENGINE_API Ray {

        Ray(const float3& origin, const float3& direction) : Origin(origin), Direction(direction) {}

        float3 Origin;
        float3 Direction;
    };

    /**
     * The in-engine representation for a color without an alpha value, which is a separate type to distinguish itself from float3
     */
    struct ENGINE_API color3 {

        color3() = default;
        explicit color3(const float3& value) : Value(value) {}

        float3 Value{};

        bool operator==(const color3& rhs) const {
            return Value == rhs.Value;
        }

        bool operator!=(const color3& rhs) const {
            return Value != rhs.Value;
        }
    };

    /**
     * The in-engine representation for a color, which is a separate type to distinguish itself from float4
     */
    struct ENGINE_API color4 {

        color4() = default;
        explicit color4(const float4& value) : Value(value) {}

        float4 Value{};

        bool operator==(const color4& rhs) const {
            return Value == rhs.Value;
        }

        bool operator!=(const color4& rhs) const {
            return Value != rhs.Value;
        }
    };
}

/**
 * Allows float2 to be printed using spdlog
 */
inline std::ostream& operator<<(std::ostream& os, const modulith::float2& v) {
    return os << "float2 (x: " << v.x << ", y: " << v.y << ")";
}

/**
 * Allows float3 to be printed using spdlog
 */
inline std::ostream& operator<<(std::ostream& os, const modulith::float3& v) {
    return os << "float3 (x: " << v.x << ", y: " << v.y << ", z: " << v.z << ")";
}

/**
 * Allows float4 to be printed using spdlog
 */
inline std::ostream& operator<<(std::ostream& os, const modulith::float4& v) {
    return os << "float4 (x: " << v.x << ", y: " << v.y << ", z: " << v.z << ", w: " << v.w << ")";
}

/**
 * Allows int2 to be printed using spdlog
 */
inline std::ostream& operator<<(std::ostream& os, const modulith::int2& v) {
    return os << "int2 (x: " << v.x << ", y: " << v.y << ")";
}

/**
 * Allows int3 to be printed using spdlog
 */
inline std::ostream& operator<<(std::ostream& os, const modulith::int3& v) {
    return os << "int3 (x: " << v.x << ", y: " << v.y << ", z: " << v.z << ")";
}

/**
 * Allows int4 to be printed using spdlog
 */
inline std::ostream& operator<<(std::ostream& os, const modulith::int4& v) {
    return os << "int4 (x: " << v.x << ", y: " << v.y << ", z: " << v.z << ", w: " << v.w << ")";
}

/**
 * Allows quat to be printed using spdlog
 */
inline std::ostream& operator<<(std::ostream& os, const modulith::quat v) {
    return os << "quat (x: " << v.x << ", y: " << v.y << ", z: " << v.z << ", w: " << v.w << ")";
}

namespace YAML{
    using namespace modulith;

    // The structs in this namespace allow the (de-)serialization of all of the primitive data structures / aliases defined above

    template<>
    struct convert<float2> {
        static Node encode(const float2& val) {
            Node node;
            node["x"] = val.x;
            node["y"] = val.y;
            return node;
        }

        static bool decode(const Node& node, float2& res) {
            try {
                res.x = node["x"].as<float>();
                res.y = node["y"].as<float>();
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<float3> {
        static Node encode(const float3& val) {
            Node node;
            node["x"] = val.x;
            node["y"] = val.y;
            node["z"] = val.z;
            return node;
        }

        static bool decode(const Node& node, float3& res) {

            try {
                res.x = node["x"].as<float>();
                res.y = node["y"].as<float>();
                res.z = node["z"].as<float>();
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<float4> {
        static Node encode(const float4& val) {
            Node node;
            node["x"] = val.x;
            node["y"] = val.y;
            node["z"] = val.z;
            node["w"] = val.w;
            return node;
        }

        static bool decode(const Node& node, float4& res) {

            try {
                res.x = node["x"].as<float>();
                res.y = node["y"].as<float>();
                res.z = node["z"].as<float>();
                res.w = node["w"].as<float>();
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<int2> {
        static Node encode(const int2& val) {
            Node node;
            node["x"] = val.x;
            node["y"] = val.y;
            return node;
        }

        static bool decode(const Node& node, int2& res) {
            try {
                res.x = node["x"].as<int>();
                res.y = node["y"].as<int>();
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<int3> {
        static Node encode(const int3& val) {
            Node node;
            node["x"] = val.x;
            node["y"] = val.y;
            node["z"] = val.z;
            return node;
        }

        static bool decode(const Node& node, int3& res) {

            try {
                res.x = node["x"].as<int>();
                res.y = node["y"].as<int>();
                res.z = node["z"].as<int>();
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<int4> {
        static Node encode(const int4& val) {
            Node node;
            node["x"] = val.x;
            node["y"] = val.y;
            node["z"] = val.z;
            node["w"] = val.w;
            return node;
        }

        static bool decode(const Node& node, int4& res) {

            try {
                res.x = node["x"].as<int>();
                res.y = node["y"].as<int>();
                res.z = node["z"].as<int>();
                res.w = node["w"].as<int>();
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<quat> {
        static Node encode(const quat& val) {
            Node node;
            node["x"] = val.x;
            node["y"] = val.y;
            node["z"] = val.z;
            node["w"] = val.w;
            return node;
        }

        static bool decode(const Node& node, quat& res) {

            try {
                res.x = node["x"].as<float>();
                res.y = node["y"].as<float>();
                res.z = node["z"].as<float>();
                res.w = node["w"].as<float>();
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<color3> {
        static Node encode(const color3& val) {
            Node node;
            node["r"] = val.Value.r;
            node["g"] = val.Value.g;
            node["b"] = val.Value.b;
            return node;
        }

        static bool decode(const Node& node, color3& res) {

            try {
                res.Value.r = node["r"].as<float>();
                res.Value.g = node["g"].as<float>();
                res.Value.b = node["b"].as<float>();
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<color4> {
        static Node encode(const color4& val) {
            Node node;
            node["r"] = val.Value.r;
            node["g"] = val.Value.g;
            node["b"] = val.Value.b;
            node["a"] = val.Value.a;
            return node;
        }

        static bool decode(const Node& node, color4& res) {

            try {
                res.Value.r = node["r"].as<float>();
                res.Value.g = node["g"].as<float>();
                res.Value.b = node["b"].as<float>();
                res.Value.a = node["a"].as<float>();
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<float3x3> {
        static Node encode(const float3x3& val) {
            Node node;
            node["x0"] = val[0].x;
            node["y0"] = val[0].y;
            node["z0"] = val[0].z;

            node["x1"] = val[1].x;
            node["y1"] = val[1].y;
            node["z1"] = val[1].z;

            node["x2"] = val[2].x;
            node["y2"] = val[2].y;
            node["z2"] = val[2].z;

            return node;
        }

        static bool decode(const Node& node, float3x3& res) {

            try {
                res[0].x = node["x0"].as<float>();
                res[0].y = node["y0"].as<float>();
                res[0].z = node["z0"].as<float>();

                res[1].x = node["x1"].as<float>();
                res[1].y = node["y1"].as<float>();
                res[1].z = node["z1"].as<float>();

                res[2].x = node["x2"].as<float>();
                res[2].y = node["y2"].as<float>();
                res[2].z = node["z2"].as<float>();

                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };

    template<>
    struct convert<float4x4> {
        static Node encode(const float4x4& val) {
            Node node;
            node["x0"] = val[0].x;
            node["y0"] = val[0].y;
            node["z0"] = val[0].z;
            node["w0"] = val[0].w;

            node["x1"] = val[1].x;
            node["y1"] = val[1].y;
            node["z1"] = val[1].z;
            node["w1"] = val[1].w;

            node["x2"] = val[2].x;
            node["y2"] = val[2].y;
            node["z2"] = val[2].z;
            node["w2"] = val[2].w;

            node["x3"] = val[3].x;
            node["y3"] = val[3].y;
            node["z3"] = val[3].z;
            node["w3"] = val[3].w;

            return node;
        }

        static bool decode(const Node& node, float4x4& res) {

            try {
                res[0].x = node["x0"].as<float>();
                res[0].y = node["y0"].as<float>();
                res[0].z = node["z0"].as<float>();
                res[0].w = node["w0"].as<float>();

                res[1].x = node["x1"].as<float>();
                res[1].y = node["y1"].as<float>();
                res[1].z = node["z1"].as<float>();
                res[1].w = node["w1"].as<float>();

                res[2].x = node["x2"].as<float>();
                res[2].y = node["y2"].as<float>();
                res[2].z = node["z2"].as<float>();
                res[2].w = node["w2"].as<float>();

                res[3].x = node["x3"].as<float>();
                res[3].y = node["y3"].as<float>();
                res[3].z = node["z3"].as<float>();
                res[3].w = node["w3"].as<float>();

                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };
}

