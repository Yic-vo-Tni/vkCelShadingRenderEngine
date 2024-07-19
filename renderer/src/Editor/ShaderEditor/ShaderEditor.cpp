//
// Created by lenovo on 7/19/2024.
//

#include "ShaderEditor.h"

namespace yic {


    ShaderEditor::ShaderEditor() = default;

    auto ShaderEditor::openShaderFile(const std::string& path) -> void {
        auto inst = get();
        if (inst->mShaderEditorThread && inst->mCheckEndLoop.load()){
            inst->mCheckEndLoop.store(false);
            if (inst->mShaderEditorThread->joinable()){
                inst->mShaderEditorThread->detach();
            }
            inst->mShaderEditorThread.reset();
        }
        if (!inst->mShaderEditorThread){
            inst->mShaderEditorThread = std::make_shared<std::thread>([=] {
                wxDISABLE_DEBUG_SUPPORT();
                se::filePath = path;

                auto *app = new se::App;
                wxApp::SetInstance(app);
                wxEntry();
                inst->mCheckEndLoop = true;
            });
        } else{
            if (se::Frame::Instance()){
                wxCommandEvent event(se::CUSTOM_EVT);
                event.SetString(path);
                wxPostEvent(se::Frame::Instance(), event);
            }
        }
    }

    auto ShaderEditor::end() -> void {
        auto inst = get();

        if (inst->mCheckEndLoop.load()) {
            if (inst->mShaderEditorThread->joinable()) {
                inst->mShaderEditorThread->detach();
            }
        }
    }

} // yic