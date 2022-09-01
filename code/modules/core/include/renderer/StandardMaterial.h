/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include <renderer/primitives/Texture.h>
#include "Material.h"


namespace modulith{

    /**
     * An implementation of a material for phong lighting.
     * The underlying shader needs to have the following uniforms:
     * vec3 u_Material.DiffuseColor
     * bool u_Material.HasDiffuseTex
     * sampler2D u_Material.DiffuseTex
     *
     * vec3 u_Material.SpecularColor
     * bool u_Material.HasSpecularTex
     * sampler2D u_Material.SpecularTex
     *
     * float u_Material.Shininess
     */
    class CORE_API StandardMaterial : public Material {
    public:
        /**
         * Creates a standard material with default properties
         */
        explicit StandardMaterial(const shared<Shader>& shader) : Material(shader) {}

        /**
         * Creates a standard material from the given shader with a single color
         */
        StandardMaterial(const shared<Shader>& shader, float4 color, float specularIntensity, float shininess) : Material(shader){
            DiffuseColor = color;
            SpecularColor = float4(specularIntensity);
            Shininess = shininess;
        }

        /**
        * Creates a standard material from the given shader with a single color
        */
        StandardMaterial(const shared<Shader>& shader, float4 color, float4 specularColor, float shininess) : Material(shader){
            DiffuseColor = color;
            SpecularColor = specularColor;
            Shininess = shininess;
        }

        /**
         * Creates a standard material from the given shader with textures
         */
        StandardMaterial(
            shared<Shader> shader, shared<Texture> diffuseTexture, shared<Texture> specularTexture,
            float shininess
        );

        /**
         * Uploads the standard-material specific and custom uniforms to the gpu
         */
        void UploadUniforms() override;

        /**
         * The diffuse texture of the standard material.
         * The material needs to be re-bound after changing this.
         */
        shared<Texture> DiffuseTexture = nullptr;

        /**
         * The diffuse color of the standard material.
         * The material needs to be re-bound after changing this.
         */
        float4 DiffuseColor{1.0f, 1.0f, 1.0f, 1.0f};

        /**
         * The specular texture of the standard material.
         * The material needs to be re-bound after changing this.
         */
        shared<Texture> SpecularTexture = nullptr;

        /**
         * The specular color of the standard material.
         * The material needs to be re-bound after changing this.
         */
        float4 SpecularColor{1.0f, 1.0f, 1.0f, 1.0f};

        /**
         * The shininess of the standard material.
         * The material needs to be re-bound after changing this.
         */
        float Shininess = 32;
    };
}
