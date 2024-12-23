#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>
#include <fstream>
#include "frame.h"
#include <vector>
#include <functional>
// #include <iostream>
// using std::cout;
// using std::endl;

frame::frame()
    : wxFrame(
        nullptr, wxID_ANY, wxT("Go++"),
        wxDefaultPosition, wxDefaultSize,
        wxCAPTION | wxMINIMIZE_BOX | wxCLOSE_BOX | wxSYSTEM_MENU)
{
    game_board* initial_board = new game_board();
    can.set_game_board(initial_board);

    // wxBoxSizer* sizer = new wxBoxSizer{wxVERTICAL};
    // m_canvas = new wxPanel(this);
    // sizer->Add(m_canvas, 1, wxEXPAND);
    // SetSizer(sizer);
    // Layout();

    auto mainMenu = new wxMenuBar(0l);
auto menuGame = new wxMenu(0l);
menuGame->Append(ID_EndGame, wxT("结束下棋(&G)\tCtrl-G"), wxT("结束当前对局，进入死子标记阶段。"));
menuGame->Append(ID_EndMarkDead, wxT("结束标记死子(&D)\tCtrl-D"), wxT("结束当前的标记死子操作，结算棋局。"));
mainMenu->Append(menuGame, wxT("游戏(&G)"));
Bind(wxEVT_MENU, &frame::OnEndGame, this, ID_EndGame);  // 使用 frame 类名

    auto menuFile = new wxMenu(0l);
    menuFile->Append(ID_Open, wxT("打开(&O)\tCtrl-O"), wxT("打开以前的局面。"));  //打开存档
    menuFile->Append(ID_Save, wxT("保存(&S)\tCtrl-S"), wxT("保存当前局面。"));  //保存棋谱
    menuFile->Append(ID_SaveAs, wxT("另存为(&A)\tCtrl-Shift-S"), wxT("当前局面另存到其他文件。"));  //另存为棋谱
    menuFile->AppendSeparator();
    menuFile->Append(ID_Clear, wxT("清空(&C)\tCtrl-D"), wxT("清空当前的棋盘。"));  //清空当前棋谱
    menuFile->AppendSeparator();
    menuFile->Append(ID_Exit, wxT("退出(&X)\tAlt-F4"), wxT("退出程序。"));  //正常退出
    mainMenu->Append(menuFile, wxT("文件(&F)"));  //不太确定功能

    menuEdit = new wxMenu(0l);
    menuEdit->Append(ID_Undo, wxT("撤销(&U)\tCtrl-Z"), wxT("撤销上一步操作。"));  
    menuEdit->Append(ID_Redo, wxT("重复(&R)\tCtrl-Y"), wxT("重做上一步操作。"));
    mainMenu->Append(menuEdit, wxT("编辑(&E)"));  //应该是关于棋谱信息的编辑

    auto menuMode = new wxMenu(0l);
    menuMode->AppendRadioItem(ID_ModeFlipKeep, wxT("翻转/保持(&K)\tCtrl-K"), wxT("按鼠标左键更换棋子颜色，按鼠标右键保持棋子颜色。"));
    menuMode->AppendRadioItem(ID_ModeBlackWhite, wxT("黑棋/白棋(&A)\tCtrl-A"), wxT("按鼠标左键落黑子，按鼠标右键落白子。"));
    mainMenu->Append(menuMode, wxT("鼠标模式(&M)"));
    //通过左右键确定黑白棋子，需要修改

    auto menuHelp = new wxMenu(0l);
    menuHelp->Append(ID_About, wxT("关于(&A)...\tF12"), wxT("关于这个程序和作者。"));
    mainMenu->Append(menuHelp, wxT("帮助(&H)"));
    //关于app（“帮助”）

    SetMenuBar(mainMenu);

    menuEdit->Enable(ID_Undo, false);
    menuEdit->Enable(ID_Redo, false);

    Bind(wxEVT_MENU, &frame::OnOpen, this, ID_Open);  //注册，登陆
    Bind(wxEVT_MENU, &frame::OnSave, this, ID_Save);  //保存
    Bind(wxEVT_MENU, &frame::OnSaveAs, this, ID_SaveAs);
    Bind(wxEVT_MENU, &frame::OnClear, this, ID_Clear);  //清空
    Bind(wxEVT_MENU, &frame::OnExit, this, ID_Exit);  
    Bind(wxEVT_MENU, &frame::OnAbout, this, ID_About);
    Bind(wxEVT_MENU, &frame::OnEndMarkDead, this, ID_EndMarkDead);
    Bind(wxEVT_PAINT, &frame::OnPaint, this);

    Bind(wxEVT_LEFT_UP, &frame::OnLeftUp, this);
    Bind(wxEVT_RIGHT_UP, &frame::OnRightUp, this);
    //左右键

    Bind(wxEVT_MENU, &frame::OnFlipKeep, this, ID_ModeFlipKeep);
    Bind(wxEVT_MENU, &frame::OnBlackWhite, this, ID_ModeBlackWhite);

    Bind(wxEVT_MENU, &frame::OnUndo, this, ID_Undo);  //撤销
    Bind(wxEVT_MENU, &frame::OnRedo, this, ID_Redo);  //重置

    CreateStatusBar();

    SetClientSize(sizeTotal, sizeTotal);
	CentreOnScreen(wxBOTH);

    Refresh(true);

    colours.push(game_board::White);
}
void frame::OnEndMarkDead(wxCommandEvent& event)
{
    if (!isDeadPhase) {
        wxLogError("当前不在死子标记阶段，无法结束标记。");
        return;
    }

    // 调用can.calc()来获取目数
    rec score = can.calc();

    // 弹出结算窗口，显示黑棋和白棋的目数
    wxString result = wxString::Format(_("黑棋：%.1lf 子\n白棋：%.1lf 子"), score.b, score.w);
    
    wxMessageDialog dialog(this, result, _("结算结果"), wxOK | wxICON_INFORMATION);
    dialog.ShowModal();

    // 结束标记死子的阶段
    isDeadPhase = 0;
}
void frame::OnOpen(wxCommandEvent& event)
{
    if (can.can_undo())
    {
        if (wxMessageBox(_("当前的棋谱还没有保存，打开已有棋谱后当前棋谱将会丢失，要继续吗？"), _("确认放弃当前棋谱"),
                         wxICON_QUESTION | wxYES_NO, this) == wxNO )
            return;
        //else: proceed asking to the user the new file to open
    }
    
    wxFileDialog openFileDialog{
        this, _("打开盘面存档文件"), ".", "",
        "盘面存档文件(*.go)|*.go|所有文件(*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST
    };
    //打开存档
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;     //突然不想这么做了……
    
    // proceed loading the file chosen by the user;
    // this can be done with e.g. wxWidgets input streams:
    std::ifstream input_stream{openFileDialog.GetPath().mb_str(), std::ios::in};
    if (! input_stream)
    {
        wxLogError("未能打开文件“%s”。", openFileDialog.GetPath());
        return;
    }
    can.load(input_stream);

    Refresh(false);
}
void frame::OnEndGame(wxCommandEvent& event)
{
    game_board* current_board = can.get_current_board();
    if (!current_board)
    {
        wxLogError("当前棋盘未初始化，无法结束游戏。");
        return;
    }
    isDeadPhase = 1;  // 进入标记死棋阶段
    wxMessageBox(_("游戏结束，您现在可以标记死棋。"), _("提示"), wxOK | wxICON_INFORMATION, this);
}

