//
// Created by lenovo on 7/19/2024.
//

#ifndef VKCELSHADINGRENDERER_SHADEREDITOR_H
#define VKCELSHADINGRENDERER_SHADEREDITOR_H

namespace se {

    inline wxDECLARE_EVENT(CUSTOM_EVT, wxCommandEvent);
    inline wxDEFINE_EVENT(CUSTOM_EVT, wxCommandEvent);

    inline wxString filePath;

    class App : public wxApp {
    public:
        bool OnInit() override;
    };

    class Frame : public wxFrame {
    public:
        explicit Frame(const wxString &title);

        void onSave(wxCommandEvent &event);

        void openFile(const wxString &path);

        void onCustomEvent(wxCommandEvent &event);

        static Frame *Instance();

        wxString filePath;
    private:
        static Frame *instance;
        wxButton *saveButton;
        wxAuiNotebook *notebook;
    };

    inline Frame *Frame::instance = nullptr;

    inline Frame *Frame::Instance() {
        return instance;
    }

    inline bool App::OnInit() {
        auto *frame = new Frame("shader editor");
        frame->Show(true);
        frame->openFile(filePath);
        return true;
    }

    inline Frame::Frame(const wxString &title)
            : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1000, 700)) {
        notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                     wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS |
                                     wxAUI_NB_TAB_EXTERNAL_MOVE | wxNB_MULTILINE);
        SetSizer(new wxBoxSizer(wxVERTICAL));
        GetSizer()->Add(notebook, 1, wxEXPAND);

        saveButton = new wxButton(this, wxID_SAVE, wxT("Save"));
        saveButton->Bind(wxEVT_BUTTON, &Frame::onSave, this);
        GetSizer()->Add(saveButton, 0, wxALIGN_CENTER | wxALL, 5);

        instance = this;

        Bind(CUSTOM_EVT, &Frame::onCustomEvent, this);
    }

    inline void Frame::onCustomEvent(wxCommandEvent &event) {
        wxString path = event.GetString();
        openFile(path);
    }


    inline void Frame::onSave(wxCommandEvent &event) {
        int selection = notebook->GetSelection();
        if (selection != wxNOT_FOUND) {
            auto *textCtrl = dynamic_cast<wxStyledTextCtrl *>(notebook->GetPage(selection));
            if (textCtrl) {
                wxString path = notebook->GetPageToolTip(selection);
                wxFile file;
                if (file.Open(path, wxFile::write)) {
                    file.Write(textCtrl->GetText());
                    file.Close();
//                    wxMessageBox("File saved successfully!", "Info", wxOK | wxICON_INFORMATION);
                } else {
                    wxMessageBox("Failed to save file!", "Error", wxOK | wxICON_ERROR);
                }
            }
        }
    }

    inline void Frame::openFile(const wxString &path) {
        auto *textCtrl = new wxStyledTextCtrl(notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
        wxFile file;
        wxString content;

        wxFont font(11, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("JetBrains Mono"));
        textCtrl->StyleSetFont(wxSTC_STYLE_DEFAULT, font);

        textCtrl->StyleClearAll();

        textCtrl->SetLexer(wxSTC_LEX_CPP);

        textCtrl->SetKeyWords(0, wxT("layout"));
        textCtrl->StyleSetForeground(wxSTC_C_WORD, wxColour(150, 0, 0));
        textCtrl->StyleSetBold(wxSTC_C_WORD, true);

        textCtrl->SetKeyWords(1, wxT("location"));
        textCtrl->StyleSetForeground(wxSTC_C_WORD2, wxColour(0, 0, 150));
        textCtrl->StyleSetBold(wxSTC_C_WORD, true);

        textCtrl->StyleSetForeground(wxSTC_C_COMMENT, wxColour(150, 150, 150));
        textCtrl->StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(150, 150, 150));
        textCtrl->StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(150, 150, 150));
        textCtrl->StyleSetForeground(wxSTC_C_NUMBER, wxColour(0, 150, 0));
        textCtrl->StyleSetForeground(wxSTC_C_STRING, wxColour(150, 0, 0));
        textCtrl->StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(150, 150, 0));
        textCtrl->StyleSetForeground(wxSTC_C_OPERATOR, wxColour(150, 150, 150));
        textCtrl->StyleSetForeground(wxSTC_C_IDENTIFIER, wxColour(0, 0, 0));
        textCtrl->StyleSetForeground(wxSTC_C_STRINGEOL, wxColour(0, 0, 0));

        if (file.Open(path)) {
            file.ReadAll(&content);
            textCtrl->SetText(content);
        }

        notebook->AddPage(textCtrl, wxFileNameFromPath(path), true);
        notebook->SetPageToolTip(notebook->GetPageCount() - 1, path);
    }
}


namespace yic{

    class ShaderEditor {
    public:
        vkGet auto get = []{ return Singleton<ShaderEditor>::get(); };
        ShaderEditor();

        static auto openShaderFile(const std::string& path) -> void;
        static auto end() -> void;

    private:
        std::atomic_bool mCheckEndLoop{false};
        std::shared_ptr<std::thread> mShaderEditorThread{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_SHADEREDITOR_H
