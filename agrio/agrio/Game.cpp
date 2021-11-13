#include "Game.h"

/********************************** Main **********************************/
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

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, 
		0, 0, win_x_size + 14, win_y_size + 36, NULL, (HMENU)NULL, g_hInst, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

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
		/*********************************************이미지 로드*****************************************************/
		sprites[(int)SPRITE::btnPlay].Load(TEXT("resource/PLAY.png"));
		sprites[(int)SPRITE::btnExit].Load(TEXT("resource/EXIT.png"));
		sprites[(int)SPRITE::btnReplay].Load(TEXT("resource/REPLAY.png"));
		sprites[(int)SPRITE::bgTitle].Load(TEXT("resource/startBack.png"));
		sprites[(int)SPRITE::bgStage1].Load(TEXT("resource/stage.png"));
		sprites[(int)SPRITE::bgEnd].Load(TEXT("resource/endBack.png"));

		sprites[(int)SPRITE::p0].Load(TEXT("resource/Izuna_move.png"));
		sprites[(int)SPRITE::p1].Load(TEXT("resource/Gen-An_move.png"));
		sprites[(int)SPRITE::p2].Load(TEXT("resource/Hinagiku_move.png"));
		sprites[(int)SPRITE::p3].Load(TEXT("resource/Ichika_move.png"));
		sprites[(int)SPRITE::p4].Load(TEXT("resource/Kagen_move.png"));
		sprites[(int)SPRITE::p5].Load(TEXT("resource/Mitsumoto_move.png"));
		sprites[(int)SPRITE::p6].Load(TEXT("resource/Shino_move.png"));
		sprites[(int)SPRITE::p7].Load(TEXT("resource/Sizune_move.png"));

		sprites[(int)SPRITE::p0Atk].Load(TEXT("resource/Izuna_attack.png"));
		sprites[(int)SPRITE::p1Atk].Load(TEXT("resource/Gen-An_attack.png"));
		sprites[(int)SPRITE::p2Atk].Load(TEXT("resource/Hinagiku_attack.png"));
		sprites[(int)SPRITE::p3Atk].Load(TEXT("resource/Ichika_attack.png"));
		sprites[(int)SPRITE::p4Atk].Load(TEXT("resource/Kagen_attack.png"));
		sprites[(int)SPRITE::p5Atk].Load(TEXT("resource/Mitsumoto_attack.png"));
		sprites[(int)SPRITE::p6Atk].Load(TEXT("resource/Shino_attack.png"));
		sprites[(int)SPRITE::p7Atk].Load(TEXT("resource/Sizune_attack.png"));

		sprites[(int)SPRITE::pistol].Load(TEXT("resource/attack_pistol.png"));
		sprites[(int)SPRITE::uzi].Load(TEXT("resource/attack_uzi.png"));
		sprites[(int)SPRITE::shotgun].Load(TEXT("resource/attack_shotgun.png"));

		sprites[(int)SPRITE::uiPistol].Load(TEXT("resource/ui_pistol.png"));
		sprites[(int)SPRITE::uiUzi].Load(TEXT("resource/ui_uzi.png"));
		sprites[(int)SPRITE::uiShotgun].Load(TEXT("resource/ui_shotgun.png"));
		sprites[(int)SPRITE::uiBox].Load(TEXT("resource/ui_box.png"));
		sprites[(int)SPRITE::uiPotion].Load(TEXT("resource/ui_box.png"));	// uiposition 이미지 만들어야함

		sprites[(int)SPRITE::itemBox].Load(TEXT("resource/itembox.png"));
		/*********************************************이미지 로드*****************************************************/

		SetTimer(hWnd, 0, 10, (TIMERPROC)TimerProc);	// updateLoop
		break;
	
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = win_x_size + 14;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = win_y_size + 36;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = win_x_size + 14;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = win_y_size + 36;
		return false;
		break;

	case WM_PAINT: // 랜더링
		hdc = BeginPaint(hWnd, &ps);

		if (scene == SCENE::title)
		{
			dc.Create(win_x_size, win_y_size, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();					// == CreateComaptibleDC

			sprites[(int)SPRITE::bgTitle].Draw(memdc1, 0, 0, win_x_size, win_y_size);
			sprites[(int)SPRITE::btnPlay].Draw(memdc1, play_button_rect);
			sprites[(int)SPRITE::btnExit].Draw(memdc1, exit_button_rect);

			/*charac_sprite[current_char_num].Draw(memdc1, 680, 100, character_width * 2, character_height * 2,
				char_move_sprite_rect[player.direction][player.sprite_num].left, char_move_sprite_rect[player.direction][player.sprite_num].top, 18, 30);*/

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);	// 아래에 Bitblt랑 동일
			dc.ReleaseDC();		// dc 해제
			dc.Destroy();		// 썼던 dc 삭제
		}

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
	// idEvent에 메세지가 담겨져서 옴 근데 안쓸 예정
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GameObject::LoginOk(void* pk)
{

}

void GameObject::ObjMove(void* pk)
{

}

void GameObject::PutObj(void* pk)
{

}

void GameObject::RemoveObj(void* pk)
{

}

void GameObject::Render()
{

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::PlayerState(void* pk)
{

}

void Player::ChangeWeapon(void* pk)
{

}

void Player::ChangeHp(void* pk)
{
	
}

void Player::GetItem(void* pk)
{
	
}

void Player::ItemCount(void* pk)
{
	
}

void Player::UseItem(int index)
{
	
}

void Player::Render()
{

}