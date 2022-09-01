/*
 * \brief
 * \author Daniel Götz
 */

#include "renderer/ModelLoaderUtils.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <renderer/StandardMaterial.h>
#include "Context.h"
#include "renderer/RenderContext.h"

namespace modulith{

    shared<Mesh> createMeshFromAssimp(const shared<RenderContext>& renderCtx, const aiMesh* assimpMesh, aiMatrix4x4 sceneToMesh) {
        auto vertices = std::vector<Vertex>();
        vertices.reserve(assimpMesh->mNumVertices);
        for (unsigned int i = 0; i < assimpMesh->mNumVertices; ++i) {
            aiVector3D position = sceneToMesh * assimpMesh->mVertices[i];
            aiVector3D normal = assimpMesh->HasNormals() ? sceneToMesh * assimpMesh->mNormals[i] : aiVector3D(0,0,0);

            Vertex vertex{
                float3{position.x, position.y, position.z},
                glm::normalize(float3{normal.x, normal.y, normal.z})
            };

            if (assimpMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                float2 vec;
                vec.x = assimpMesh->mTextureCoords[0][i].x;
                vec.y = assimpMesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            } else
                vertex.TexCoords = float2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        auto indices = std::vector<uint32_t>();
        vertices.reserve(assimpMesh->mNumFaces);

        for (unsigned int i = 0; i < assimpMesh->mNumFaces; ++i) {
            aiFace face = assimpMesh->mFaces[i];
            CoreAssert(face.mNumIndices == 3,
                "Only assimp meshes with faces of exactly 3 vertices are supported! Encountered {} vertices.",
                face.mNumIndices);
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        return std::make_shared<Mesh>(vertices, indices);
    }

    void traverseAssimpSceneAndAddMeshesRec(
        const shared<RenderContext>& renderCtx,
        const aiScene* scene, aiNode* current, aiMatrix4x4 sceneToParent, std::vector<std::pair<shared<Mesh>, unsigned int>>& result
    ) {
        auto sceneToNode = sceneToParent * current->mTransformation;

        for (unsigned int i = 0; i < current->mNumMeshes; ++i) {
            auto* mesh = scene->mMeshes[current->mMeshes[i]];
            result.emplace_back(createMeshFromAssimp(renderCtx, mesh, sceneToNode), mesh->mMaterialIndex);
        }

        for (unsigned int i = 0; i < current->mNumChildren; ++i) {
            traverseAssimpSceneAndAddMeshesRec(renderCtx, scene, current->mChildren[i], sceneToNode, result);
        }
    }

    std::vector<shared<Material>> collectAssimpMaterials(const shared<RenderContext>& renderCtx, const aiScene* scene, const Address& address){
        std::vector<shared<Material>> res{};
        auto shader = renderCtx->RendererAPI()->CreateShader(Address(std::filesystem::path("shaders") / "PhongShader.glsl"));
        for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {

            auto mat = std::make_shared<StandardMaterial>(shader);

            aiColor3D diffuseColor (0.f,0.f,0.f);
            scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE,diffuseColor);
            mat->DiffuseColor = float4(diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f);

            aiString diffuseTex;
            if(scene->mMaterials[i]->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), diffuseTex) == aiReturn_SUCCESS){
                mat->DiffuseTexture = renderCtx->RendererAPI()->CreateTexture2D(address.ParentAddress() / diffuseTex.C_Str());
            }

            aiColor3D specularColor (0.f,0.f,0.f);
            scene->mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR,specularColor);
            mat->SpecularColor = float4(specularColor.r, specularColor.g, specularColor.b, 1.0f);

            aiString specularTex;
            if(scene->mMaterials[i]->Get(AI_MATKEY_TEXTURE_SPECULAR(0), diffuseTex) == aiReturn_SUCCESS){
                mat->SpecularTexture = renderCtx->RendererAPI()->CreateTexture2D(address.ParentAddress() / specularTex.C_Str());
            }

            res.push_back(mat);
        }
        return res;
    }

    std::vector<Model> loadFromAssimpScene(const Address& address, bool importMaterials){
        auto& ctx = Context::Instance();
        auto assetCtx = ctx.Get<AssetContext>();
        auto path = assetCtx->ResolveAddressOrThrow(address, "ModelLoaderUtils");

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(
            path.generic_string(),
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType
        );

        CoreAssert(scene != nullptr, "Importing of mesh at {} failed. Error: {}", path.generic_string(), importer.GetErrorString());

        auto renderCtx = ctx.Get<RenderContext>();

        std::vector<std::pair<shared<Mesh>, unsigned int>> meshes;
        traverseAssimpSceneAndAddMeshesRec(renderCtx, scene, scene->mRootNode, aiMatrix4x4(), meshes);

        std::vector<Model> res{};
        if (scene->HasMaterials() && importMaterials) {
            auto materials = collectAssimpMaterials(renderCtx, scene, address);

            for (const auto& mesh : meshes) {
                auto materialIdx = mesh.second;
                if (materialIdx >= 0 && materialIdx < materials.size()) {
                    res.emplace_back(mesh.first, materials[materialIdx]);
                } else {
                    res.emplace_back(mesh.first, nullptr);
                }
            }

        } else {
            for (const auto& mesh : meshes)
                res.emplace_back(mesh.first, nullptr);
        }

        return res;
    }

    shared<Mesh> ModelLoaderUtils::LoadSingleFromFile(const Address& address) {
        auto loadedMeshes = LoadMeshesFromFile(address);
        CoreAssert(!loadedMeshes.empty(), "There were no meshes to load at address {}", address.AsString())

        if(loadedMeshes.size() > 1)
            CoreLogWarn("LoadSingleFromFile was called on model found at {}, but the model has multiple submeshes. Only using the first", address.AsString())

        return loadedMeshes[0];
    }

    std::vector<shared<Mesh>> ModelLoaderUtils::LoadMeshesFromFile(const Address& address) {
        std::vector<shared<Mesh>> res{};
        for(const auto& model : loadFromAssimpScene(address, false)){
            res.push_back(model.Mesh);
        }
        return res;
    }

    std::vector<Model> ModelLoaderUtils::LoadSceneFromFile(const Address& address) {
        return loadFromAssimpScene(address, true);
    }

}