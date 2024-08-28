//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_FILEOPERATION_H
#define VKCELSHADINGRENDERER_FILEOPERATION_H

#include <regex>

namespace fo{
    namespace fs = std::filesystem;

    inline std::string loadFile(const std::string& filename, bool binary = true)
    {
        std::string   result;
        std::ifstream stream(filename, std::ios::ate | (binary ? std::ios::binary : std::ios_base::openmode(0)));

        if(!stream.is_open())
        {
            return result;
        }

        result.reserve(stream.tellg());
        stream.seekg(0, std::ios::beg);

        result.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return result;
    }

    inline vk::DeviceAddress getBufDeviceAddress(vk::Device device, vk::Buffer buffer){
        if (!buffer){
            return 0;
        }

        return device.getBufferAddress(vk::BufferDeviceAddressInfo{buffer});
    }

    inline fs::path findFileInDirectory(const fs::path& directory, const fs::path& filename){
        if(!fs::exists(directory) || !fs::is_directory(directory)){
            vkError("the directory is valid");
        }

        for(const auto& subDir : fs::recursive_directory_iterator(directory)){
            if (subDir.is_regular_file()){
                if(subDir.path().filename() == filename){
                    return subDir.path();
                }
            }
        }

        vkError("failed to find the file in the directory");
        return {};
    }

    inline std::string getFileNameFromPath(const std::string& path){
        size_t pos = path.find_last_of("/\\");
        if(pos != std::string::npos && pos + 1 < path.size()){
            return path.substr(pos + 1);
        }

        return path;
    }

    struct path{
        static inline std::string spv(const std::string& path){
            std::regex pattern("(\\.[^\\.]+)$");
            std::string replace = ".spv";
            auto spvPath = std::regex_replace(path, pattern, replace);
            auto p = spv_path + std::string("/") + spvPath;
            return p;
        }
    };


}


#endif //VKCELSHADINGRENDERER_FILEOPERATION_H
