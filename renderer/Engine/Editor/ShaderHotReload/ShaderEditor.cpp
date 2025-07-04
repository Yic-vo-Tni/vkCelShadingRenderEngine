//
// Created by lenovo on 6/9/2025.
//


#include "ShaderEditor.h"

#include "nlohmann/json.hpp"
#include "webview/webview.h"
#include "ShaderHotReload.h"


namespace ui {

    ShaderEditor::ShaderEditor() = default;

    ShaderEditor::~ShaderEditor() = default;

    auto ShaderEditor::build(const vot::string& pt) -> void {
        path = pt;

        std::thread( [this]{
            webview::webview webview(true, nullptr);

            webview.set_title("Shader Editor");
            webview.set_size(900, 800, WEBVIEW_HINT_NONE);

            auto b1 = webview.bind("pageLoaded", [&](const std::string & /*unused*/) -> std::string {
                auto f = openFile(shader_path + std::string (path));
                webview.eval("showMessage(`" + f + "`);");
                return "";
            });

            auto b2 = webview.bind("saveFile", [&](const std::string& newContext) -> std::string{
                auto j = nlohmann::json::parse(newContext);
                std::string realContext = j.at(0).get<std::string>();

               bool ok = writeFile(shader_path + std::string (path), realContext);

                if (ok){
                    yic::logger->info("success save");
                    yic::shaderHot->compile();
                    yic::shaderHot->update(path);
                } else {
                    yic::logger->warn("failed save");
                }

                return "";
            });

            webview.navigate(webview_path "monaco.html");

            webview.run();
        }).detach();
    }

    auto ShaderEditor::openFile(const std::string &pt) -> std::string {
        std::ifstream ifstream(pt);
        return  std::string((std::istreambuf_iterator<char>(ifstream)), std::istreambuf_iterator<char>());
    }

    bool ShaderEditor::writeFile(const std::string &pt, const std::string &context) {
        std::ofstream out(pt, std::ios::binary);
        if (!out) return false;
        out << context;
        return true;
    }

//    auto ShaderEditor::switchFile(const std::string &pt) -> void {
//        std::string context = openFile(pt);
//    }




} // ui










































//    auto ShaderEditor::registerWindow() -> bool {
//        WNDCLASS wc{};
//        wc.lpfnWndProc = wndProcStatic;
//        wc.hInstance = hInst;
//        wc.lpszClassName = reinterpret_cast<LPCSTR>(name);
//        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
//
//        return RegisterClass(&wc) != 0;
//    }
//
//    auto ShaderEditor::createWindow(int show) -> bool {
//        if (!registerWindow()) return false;
//
//        hMainWnd = CreateWindowEx(
//                0,
//                reinterpret_cast<LPCSTR>(name),
//                reinterpret_cast<LPCSTR>(L"Shader Editor"),
//                WS_OVERLAPPEDWINDOW,
//                CW_USEDEFAULT, CW_USEDEFAULT,
//                800, 600,
//                nullptr, nullptr, hInst, this
//                );
//
//        if (!hMainWnd) return false;
//
//        ShowWindow(hMainWnd, show);
//        UpdateWindow(hMainWnd);
//
//        return true;
//    }
//
//    auto ShaderEditor::bringToFront() -> void {
//        if (hMainWnd){
//            SetForegroundWindow(hMainWnd);
//            ShowWindow(hMainWnd, SW_RESTORE);
//        }
//    }
//
//    auto ShaderEditor::getShaderSource() -> vot::string {
//        if (!hEditor) return {};
//
//        auto len = (int)SendMessage(hEditor, SCI_GETTEXTLENGTH, 0, 0);
//        vot::string src(len, '\0');
//        SendMessage(hEditor, SCI_GETTEXT, len + 1, (LPARAM)src.data());
//
//        return src;
//    }
//
//    auto CALLBACK ShaderEditor::wndProcStatic(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT {
//        if (msg == WM_CREATE){
//            auto cs = reinterpret_cast<CREATESTRUCT*>(lp);
//            auto self = reinterpret_cast<ShaderEditor*>(cs->lpCreateParams);
//            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self);
//            return self->wndProc(hwnd, msg, wp, lp);
//        }
//
//        auto self = reinterpret_cast<ShaderEditor*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
//
//        return self ? self->wndProc(hwnd, msg, wp, lp) : DefWindowProc(hwnd, msg, wp, lp);
//    }
//
//    auto CALLBACK ShaderEditor::wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) -> LRESULT {
//        switch (msg) {
//            case WM_CREATE:
//                onCreate(hwnd, reinterpret_cast<LPCREATESTRUCT>(lp));
//                break;
//            case WM_SIZE:
//                onSize(LOWORD(lp), HIWORD(lp));
//                break;
//            case WM_COMMAND:
//                if (LOWORD(wp) == 1)
//                    onCompile();
//                break;
//            case WM_DESTROY:
//                PostQuitMessage(0);
//                break;
//            default:
//                return DefWindowProc(hwnd, msg, wp, lp);
//        }
//        return 0;
//    }
//
//    auto ShaderEditor::onCreate(HWND hwnd, LPCREATESTRUCT cs) -> void {
//        hMainWnd = hwnd;
//
//        Scintilla_RegisterClasses(hInst);
//        RECT rc;
//        GetClientRect(hwnd, &rc);
//        int btnH = 30;
//
//        hEditor = CreateWindowEx(0,
//                                 reinterpret_cast<LPCSTR>(L"Scintilla"),
//                                 reinterpret_cast<LPCSTR>(L""),
//                                 WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
//                                 0, 0, rc.right, rc.bottom - btnH,
//                                 hwnd, nullptr, hInst, nullptr);
//
//        SendMessage(hEditor, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM )L"Consoles");
//        SendMessage(hEditor, SCI_STYLESETSIZE, STYLE_DEFAULT, 14);
//
//        hButton = CreateWindow(
//                reinterpret_cast<LPCSTR>(L"Button"),
//                reinterpret_cast<LPCSTR>(L"Compile"),
//                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
//                10, rc.bottom - btnH + 4, 100, btnH - 8,
//                hwnd, (HMENU)1, hInst, nullptr
//                );
//    }
//
//    auto ShaderEditor::onSize(int w, int h) -> void {
//        if (!hEditor || !hButton) return;
//        int btnH = 30;
//        MoveWindow(hEditor, 0, 0, w, h - btnH, TRUE);
//        MoveWindow(hButton, 10, h- btnH + 4, 100, btnH - 8, TRUE);
//    }
//
//    auto ShaderEditor::onCompile() -> void {
//        auto src = getShaderSource();
//        MessageBoxA(hMainWnd, src.c_str(), "shader", MB_OK);
//    }
//
//    auto ShaderEditor::open(int show) -> bool {
//        if (hMainWnd){
//            bringToFront();
//            return true;
//        }
//        return createWindow(show);
//    }


