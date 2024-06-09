//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_FILEOPERATION_H
#define VKCELSHADINGRENDERER_FILEOPERATION_H

namespace fo{

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

}


#endif //VKCELSHADINGRENDERER_FILEOPERATION_H
