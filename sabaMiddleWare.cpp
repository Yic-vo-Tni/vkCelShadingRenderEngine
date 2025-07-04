#include <Saba/Base/Path.h>
#include <Saba/Model/MMD/PMXModel.h>
#include <fstream>
#include <atomic>
#include "iostream"
#include "filesystem"


int main(int argc, char* argv[]){
    std::string pt = argv[1];
//    std::string pt = "E:\\Material\\model\\Nilou\\Nilou.pmx";

    auto mResDir = saba::PathUtil::GetExecutablePath();
    mResDir = saba::PathUtil::GetDirectoryName(mResDir);
    mResDir = saba::PathUtil::Combine(mResDir, "resource");

    auto mMmdDir = saba::PathUtil::Combine(mResDir, "mmd");

    auto pmx = std::make_unique<saba::PMXModel>();
    pmx->Load(pt, mMmdDir);

    std::vector<std::string> ptMats;

    for(auto i = 0; i < pmx->GetMaterialCount(); i++){
        ptMats.emplace_back(pmx->GetMaterials()[i].m_texture);
    }

    std::vector<uint16_t> indices16;
    for(auto i = 0; i < pmx->GetIndexCount(); i++){
        auto x = static_cast<const uint16_t * >(pmx->GetIndices())[i];
        indices16.push_back(x);
    }
//
//    for(const auto & ptMat : ptMats){
//        std::cout << ptMat << std::endl;
//    }

    std::string dirPath = R"(H:\VkCelShadingRenderer\renderer\resource\webview)";
    if (!std::filesystem::exists(dirPath)) {
        std::filesystem::create_directories(dirPath);
    }

    std::ofstream outFile(dirPath + "\\tex.txt", std::ios::trunc);
    if (!outFile) {
        std::cerr << "文件打开失败: " << std::endl;
    }

    if (ptMats.empty()) {
        std::cerr << "警告: 无材质数据可写入!" << std::endl;
    }

    for (auto& mat : ptMats) {
        outFile << mat << std::endl;
    }
    outFile.close();

    return 0;
}