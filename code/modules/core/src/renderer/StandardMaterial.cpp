/*
 * \brief
 * \author Daniel Götz
 */

#include "renderer/StandardMaterial.h"

namespace modulith{

    StandardMaterial::StandardMaterial(
        shared<Shader> shader, shared<Texture> diffuseTexture, shared<Texture> specularTexture,
        float shininess
    ) : Material(std::move(shader)), DiffuseColor(float4(1.0f)), SpecularColor(float4(1.0f)),
        DiffuseTexture(std::move(diffuseTexture)), SpecularTexture(std::move(specularTexture)), Shininess(shininess) {}


    void StandardMaterial::UploadUniforms() {
        Material::UploadUniforms();

        _shader->UploadUniformFloat3("u_Material.DiffuseColor", DiffuseColor);
        _shader->UploadUniformBool("u_Material.HasDiffuseTex", DiffuseTexture != nullptr);
        if (DiffuseTexture) {
            _shader->UploadUniformInt1("u_Material.DiffuseTex", 0);
            DiffuseTexture->Bind(0);
        }

        _shader->UploadUniformFloat3("u_Material.SpecularColor", SpecularColor);
        _shader->UploadUniformBool("u_Material.HasSpecularTex", SpecularTexture != nullptr);
        if (SpecularTexture) {
            _shader->UploadUniformInt1("u_Material.SpecularTex", 1);
            SpecularTexture->Bind(1);
        }
        _shader->UploadUniformFloat1("u_Material.Shininess", Shininess);
    }


}
