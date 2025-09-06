//
// Created by lenovo on 9/27/2024.
//

#ifndef VKCELSHADINGRENDERER_FILEOPERATION_H
#define VKCELSHADINGRENDERER_FILEOPERATION_H


namespace fo{
    namespace fs = std::filesystem;

    inline std::string loadFile(const vot::string& filename, bool binary = true)
    {
        std::string result;
        std::ifstream stream(filename.c_str(), std::ios::ate | (binary ? std::ios::binary : std::ios_base::openmode(0)));

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

    inline auto readFile(const vot::string& path) -> vot::vector<unsigned char> {
        auto wpt = boost::locale::conv::utf_to_utf<wchar_t>(path.c_str());
        HANDLE fileHandle = CreateFileW(wpt.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fileHandle == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to open file" << std::endl;
            return {};
        }

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(fileHandle, &fileSize)) {
            CloseHandle(fileHandle);
            std::cerr << "Failed to get file size" << std::endl;
            return {};
        }

        vot::vector<unsigned char> buffer(static_cast<size_t>(fileSize.QuadPart));
        DWORD bytesRead;
        if (!ReadFile(fileHandle, buffer.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr) || bytesRead != fileSize.QuadPart) {
            CloseHandle(fileHandle);
            std::cerr << "Failed to read file" << std::endl;
            return {};
        }

        CloseHandle(fileHandle);
        return buffer;
    }

    inline fs::path findFileInDirectory(const fs::path& directory, const fs::path& filename){
        if(!fs::exists(directory) || !fs::is_directory(directory)){
            yic::logger->error("the directory is valid");
        }

        for(const auto& subDir : fs::recursive_directory_iterator(directory)){
            if (subDir.is_regular_file()){
                if(subDir.path().filename().wstring() == filename.wstring()){
                    return subDir.path();
                }
            }
        }

        yic::logger->error("failed to find the file in the directory");
        return {};
    }
    inline std::optional<fs::path> findFileInDirectory(const fs::path& directory, const char* path){
        auto pt = boost::locale::conv::utf_to_utf<wchar_t>(path);
        fs::path filename = pt;
        if (!fs::exists(directory) || !fs::is_directory(directory)){
            yic::logger->error("the directory is valid");
        }

        for(const auto& subDir : fs::recursive_directory_iterator(directory)){
            if (subDir.is_regular_file()){
                if (subDir.path().filename().wstring() == filename.filename()){
                    return subDir.path();
                }
            }
        }

        return std::nullopt;
    }

    inline auto utf8_to_utf16(const char* pt) -> std::basic_string<wchar_t>{
        return boost::locale::conv::utf_to_utf<wchar_t>(pt);
    }

    inline auto utf16_to_utf_8(const fs::path& pt) -> vot::string{
        return boost::locale::conv::utf_to_utf<char>(pt.u16string()).c_str();
    }
//
//    inline std::string getFileNameFromPath(const std::string& path){
//        size_t pos = path.find_last_of("/\\");
//        if(pos != std::string::npos && pos + 1 < path.size()){
//            return path.substr(pos + 1);
//        }
//
//        return path;
//    }
//
//    struct path{
//        static inline std::string spv(const std::string& path){
//            std::regex pattern("(\\.[^\\.]+)$");
//            std::string replace = ".spv";
//            auto spvPath = std::regex_replace(path, pattern, replace);
//            auto p = spv_path + std::string("/") + spvPath;
//            return p;
//        }
//    };


}

#endif //VKCELSHADINGRENDERER_FILEOPERATION_H
