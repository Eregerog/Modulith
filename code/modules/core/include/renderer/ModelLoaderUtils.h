/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include <utility>

#include "CoreModule.h"
#include "Mesh.h"
#include "Material.h"
#include "ecs/EntityManager.h"
#include "assets/AssetContext.h"

namespace fs = std::filesystem;


namespace modulith{

    struct CORE_API Model {

        Model(shared<modulith::Mesh>  mesh, shared<modulith::Material>  material) : Mesh(std::move(mesh)),
                                                                                                Material(std::move(material)) {}

        shared<Mesh> Mesh;
        shared<Material> Material;
    };

    class CORE_API ModelLoaderUtils {

    public:

        static shared<Mesh> LoadSingleFromFile(const Address& address);

        static std::vector<shared<Mesh>> LoadMeshesFromFile(const Address& address);

        static std::vector<Model> LoadSceneFromFile(const Address& address);

    };

}