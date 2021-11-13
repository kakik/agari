#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <random>
#include <time.h>
#include <math.h>
#include <atlimage.h>
#include <iostream>

/********************************** 좌표 **********************************/
// 스프라이트
const RECT char_move_sprite_rect[8][4] = //스프라이트 좌표
{ { { 0,0,18,30 },{ 32,0,50,30 },{ 64,0,82,30 },{ 96,0,114,30 } }/*N*/,{ { 133,0,151,30 },{ 165,0,173,30 },{ 197,0,215,30 },{ 229,0,247,30 } }/*NE*/,
{ { 264,0,282,30 },{ 296,0,314,30 },{ 328,0,346,30 },{ 360,0,388,30 } }/*E*/,{ { 397,0,415,30 },{ 428,0,446,30 },{ 460,0,488,30 },{ 492,0,510,30 } }/*SE*/,
{ { 0,36,18,66 },{ 32,36,50,66 },{ 64,36,82,66 },{ 96,36,114,66 } }/*S*/,{ { 133,36,151,66 },{ 165,36,173,66 },{ 197,36,215,66 },{ 229,36,247,66 } }/*SW*/,
{ { 264,36,282,66 },{ 296,36,314,66 },{ 328,36,346,66 },{ 360,36,388,66 } }/*W*/,{ { 397,36,415,66 },{ 428,36,446,66 },{ 460,36,488,66 },{ 492,36,510,66 } }/*NW*/ };
const RECT char_atk_sprite_rect[8][2] =  //공격모션 스프라이트 좌표
{ {{0,0,30,32}, {30,0,60,32} }/*N*/, {{ 60,0,90,32 }, {90,0,120,32} }/*NE*/,{{120,0,150,32},{150,0,180,32}}/*E*/,{{180,0,210,32},{210,0,240,32}}/*SE*/,
{ { 0,32,30,64 },{ 30,32,60,64 } }/*S*/,{ { 60,32,90,64 },{ 90,32,120,64 } }/*SW*/,{ { 120,32,150,64 },{ 150,32,180,64 } }/*W*/,{ { 180,32,210,64 },{ 210,32,240,64 }/*NW*/ } };
const RECT char_weapon_sprite_rect[8][2] = //무기 스프라이트 좌표
{ {{0,0,50,52},{50,0,100,52}}/*N*/, {{100,0,150,52},{150,0,200,52}}/*NE*/ ,{ {200,0,250,52},{250,0,300,52}}/*E*/, {{300,0,350,52},{350,0,400,52}}/*SE*/,
{ { 0,52,50,104 },{ 50,52,100,104 } }/*S*/,{ { 100,52,150,104 },{ 150,52,200,104 } }/*SW*/ ,{ { 200,52,250,104 },{ 250,52,300,104 } }/*W*/,{ { 300,52,350,104 },{ 350,52,400,104 } }/*NW*/ };
const RECT monster_atk_sprite_rect[8][2] =  //공격모션 스프라이트 좌표
{ { { 0,0,34,37 },{ 34,0,68,37 } }/*N*/,{ { 68,0,102,37 },{ 102,0,136,37 } }/*NE*/,{ { 136,0,170,37 },{ 170,0,204,37 } }/*E*/,{ { 204,0,238,37 },{ 238,0,272,37 } }/*SE*/,
 { { 0,37,34,74 },{ 34,37,68,74 } }/*S*/,{ { 68,37,102,74 },{ 102,37,136,74 } }/*SW*/,{ { 136,37,170,74 },{ 170,37,204,74 } }/*W*/,{ { 204,37,238,74 },{ 238,37,272,74 } }/*NW*/ };

// UI 좌표
const RECT logo_rect = //로고 위치      (스타트화면)
{ 
	120,120,750,370 
};          
const RECT play_button_rect = //play버튼 위치  (스타트화면)
{ 
	650,480,800,550 
};   
const RECT exit_button_rect = //exit버튼 위치  (스타트화면)
{ 
	650,600,800,670 
};   
const RECT weapon_image_rect[6] =  //무기 6개 이미지 위치
{ 
	{ 220,730,270,780 },{ 300,730,350,780 },{ 380,730,430,780 },{ 460,730,510,780 },{ 540,730,590,780 },{ 620,730,670,780 } 
}; 
const RECT GAMEOVER_rect = //게임오버 버튼 위치  (게임엔드화면)
{ 
	220,50,620,150 
};       
const RECT rankingbox_rect = //랭킹박스 위치       (게임엔드화면)
{ 
	100,200,745,600 
};   
const RECT replay_button_rect = //리플레이 버튼 위치  (게임엔드화면)
{ 
	150,640,300,710 
}; 
const RECT exit2_button_rect = //exit버튼 위치       (게임엔드화면)    //exit버튼이 두 종류라는걸 깨닫았을땐 이미 늦었다 정답은 exit2
{ 
	530,640,680,710 
};  

/********************************** 변수 **********************************/
HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("agari!");


#define character_width 38
#define character_height 60

const int win_x_size = 900;      //윈도우 x사이즈
const int win_y_size = 800;      //윈도우 y사이즈



/********************************** 함수 **********************************/

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime);



/******************************************** 네트워크 게임 프로그래밍 ********************************************/
struct Coordinate {
	short x;
	short y;
};

class GameObject {
private:
	Coordinate pos;
	char direction;
	char sprite;
	unsigned char width, height;
	bool isActive;

public:
	void LoginOk(void* pk)
	{

	}

	void ObjMove(void* pk)
	{

	}

	void PutObj(void* pk)
	{

	}

	void RemoveObj(void* pk)
	{

	}

	void Render()
	{

	}
};

class Player : GameObject {
private:
	char curGun;
	char state;
	short hp;
	short items[8];

public:
	void PlayerState(void* pk)
	{

	}

	void ChangeWeapon(void* pk)
	{

	}

	void ChangeHp(void* pk)
	{

	}

	void GetItem(void* pk)
	{

	}

	void ItemCount(void* pk)
	{

	}

	void UseItem(int index)
	{

	}
};

/********************************** Main **********************************/
MSG Message;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;

	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = g_hInst;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, _T("agari!"), WS_OVERLAPPEDWINDOW, 0, 0, win_x_size + 14, win_y_size + 36, NULL, (HMENU)NULL, g_hInst, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	srand(GetTickCount());  //겟틱카운트! 랜덤시드~

	while (GetMessage(&Message, 0, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	srand((unsigned int)time(NULL));

	HDC hdc, memdc1, memdc2;
	PAINTSTRUCT ps;

	HBRUSH hbrush, oldbrush;
	HPEN hpen, oldpen;

	CImage img;
	CImage dc, dc2;

	HFONT hFont, oldFont;

	TCHAR str[500] = {};

	int play_size_left, play_size_top;

	static bool play_button = false;  //시작화면 start버튼
	static bool exit_button = false;  //시작화면 exit버튼

	static bool replay_button = false;  //엔드화면 replay버튼
	static bool exit2_button = false;  //엔드화면 exit버튼


	switch (uMsg) 
	{
	case WM_CREATE:

		break;
	case WM_PAINT: // 랜더링
		hdc = BeginPaint(hWnd, &ps);

		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN: // input key

		break;

	case WM_LBUTTONUP:
		break;

	case WM_MOUSEMOVE:


		break;
	case WM_KEYDOWN:
		break;

	case WM_KEYUP:
		break;
	case WM_DESTROY:

		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime)
{
	switch (idEvent)
	{

	}
}