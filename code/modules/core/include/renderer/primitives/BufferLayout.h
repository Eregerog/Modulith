/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <CoreModule.h>

namespace modulith{

    /**
     * All data types of uniforms that are supported
     */
    enum class CORE_API  ShaderDataType {
        None = 0,
        Float,
        Float2,
        Float3,
        Float4,
        Mat3,
        Mat4,
        Int,
        Int2,
        Int3,
        Int4,
        Bool
    };

    /**
     * @return Returns the size (in bytes) of the given shader data type
     */
    static uint32_t ShaderDataTypeSize(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:
                return 1 * sizeof(float);
            case ShaderDataType::Float2:
                return 2 * sizeof(float);
            case ShaderDataType::Float3:
                return 3 * sizeof(float);
            case ShaderDataType::Float4:
                return 4 * sizeof(float);
            case ShaderDataType::Mat3:
                return 3 * 3 * sizeof(float);
            case ShaderDataType::Mat4:
                return 4 * 4 * sizeof(float);
            case ShaderDataType::Int:
                return 1 * sizeof(int);
            case ShaderDataType::Int2:
                return 2 * sizeof(int);
            case ShaderDataType::Int3:
                return 3 * sizeof(int);
            case ShaderDataType::Int4:
                return 4 * sizeof(int);
            case ShaderDataType::Bool:
                return sizeof(bool);
            default: CoreAssert(false, "Unsupported ShaderDataType {0}", type);
                return 0;
        }
    }

    /**
     * @return Returns the amount of components of the given data type
     */
    static uint32_t ShaderGetComponentCount(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Bool:
            case ShaderDataType::Int:
            case ShaderDataType::Float:
                return 1;
            case ShaderDataType::Int2:
            case ShaderDataType::Float2:
                return 2;
            case ShaderDataType::Int3:
            case ShaderDataType::Float3:
                return 3;
            case ShaderDataType::Int4:
            case ShaderDataType::Float4:
                return 4;
            case ShaderDataType::Mat3:
                return 9;
            case ShaderDataType::Mat4:
                return 16;
            default: CoreAssert(false, "Unsupported ShaderDataType {0}", type);
                return 0;
        }
    }

    /**
     * Represents a single data element used for each vertex inside a VertexBuffer
     */
    struct CORE_API BufferElement {
        /**
         * The name of this element, for debugging purposes
         */
        std::string Name;
        /**
         * The type of data stored
         */
        ShaderDataType Type;

        /**
         * The offset of the data (in bytes) from the beginning of the vertex's data inside the vertex buffer
         */
        uint32_t Offset{};

        /**
         * The size of the data (in bytes)
         */
        uint32_t Size{};

        /**
         * If the given value should be normalized when uploaded to the GPU
         */
        bool ShouldBeNormalized{};

        BufferElement() = default;

        /**
         * @param type The type of the BufferElement's data
         * @param name The debug name
         * @param shouldBeNormalized If the value should be normalized when uploaded to the GPU
         */
        BufferElement(const ShaderDataType& type, std::string name, bool shouldBeNormalized = false)
            : Name(std::move(name)),
              Type(type),
              ShouldBeNormalized(shouldBeNormalized),
              Size(ShaderDataTypeSize(type)),
              Offset(0) {}

        /**
         * @return Returns of how many single data components the data type is composed
         */
        [[nodiscard]] uint32_t GetComponentCount() const { return ShaderGetComponentCount(Type); }
    };

    /**
     * This class describes the layout of the data inside a VertexBuffer.
     * Because multiple variables can be stored contiguously for each Vertex,
     * a description (provided by this class) is needed
     */
    class CORE_API BufferLayout {
    public:

        /**
         * Creates a buffer layout with no elements
         */
        BufferLayout() = default;

        /**
         * Creates a buffer layout from the given elements
         * @param elements
         */
        BufferLayout(const std::initializer_list<BufferElement>& elements) : _elements(elements) {
            calculateOffset();
        }

        /**
         * @return Returns the elements inside the buffer layout
         */
        [[nodiscard]] inline const std::vector<BufferElement>& GetElements() const { return _elements; }

        /**
         * @return Returns the stride (in bytes) between the data of each vertex described by this layout
         */
        [[nodiscard]] uint32_t GetStride() const { return _stride; }

    private:

        void calculateOffset() {
            uint32_t offset = 0;
            _stride = 0;
            for (auto& element : _elements) {
                element.Offset = offset;
                offset += element.Size;
                _stride += element.Size;
            }
        }

        uint32_t _stride = 0;
        std::vector<BufferElement> _elements{};
    };

}
