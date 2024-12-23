#include "game.h"
#include "math.h"
#include<stdio.h>

#define DB double

game_board::game_board() {
	// 初始化棋盘
	for (int i = 0; i < 21; ++ i)
		board[0][i] = board[i][0] = board[20][i] = board[i][20] = 42;
	for (int i = 1; i <= 19; ++ i)
		for (int j = 1; j <= 19; ++ j)
			record[i][j] = board[i][j] = flags[i][j] = dead_stones[i][j]=0;
}

void game_board::remove_dead(int colour) {
	clear_flags();
	for (int i = 1; i <= 19; ++ i)
		for (int j = 1; j <= 19; ++ j) {
			clear_lib_flags();
			// 不是当前颜色，或者已经访问过
			if (board[i][j] != colour || flags[i][j] != 0)
				continue;
			// 访问当前交叉点上的一块棋
			visit(colour, i, j);
			++ count;
			// 如果没有气，就提子
			if (liberties == 0)
				remove(colour, i, j);
		}
}
//判断是否需要提子

void game_board::visit(int colour, int x, int y) {
	static constexpr int dx[] {-1, 0, 1, 0};
	static constexpr int dy[] {0, -1, 0, 1};
	// 已经访问过
	if (flags[x][y] != 0 || lib_flags[x][y] != 0)
		return;
	// 空白格子
	if (board[x][y] == Blank) {
		lib_flags[x][y] = colour;
		++ liberties;
		return;
	}
	// 非己方棋子（禁下）
	if (board[x][y] != colour)
		return;
	// 标记当前这个点
	flags[x][y] = count;
	// 访问四个方向的四个交叉点
	for (int i = 0; i < 4; ++ i)
		visit(colour, x + dx[i], y + dy[i]);
}
//判断是否可以落子到该位置

void game_board::clear_flags() {
	count = 1;
	for (int i = 1; i <= 19; ++ i)
		for (int j = 1; j <= 19; ++ j)
			flags[i][j] = 0;
}

void game_board::clear_lib_flags() {
	liberties = 0;
	for (int i = 1; i <= 19; ++ i)
		for (int j = 1; j <= 19; ++ j)
			lib_flags[i][j] = 0;
}

void game_board::remove(int colour, int x, int y) {
	static constexpr int dx[] {-1, 0, 1, 0};
	static constexpr int dy[] {0, -1, 0, 1};
	// 不是当前颜色
	if (board[x][y] != colour)
		return;
	// 提起当前这个子
	board[x][y] = Blank;
	// 四个方向的四个交叉点都提子
	for (int i = 0; i < 4; ++ i)
		remove(colour, x + dx[i], y + dy[i]);
}

void game_board::place(int colour, int x, int y) {
	clear_flags();
	board[x][y] = colour;
	remove_dead(-colour);
}

#define ll long long
bool game_board::can_place(int colour, int x, int y) {
	printf("!");
    static constexpr int dx[] {-1, 0, 1, 0};
    static constexpr int dy[] {0, -1, 0, 1};
	if (board[x][y] != Blank)
		return false;
	// 测试禁着点（禁下点）
	game_board temp {*this};

    // if(abs(px-x)+abs(py-y)==1){
    //     ll cur=1;
    //     for(ll i=0;i<4;i++){
    //         ll cx=px+dx[i],cy=py+dy[i];
    //         if(cx==x&&cy==y)continue;
    //         if(temp.board[cx][cy]==Blank||temp.board[cx][cy]==-colour)cur=0;
    //     }
    //     for(ll i=0;i<4;i++){
    //         ll cx=x+dx[i],cy=y+dy[i];
    //         if(cx==px&&cy==py)continue;
    //         if(temp.board[cx][cy]==Blank||temp.board[cx][cy]==colour)cur=0;
    //     }
	// 	if(cur&&tx==x&ty==y)return false;
    // }

	if(tx==x&&ty==y)return false;

	temp.place(colour, x, y);

	temp.clear_flags();
	for (int i = 1; i <= 19; ++ i)
		for (int j = 1; j <= 19; ++ j) {
			temp.clear_lib_flags();
			// 不是当前颜色，或者已经访问过
			if (temp.board[i][j] != colour || temp.flags[i][j] != 0)
				continue;
			// 访问当前交叉点上的一块棋
			temp.visit(colour, i, j);
			++ temp.count;
			// 如果没有气，就提子
			printf("i=%lld j=%lld lib=%lld\n",i,j,temp.liberties);
			fflush(stdout);
			if (temp.liberties == 0)
				return false;
		}
	ll cur1=0,cur2=0,ux=0,uy=0,tagg=0;
    for(ll i=0;i<4;i++){
        ll cx=x+dx[i],cy=y+dy[i];
		cur1+=temp.board[cx][cy]==-colour||cx==0||cx>19||cy==0||cy>19;
		if(!temp.board[cx][cy]&&cx&&cx<=19&&cy&&cy<=19)ux=cx,uy=cy,cur2++;
	}
	if(cur1==3&&cur2==1){
		tagg=1;
		for(ll i=0;i<4;i++){
			ll cx=ux+dx[i],cy=uy+dy[i];
			if(cx==x&&cy==y)continue;
			if(temp.board[cx][cy]^colour&&cx&&cx<=19&&cy&&cy<=19){tagg=0;break;}
		}
	}
	if(board[ux][uy]!=-colour)tagg=0;
	tx=tagg?ux:0,ty=tagg?uy:0;
	return true;
}

bool game_board::is_dead(int x, int y) {
    return dead_stones[x][y];
}

// 添加死棋标记
void game_board::mark_dead(int x, int y) {
    if (board[x][y] != Blank) {  // 只有棋子才能被标记为死棋
        dead_stones[x][y] = true;
    }
}

// 移除死棋标记
void game_board::unmark_dead(int x, int y) {
    if (dead_stones[x][y]) {
        dead_stones[x][y] = false;
        // 撤回操作时，可以恢复棋盘上的棋子，假设恢复为黑或白
        board[x][y] = (x + y) % 2 == 0 ? Black : White;  // 假设根据坐标恢复棋子
    }
}

// 处理点击事件，标记或撤回死棋标记
void game_board::on_click(int x, int y) {
    // 如果当前点击的位置已经是死棋，撤回标记
    if (is_dead(x, y)) {
        unmark_dead(x, y);
    } else {
        mark_dead(x, y);
    }
}

ll game_board::dfs(ll x,ll y){
	if(board[x][y]!=0)return board[x][y];
	if(x==0||y==0||x>19||y>19)return 0;
	static constexpr int dx[] {-1, 0, 1, 0};
	static constexpr int dy[] {0, -1, 0, 1};
	ll res=0;
	for(ll _=0;_<4;_++){
		ll cur=dfs(x+dx[_],y+dy[_]);
		if(!cur)continue;
		if(!res){res=cur;continue;}
		if(cur^res)return 2;
	}
}
rec game_board::calc(){
	DB resw=0,resb=0;cnt=0;
	for(ll i=1;i<=19;i++)
		for(ll j=1;j<=19;j++){
			if(board[i][j]==-1){resw++;continue;}
			if(board[i][j]==1){resb++;continue;}
			ll cur=dfs(i,j);
			if(cur==-1)resw+=cnt;
			else if(cur==1)resb+=cnt;
			else resw+=cnt/2.0,resb+=cnt/2.0;
		}
	return (rec){resw,resb};
}
