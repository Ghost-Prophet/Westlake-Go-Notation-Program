#include "canvas.h"
#include <wx/filedlg.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/choice.h>
#include <wx/msgdlg.h>
#include <vector>
#include <memory>
#include <wx/choicdlg.h>  // wxGetSingleChoice
#include <wx/log.h>       // wxLogError
#include <algorithm>      // std::find_if

canvas::canvas() :
    stone{
        {wxT("BLACK"), wxBITMAP_TYPE_PNG_RESOURCE},
        {wxT("WHITE"), wxBITMAP_TYPE_PNG_RESOURCE}},
    palete(sizeTotal, sizeTotal)
{
    selectBackgroundFromPreset();
    preparePalete();
    board.push(game_board());
}

void canvas::selectBackgroundFromPreset()
{
    // 可选的棋盘背景图片
    std::vector<std::pair<wxString, wxString>> backgrounds = {
        {wxT("木质纹理"), wxT("../art/wooden_texture.png")},
        {wxT("紫白"), wxT("../art/背景1 紫白.png")},
        {wxT("黄绿"), wxT("../art/背景2 黄绿.png")},
        {wxT("淡橙"), wxT("../art/背景4 淡橙.png")}
    };

    // 创建选择框
    wxArrayString choices;
    for (const auto& bg : backgrounds)
    {
        choices.Add(bg.first);
    }

    wxString selected = wxGetSingleChoice(wxT("请选择棋盘背景"), wxT("棋盘背景选择"), choices);

    if (selected.IsEmpty())
    {
        wxLogError("未选择任何图片，使用默认背景。");
        return;
    }

    // 根据用户选择加载图片
    auto it = std::find_if(backgrounds.begin(), backgrounds.end(), [&](const auto& bg) {
        return bg.first == selected;
    });

    if (it != backgrounds.end())
    {
        wxImage chosenImage;
        if (!chosenImage.LoadFile(it->second))
        {
            wxLogError("无法加载图片文件：%s", it->second);
            return;
        }

        chosenImage.Rescale(sizeTotal, sizeTotal, wxIMAGE_QUALITY_HIGH);
        palete = wxBitmap(chosenImage); // 设置背景
        wxLogMessage("对局开始！");
    }
    else
    {
        wxLogError("未知错误，无法加载选定图片。");
    }
}

void canvas::preparePalete()
{
    wxMemoryDC memDC{palete};

    // 如果 palete 是空白位图，则清空背景
    if (!palete.IsOk())
    {
        memDC.Clear();
    }

    /* 绘制棋盘背景 */
    memDC.SetPen(wxPen(*wxBLACK, 2, wxPENSTYLE_SOLID));

    // 初始化棋盘格子（空棋盘）
    int x{initial}, y{initial};
    for (int i = 0; i <= gridCount; ++i, y += gridSize)
    {
        memDC.DrawLine(x, y, x + length, y);
        memDC.DrawLine(y, x, y, x + length);
    }
    memDC.SetBrush(*wxBLACK_BRUSH);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            memDC.DrawCircle(wxPoint{stars[i], stars[j]}, 3);

    memDC.SelectObject(wxNullBitmap);
}
void canvas::set_game_board(const game_board& new_board)
{
    while (!board.empty())
        board.pop(); // 清空历史记录
    board.push(new_board); // 设置新棋盘
}
game_board* canvas::get_current_board()
{
    if (board.empty())
        return nullptr;
    return &board.top();
}
void canvas::realise(wxDC& target)
{
    // 画棋盘背景
    target.DrawBitmap(palete, wxPoint{0, 0});
    // 棋子的设备上下文
    wxMemoryDC stoneDC[]{{stone[0]}, {stone[1]}};
    const wxSize sz = stone[0].GetSize();

    // 默认字体大小
    target.SetFont(wxFont{wxFontInfo{10}.Bold().FaceName("Microsoft JhengHei")});

    // 画棋子
    for (int i = 1; i <= 19; ++i)
    for (int j = 1; j <= 19; ++j)
    {
        const int x{i * gridSize - gridSize / 2}, y{j * gridSize - gridSize / 2};
        wxMemoryDC* curStone;
        if (board.top()[i][j] == game_board::Black)
        {
            curStone = stoneDC;
            target.SetTextForeground(*wxWHITE);
        }
        else if (board.top()[i][j] == game_board::White)
        {
            curStone = stoneDC + 1;
            target.SetTextForeground(*wxBLACK);
        }

        // 画棋子
        if (board.top()[i][j] != game_board::Blank)
        {
            target.StretchBlit(wxPoint{x, y}, wxSize{gridSize, gridSize}, curStone, wxPoint{0, 0}, sz);

            // 如果位置有记录，画数字
            int cnt = board.top().record[i][j];
            if (cnt != 0)
            {
                wxString label = wxString::Format("%d", cnt);

                // 如果数字 >= 100，使用小一点的字体
                if (cnt >= 100) {
                    target.SetFont(wxFont{wxFontInfo{8}.Bold().FaceName("Microsoft JhengHei")}); // 小字体
                } else {
                    target.SetFont(wxFont{wxFontInfo{10}.Bold().FaceName("Microsoft JhengHei")}); // 默认字体
                }

                wxSize textSize = target.GetTextExtent(label);
                wxPoint pos{i * gridSize - textSize.x / 2, j * gridSize - textSize.y / 2};
                target.DrawText(label, pos);
            }

            //如果当前位置是死棋，绘制红叉
            if (can->is_dead(i, j)) {
                // 红叉位置
                int xPos = i * gridSize;
                int yPos = j * gridSize;

                target.SetPen(wxPen(*wxRED, 5));  // 设置红色线条，2像素宽
                target.SetBrush(*wxTRANSPARENT_BRUSH);  // 设置透明刷子，避免填充
                
                // 绘制红色交叉线
                target.DrawLine(xPos - (gridSize-13) / 2, yPos - (gridSize-13) / 2, xPos + (gridSize-13) / 2, yPos + (gridSize-13) / 2);  // 对角线1
                target.DrawLine(xPos - (gridSize-13) / 2, yPos + (gridSize-13) / 2, xPos + (gridSize-13) / 2, yPos - (gridSize-13) / 2);  // 对角线2
            }
        }
    }


    // 恢复默认字体
    target.SetFont(wxNullFont);
}

void canvas::load(std::istream& input)
{
    clear();
    for (int i = 1; i <= 19; ++i)
        for (int j = 1; j <= 19; ++j)
            input >> board.top()[i][j];
}

void canvas::save(std::ostream& output)
{
    for (int i = 1; i <= 19; ++i)
    {
        output << board.top()[i][1];
        for (int j = 2; j <= 19; ++j)
            output << ' ' << board.top()[i][j];
        output << std::endl;
    }
}