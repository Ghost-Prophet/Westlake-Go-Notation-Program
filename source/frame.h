#pragma once
#include <wx/frame.h>
#include <wx/panel.h>
#include "canvas.h"

enum MenuID
{
    ID_Open = 1000,
    ID_Save,
    ID_SaveAs,
    ID_Exit,
    ID_Clear,
    ID_Undo,
    ID_Redo,
    ID_ModeFlipKeep,
    ID_ModeBlackWhite,
    ID_About
};
enum {
    ID_EndGame = wxID_HIGHEST + 1,   // 这里的 wxID_HIGHEST 是 wxWidgets 自带的最大 ID
    ID_EndMarkDead,
    // 其他 ID
};
class frame : public wxFrame
{
private:
    bool isDeadPhase = false;
    int gameMode{0};
    // wxPanel* m_canvas;
    int lastColour{game_board::White};
    wxMenu* menuEdit;

    std::stack<int> colours, colRedos;
    std::string file;
public:
    frame();
    canvas can;
    void OnEndMarkDead(wxCommandEvent& event);
    void OnEndGame(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void OnPaint(wxPaintEvent& event);

    void OnLeftUp(wxMouseEvent& event);
    void OnRightUp(wxMouseEvent& event);

    void OnFlipKeep(wxCommandEvent& event);
    void OnBlackWhite(wxCommandEvent& event);

    void OnUndo(wxCommandEvent& event);
    void OnRedo(wxCommandEvent& event);
};

//Build up the fraction