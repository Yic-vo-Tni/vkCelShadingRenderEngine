//
// Created by lenovo on 8/7/2024.
//

#ifndef VKCELSHADINGRENDERER_PMXLOADER_H
#define VKCELSHADINGRENDERER_PMXLOADER_H

#include <Saba/Base/Path.h>
#include <Saba/Base/File.h>
#include <Saba/Base/UnicodeUtil.h>
#include <Saba/Base/Time.h>

#include "ModelStruct.h"
#include "Engine/Core/FileOperator/FileOperation.h"
#include "Engine/RHI/Descriptor.h"

//namespace sc {
//
//    class PmxLoader {
//        using pt = std::filesystem::path;
//    public:
//        vkGet auto get = []{ return Singleton<PmxLoader>::get(); };
//        PmxLoader();
//
//        static auto Load(const std::string& path, PipelineDesSetLayout& setLayout) -> Pmx;
//
//    private:
//        std::string mResDir;
//        std::string mMmmDir;
//    };
//
////
////    class VmdLoader{
////        VmdLoader(Pmx& pmx, const std::string& path);
////    };
//
//} // yic

#endif //VKCELSHADINGRENDERER_PMXLOADER_H
