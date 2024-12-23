#pragma once
#include <wx/dc.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/window.h>
#include <wx/bitmap.h>
#include <wx/log.h>
#include <stack>
#include <iostream>

#include "game.h"
#define ll int

class canvas
{
protected:
    wxBitmap stone[2];
    wxBitmap palete;
    std::stack<game_board> redos;
    // 棋盘和初始界面
    game_board* can;
    int cnt=0;
    int v[25][25];

public:
    canvas();
    std::stack<game_board> board;
    void set_game_board(const game_board& board);
    game_board* get_current_board();
    // 通过指针初始化
    void set_game_board(game_board* board) {
        can = board;
    }

    // 修改方法，不再创建局部变量 can，而是直接操作成员变量 can
    void add_dead_marker(int x, int y) {
        if (!can) {
            wxLogError("Game board pointer is null!");
            return;
        }
        wxLogDebug("add点击位置: (%d, %d)", x, y);
        if (x >= 0 && x < 21 && y >= 0 && y < 21) {
            can->dead_stones[x][y] = true;  // 修改 frame.cpp 中的 can
        }
    }

    void remove_dead_marker(int x, int y) {
        if (!can) {
            wxLogError("Game board pointer is null!");
            return;
        }
        wxLogDebug("rem点击位置: (%d, %d)", x, y);
        if (x >= 0 && x < 21 && y >= 0 && y < 21) {
            can->dead_stones[x][y] = false;  // 修改 frame.cpp 中的 can
        }
    }

    void realise(wxDC& target);

    game_board& get_board() {
        return board.top();
    }

    bool can_place(int colour, int x, int y) {
        return board.top().can_place(colour, x, y);
    }

    bool is_dead(int x, int y) {
        if (!can) {
            wxLogError("Game board pointer is null!");
            return false;
        }
        wxLogDebug("is dead 点击位置: (%d, %d)", x, y);
        return can->dead_stones[x][y];
    }
    ll dfs(ll x,ll y){
        if(board.top()[x][y]!=0)return board.top()[x][y];
        if(x==0||y==0||x>19||y>19)return 0;
        if(v[x][y])return 0;
        static constexpr int dx[] {-1, 0, 1, 0};
        static constexpr int dy[] {0, -1, 0, 1};
        ll res=0;cnt++,v[x][y]=1;
        for(ll _=0;_<4;_++){
            ll cur=dfs(x+dx[_],y+dy[_]);
            if(!cur)continue;
            if(!res){res=cur;continue;}
            if(cur!=res)return 2;
        }
        // wxLogError("i=%d j=%d r=%d\n",x,y,res);
        return res;
    }
    rec calc(){
        for(ll i=0;i<=20;i++)
            for(ll j=0;j<=20;j++)
                if(can->dead_stones[i][j]==true)board.top()[i][j]=0;
        for(ll i=0;i<=20;i++)
            for(ll j=0;j<=20;j++)v[i][j]=0;
        for(ll i=0;i<=20;i++)board.top()[i][0]=board.top()[i][20]=board.top()[0][i]=board.top()[20][i]=0;
        DB resw=0,resb=0;cnt=0;
        for(ll i=1;i<=19;i++)
            for(ll j=1;j<=19;j++){
                if(board.top()[i][j]==-1){resw++,v[i][j]=1;continue;}
                if(board.top()[i][j]==1){resb++,v[i][j]=1;continue;}
                ll cur=dfs(i,j);
                if(cur==-1)resw+=cnt;
                else if(cur==1)resb+=cnt;
                else if(cur==2)resw+=cnt/2.0,resb+=cnt/2.0;
                cnt=0;
            }
        return (rec){resw,resb};
    }

    void place(int colour, int x, int y) {
        game_board tmp{board.top()};
        tmp.place(colour, x, y);
        tmp.record_step(x, y);
        board.push(tmp);
        while (!redos.empty()) {
            redos.pop();
        }
    }

    void load(std::istream& input);
    void save(std::ostream& output);

    void clear() {
        while (!redos.empty()) {
            redos.pop();
        }
        while (!board.empty()) {
            board.pop();
        }
        board.push(game_board());
    }

    bool can_undo() {
        return board.size() > 1;
    }

    void undo() {
        if (can_undo()) {
            redos.push(board.top());
            board.pop();
        } else {
            wxLogError("Cannot undo, no more moves to undo.");
        }
    }

    bool can_redo() {
        return !redos.empty();
    }

    void redo() {
        if (can_redo()) {
            board.push(redos.top());
            redos.pop();
        } else {
            wxLogError("Cannot redo, no moves to redo.");
        }
    }

private:
    void selectBackgroundFromPreset();
    void preparePalete();
};