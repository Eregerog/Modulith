/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include <utils/Result.h>

namespace modulith{
    /**
     * This class contains utilities for reading and parsing YAML files using cpp-yaml
     */
    class ENGINE_API YamlUtils {
    public:
        /**
         * Tries to open and parse the yaml file at a given path
         * @param path The path to the yaml file. An error is returned if the file does not exist
         * @return Either a yaml-node when the file was parsed sucessfully or an error
         */
        static Result<YAML::Node> TryLoadFromFile(const std::filesystem::path& path){
            if(!std::filesystem::exists(path)) return Error("The file does not exist");
            try{
                return YAML::LoadFile(path.generic_string());
            }
            catch(YAML::BadFile&)
            {
                return Error("The file could not be parsed");
            }
        }

        /**
         * Tries to parse the given yaml-node as a specific type and returns the result
         * @tparam T The type the yaml node should be parsed as.
         * This should either be one of the implicitly supported types,
         * or in the case of a custom type should have its own converter.
         * @param node A result of the node that should be parsed
         * @return A result with either the parsed value of an error
         */
        template<class T>
        static Result<T> TryParse(const Result<YAML::Node>& node){
            if(!node)
                return node.GetError();
            try{
                return node.GetResult().as<T>();
            }
            catch(YAML::InvalidNode& invalidNode){
                return Error("Parsing error: " + invalidNode.msg);
            }
        }

        /**
         * Encodes the data into yaml and writes it to a file
         * @tparam T The type of data to encode. Must have a YAML::convert template specialization
         * @param toWrite The data to encode
         * @param path The path of the file to write the yaml into
         */
        template<class T>
        static void WriteToFile(const T& toWrite, const std::filesystem::path& path){
            auto node = YAML::convert<T>::encode(toWrite);

            fs::create_directories(std::filesystem::is_directory(path) ? path : path.parent_path());

            std::ofstream stream{};
            stream.open(fs::absolute(path));
            stream << node;
            stream.flush();
            stream.close();
        }
    };
}
