//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_FILEOPERATION_H
#define VKCELSHADINGRENDERER_FILEOPERATION_H

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



}


#endif //VKCELSHADINGRENDERER_FILEOPERATION_H