void frame::OnSave(wxCommandEvent& event)
{
    // TODO: Implement OnSave
    if (file.empty())
    {
        wxFileDialog saveFileDialog{
            this, _("存储盘面存档文件"), ".", "",
            "盘面存档文件(*.go)|*.go|所有文件(*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT
        };
        if (saveFileDialog.ShowModal() == wxID_CANCEL)
            return;     // 撤销该操作
        file = saveFileDialog.GetPath().mb_str();
    }
    // save the current contents in the file;
    // this can be done with e.g. wxWidgets output streams:
    std::ofstream output_stream{file, std::ios::out};
    if (! output_stream)
    {
        wxLogError("未能打开文件“%s”。", file);
        return;
    }
    can.save(output_stream);
}

void frame::OnSaveAs(wxCommandEvent& event)
{
    // TODO: Implement OnSave
    wxFileDialog saveFileDialog{
        this, _("存储盘面存档文件"), ".", "",
        "盘面存档文件(*.go)|*.go|所有文件(*.*)|*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    };
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...
    
    // save the current contents in the file;
    // this can be done with e.g. wxWidgets output streams:
    std::ofstream output_stream{saveFileDialog.GetPath().mb_str(), std::ios::out};
    if (! output_stream)
    {
        wxLogError("未能打开文件“%s”。", saveFileDialog.GetPath());
        return;
    }
    can.save(output_stream);
    file = saveFileDialog.GetPath().mb_str();
}

void frame::OnExit(wxCommandEvent& event)
{
    if (can.can_undo())
        if (wxMessageBox(
            wxT("您正在退出，这样您可能会丢失未保存的数据，确认继续吗？"),
            wxT("警告"), wxICON_EXCLAMATION | wxOK | wxCANCEL) == wxCANCEL)
            return;
    Close();
}

void frame::OnClear(wxCommandEvent& event)
{
    if (can.can_undo())
        if (wxMessageBox(
            wxT("您正在清空当前局面，这样您可能会丢失未保存的数据，确认继续吗？"),
            wxT("警告"), wxICON_EXCLAMATION | wxOK | wxCANCEL) == wxCANCEL)
            return;
    can.clear();

    while (! colours.empty())
        colours.pop();
    while (! colRedos.empty())
        colRedos.pop();
    colours.push(game_board::White);
    Refresh(false);
}
//清空正在记录的棋谱

void frame::OnAbout(wxCommandEvent& event)
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("Westlake Go Notation Program");
    aboutInfo.SetVersion("1.0");
    aboutInfo.SetDescription(_("围棋，记谱或测试用。"));
    aboutInfo.AddDeveloper("（团队成员：田笑源，贾岚晴，俞开）");
    wxAboutBox(aboutInfo);
}

void frame::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc{this};
    can.realise(dc);
}

int px=0,py=0,ppx=0,ppy=0;

void frame::OnLeftUp(wxMouseEvent& event)
{
    // 获取鼠标点击位置
    wxPoint pos = event.GetPosition();
    int x = (pos.x + gridSize / 2) / gridSize;
    int y = (pos.y + gridSize / 2) / gridSize;

    // 如果处于死棋标记阶段，判断点击的是否是死棋标记
    if (isDeadPhase) {
        // 获取当前棋盘的顶部状态
        game_board& current_board = can.board.top();  // 获取当前的棋盘

        // 判断当前位置是否有棋子
        if (current_board[x][y] != game_board::Black && current_board[x][y] != game_board::White) {
            wxLogError("你尝试标记的这个点并没有棋子。");
            return;
        }

        // 获取当前位置的棋子颜色
        int color = current_board[x][y];

        // 存储已访问的棋子位置
        std::vector<std::vector<bool>> visited(25, std::vector<bool>(25, false));
        std::vector<std::pair<int, int>> connected_stones;  // 存储连通的棋子位置

        // 四个方向的偏移量 (上，下，左，右)
        int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

        // 使用DFS来查找连通块
        std::function<void(int, int)> dfs = [&](int cx, int cy) {
            // 如果超出边界或已经访问过，返回
            if (cx < 0 || cx >20 || cy < 0 || cy >20 || visited[cx][cy])
                return;

            // 如果当前位置颜色与目标颜色不同，返回
            if (current_board[cx][cy] != color)
                return;

            // 标记为已访问
            visited[cx][cy] = true;

            // 记录当前棋子的位置
            connected_stones.push_back({cx, cy});

            // 递归访问四个方向的相邻位置
            for (const auto& direction : directions) {
                dfs(cx + direction[0], cy + direction[1]);
            }
        };

        // 从点击的位置开始进行DFS搜索
        dfs(x, y);

        // 如果连通块中有棋子，开始标记或取消标记
        if (!connected_stones.empty()) {
            // 判断是否已经是死棋标记，如果是，则取消标记
            bool is_dead = can.is_dead(x, y);  // 这里的 `is_dead` 需要根据实际情况实现

            // 遍历连通块中的所有棋子，进行标记或取消标记
            for (const auto& stone : connected_stones) {
                int cx = stone.first;
                int cy = stone.second;

                if (is_dead) {
                    can.remove_dead_marker(cx, cy);  // 移除死棋标记
                } else {
                    can.add_dead_marker(cx, cy);     // 添加死棋标记
                }
            }
            Refresh(false);  // 刷新界面
        }
        return;
    }


    // 如果是正常的落子操作
    int thisColour = gameMode == 0 ? -colours.top() : game_board::Black;
    if (!can.can_place(thisColour, x, y))
        return;
    colours.push(thisColour);
    while (!colRedos.empty())
        colRedos.pop();
    // 开始落子
    can.place(thisColour, x, y);
    Refresh(false);

    menuEdit->Enable(ID_Undo, can.can_undo());
    menuEdit->Enable(ID_Redo, can.can_redo());
    ppx = px, ppy = py, px = x, py = y;
}

void frame::OnRightUp(wxMouseEvent& event)
{
    // 落子位置
    wxPoint pos = event.GetPosition();
    int x = (pos.x + gridSize / 2) / gridSize;
    int y = (pos.y + gridSize / 2) / gridSize;
    // 落子颜色
    // 不反转落子颜色（停一手）
    int thisColour = gameMode == 0 ? colours.top() : game_board::White;
    if (! can.can_place(thisColour, x, y))
        return;
    colours.push(thisColour);
    while (! colRedos.empty())
        colRedos.pop();
    // 开始落子
    can.place(thisColour, x, y);
    // can.get_board()[x][y] = lastColour;
    Refresh(false);

    menuEdit->Enable(ID_Undo, can.can_undo());
    menuEdit->Enable(ID_Redo, can.can_redo());
	ppx=px,ppy=py,px=x,py=y;
}

void frame::OnFlipKeep(wxCommandEvent& event)
{
    gameMode = 0;
}

void frame::OnBlackWhite(wxCommandEvent& event)
{
    gameMode = 1;
}

void frame::OnUndo(wxCommandEvent& event)
{
    can.undo();
    colRedos.push(colours.top());
    colours.pop();
    menuEdit->Enable(ID_Undo, can.can_undo());
    menuEdit->Enable(ID_Redo, can.can_redo());
    Refresh(false);
}

void frame::OnRedo(wxCommandEvent& event)
{
    can.redo();
    colours.push(colRedos.top());
    colRedos.pop();
    menuEdit->Enable(ID_Undo, can.can_undo());
    menuEdit->Enable(ID_Redo, can.can_redo());
    Refresh(false);
}
