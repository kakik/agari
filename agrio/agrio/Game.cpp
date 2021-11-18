#include "Game.h"

#define SERVERIP	"121.139.87.12"
#define SERVERPORT	9000
#define BUFSIZE		512

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
SOCKET sock;
void ConnectServer();
void Send(void* Packet);
void SendMovePacket(char dir);

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

	static int MOUSE_X = 0, MOUSE_Y = 0;

	switch (uMsg)
	{
	case WM_CREATE:
	{
		{
			/*********************************************이미지 로드*****************************************************/
			sprites[(int)SPRITE::btnPlay].Load(TEXT("resource/PLAY.png"));
			sprites[(int)SPRITE::btnExit].Load(TEXT("resource/EXIT.png"));
			sprites[(int)SPRITE::btnReplay].Load(TEXT("resource/REPLAY.png"));
			sprites[(int)SPRITE::bgTitle].Load(TEXT("resource/startBack(구).png"));
			sprites[(int)SPRITE::bgStage1].Load(TEXT("resource/stage.png"));
			sprites[(int)SPRITE::bgEnd].Load(TEXT("resource/endBack.png"));

			sprites[(int)SPRITE::Izuna].Load(TEXT("resource/Izuna_move.png"));
			sprites[(int)SPRITE::GenAn].Load(TEXT("resource/Gen-An_move.png"));
			sprites[(int)SPRITE::Hinagiku].Load(TEXT("resource/Hinagiku_move.png"));
			sprites[(int)SPRITE::Ichika].Load(TEXT("resource/Ichika_move.png"));
			sprites[(int)SPRITE::Kagen].Load(TEXT("resource/Kagen_move.png"));
			sprites[(int)SPRITE::Mitsumoto].Load(TEXT("resource/Mitsumoto_move.png"));
			sprites[(int)SPRITE::Shino].Load(TEXT("resource/Shino_move.png"));
			sprites[(int)SPRITE::Sizune].Load(TEXT("resource/Sizune_move.png"));

			sprites[(int)SPRITE::Izuna_Atk].Load(TEXT("resource/Izuna_attack.png"));
			sprites[(int)SPRITE::GenAn_Atk].Load(TEXT("resource/Gen-An_attack.png"));
			sprites[(int)SPRITE::Hinagiku_Atk].Load(TEXT("resource/Hinagiku_attack.png"));
			sprites[(int)SPRITE::Ichika_Atk].Load(TEXT("resource/Ichika_attack.png"));
			sprites[(int)SPRITE::Kagen_Atk].Load(TEXT("resource/Kagen_attack.png"));
			sprites[(int)SPRITE::Mitsumoto_Atk].Load(TEXT("resource/Mitsumoto_attack.png"));
			sprites[(int)SPRITE::Shino_Atk].Load(TEXT("resource/Shino_attack.png"));
			sprites[(int)SPRITE::Sizune_Atk].Load(TEXT("resource/Sizune_attack.png"));

			sprites[(int)SPRITE::pistol].Load(TEXT("resource/attack_pistol.png"));
			sprites[(int)SPRITE::uzi].Load(TEXT("resource/attack_uzi.png"));
			sprites[(int)SPRITE::shotgun].Load(TEXT("resource/attack_shotgun.png"));
			sprites[(int)SPRITE::box].Load(TEXT("resource/box.png"));


			sprites[(int)SPRITE::uiPistol].Load(TEXT("resource/ui_pistol.png"));
			sprites[(int)SPRITE::uiUzi].Load(TEXT("resource/ui_uzi.png"));
			sprites[(int)SPRITE::uiShotgun].Load(TEXT("resource/ui_shotgun.png"));
			sprites[(int)SPRITE::uiBox].Load(TEXT("resource/ui_box.png"));
			sprites[(int)SPRITE::uiPotion].Load(TEXT("resource/ui_box.png"));	// uiposition 이미지 만들어야함

			sprites[(int)SPRITE::itemBox].Load(TEXT("resource/itembox.png"));
		}
		
		/*********************************************이미지 로드*****************************************************/
		gameObject.reserve(1000);
		for (int i = 0; i < 4; ++i)
		{
			Player* player = new Player;
			gameObject.push_back(reinterpret_cast<GameObject*>(player));
		}
		for (int i = 0; i < 100; ++i)
		{
			GameObject* obj = new GameObject;
			gameObject.push_back(obj);
		}


		gameObject[5]->test();	////////////////////////// 임시


		/////////////////////////////////////////////////////////////////////////////////////////

		TIMER = GetTickCount64();
		SetTimer(hWnd, 0, 10, (TIMERPROC)TimerProc);	// updateLoop
	}
	break;

	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = win_x_size + 14;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = win_y_size + 36;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = win_x_size + 14;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = win_y_size + 36;
		return false;
	}
	break;

	case WM_PAINT: // 랜더링
	{
		hdc = BeginPaint(hWnd, &ps);

		if (scene == SCENE::title)
		{
			dc.Create(win_x_size, win_y_size, 24);	// 배경 영역 DC
			memdc1 = dc.GetDC();					// 배경 영역 DC

			sprites[(int)SPRITE::bgTitle].Draw(memdc1, 0, 0, win_x_size, win_y_size);
			sprites[(int)SPRITE::btnPlay].Draw(memdc1, play_button_rect);
			sprites[(int)SPRITE::btnExit].Draw(memdc1, exit_button_rect);

			// 선택용 스프라이트
			sprites[selPlayer].Draw(memdc1, 680, 100, character_width * 2, character_height * 2,
				char_move_sprite_rect[(int)DIR::S][selAnimation].left, char_move_sprite_rect[(int)DIR::S][selAnimation].top, 18, 30);

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);	// 아래에 Bitblt랑 동일
			dc.ReleaseDC();		// dc 해제
			dc.Destroy();		// 썼던 dc 삭제
		}

		else if (scene == SCENE::lobby)
		{
			dc.Create(win_x_size * 2, win_y_size * 2, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();							// == CreateComaptibleDC

			sprites[(int)SPRITE::bgStage1].BitBlt(memdc1, 0, 0, SRCCOPY); //배경

			////////////////////////////////////////// Render ///////////////////////////////////////////////

			for (const auto& gameobj : gameObject)
			{
				gameobj->Render(memdc1);
			}

			////////////////////////////////////////// 화면 영역 ///////////////////////////////////////////////
			// UI 오버레이들만 memdc2에 출력함

			Coordinate playerPos = gameObject[playerID]->GetPos();	// 플레이어 좌표

			dc2.Create(win_x_size, win_y_size, 24);		// 화면 출력용 DC
			memdc2 = dc2.GetDC();						// 화면 출력용 DC

			if (playerPos.x <= (win_x_size / 2))
			{
				play_size_left = 0;
			}
			else if (playerPos.x >= (win_x_size * 2) - (win_x_size / 2))
			{
				play_size_left = (win_x_size * 2) - win_x_size;
			}
			else
			{
				play_size_left = playerPos.x - (win_x_size / 2);
			}

			if (playerPos.y <= (win_y_size / 2))
			{
				play_size_top = 0;
			}
			else if (playerPos.y >= (win_y_size * 2) - (win_y_size / 2))
			{
				play_size_top = (win_y_size * 2) - win_y_size;
			}
			else
			{
				play_size_top = playerPos.y - (win_y_size / 2);
			}

			dc.BitBlt(memdc2, 0, 0, win_x_size, win_y_size, play_size_left, play_size_top);

			////////////////////////////////////////// UI ///////////////////////////////////////////////

			TCHAR bullet_num[5] = {};   //총알개수
			TCHAR weapon_num[2] = {};   //무기번호

			hbrush = CreateSolidBrush(RGB(255, 255, 255));
			oldbrush = (HBRUSH)SelectObject(memdc2, hbrush);
			for (int i = 0; i < 5; ++i)
			{
				RECT print_rect = weapon_image_rect[i];		// 개수 받아올수 있도록 수정해야함
				wsprintf(bullet_num, TEXT("%d"), 0);
				wsprintf(weapon_num, TEXT("%d"), i + 1);

				//if (selected_weapon == i)  //선택된 무기는 25픽셀 위에 출력
				//{
				//	print_rect.top -= 25;
				//	print_rect.bottom -= 25;
				//}
				RoundRect(memdc2, print_rect.left, print_rect.top, print_rect.right, print_rect.bottom, 10, 10);

				sprites[(int)SPRITE::uiPistol + i].Draw(memdc2, print_rect);

				SetBkMode(memdc2, TRANSPARENT);  //투명배경

				print_rect.top -= 20;
				print_rect.bottom -= 20;
				DrawText(memdc2, bullet_num, _tcslen(bullet_num), &print_rect, DT_CENTER | DT_VCENTER);

				print_rect.top += 70;
				print_rect.bottom += 70;
				DrawText(memdc2, weapon_num, _tcslen(weapon_num), &print_rect, DT_CENTER | DT_VCENTER);
			}
			SelectObject(memdc2, oldbrush);
			DeleteObject(hbrush);

			///////////////////////////////////////////////////////////////////////////////////////////

			dc2.Draw(hdc, 0, 0);

			dc.ReleaseDC();		// dc 해제
			dc.Destroy();		// 썼던 dc 삭제
			dc2.ReleaseDC();
			dc2.Destroy();
		}

		else if (scene == SCENE::stage1)
		{

		}

		else if (scene == SCENE::gameover)
		{

			dc.Create(win_x_size, win_y_size, 24);
			memdc1 = dc.GetDC();
			sprites[(int)SPRITE::bgEnd].Draw(memdc1, 0, 0, win_x_size, win_y_size);


			sprites[(int)SPRITE::btnReplay].Draw(memdc1, replay_button_rect);	//replay버튼
			sprites[(int)SPRITE::btnExit].Draw(memdc1, exit2_button_rect);		//exit버튼

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);

			dc.ReleaseDC();		// dc 해제
			dc.Destroy();		// 썼던 dc 삭제
		}
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_LBUTTONDOWN:
	{
		MOUSE_X = LOWORD(lParam);
		MOUSE_Y = HIWORD(lParam);

		if (scene == SCENE::title)
		{
			if ((play_button_rect.left <= MOUSE_X) && (play_button_rect.top <= MOUSE_Y) && (MOUSE_X <= play_button_rect.right) && (MOUSE_Y <= play_button_rect.bottom))  //play버튼 선택, 눌렀다 떼야 작동
			{
				play_button = true;
			}
			else if ((exit_button_rect.left <= MOUSE_X) && (exit_button_rect.top <= MOUSE_Y) && (MOUSE_X <= exit_button_rect.right) && (MOUSE_Y <= exit_button_rect.bottom)) //exit버튼 선택, 눌렀다 떼야 작동
			{
				exit_button = true;
			}
		}
		else if (scene == SCENE::gameover)
		{
			if ((replay_button_rect.left <= MOUSE_X) && (replay_button_rect.top <= MOUSE_Y) && (MOUSE_X <= replay_button_rect.right) && (MOUSE_Y <= replay_button_rect.bottom))  //replay버튼 선택, 눌렀다 떼야 작동
			{
				replay_button = true;
			}
			else if ((exit2_button_rect.left <= MOUSE_X) && (exit2_button_rect.top <= MOUSE_Y) && (MOUSE_X <= exit2_button_rect.right) && (MOUSE_Y <= exit2_button_rect.bottom)) //exit2버튼 선택, 눌렀다 떼야 작동
			{
				exit2_button = true;
			}
		}
	}
	break;

	case WM_LBUTTONUP:
	{
		if (scene == SCENE::title)
		{
			if (play_button == true)
			{
				ConnectServer();
				while (!isLoginOk);
				scene = SCENE::lobby;
				play_button = false;
				InvalidateRect(hWnd, NULL, false);
			}
			else if (exit_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //프로그램 종료
				break;
			}
		}
		else if (scene == SCENE::gameover)
		{
			if (replay_button == true)
			{
				scene = SCENE::title;
				replay_button = false;

				InvalidateRect(hWnd, NULL, false);
			}
			else if (exit2_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //프로그램 종료
				break;
			}
		}
	}
	break;

	case WM_MOUSEMOVE:
	{
	}
	break;

	case WM_KEYDOWN:
	{
		if (scene == SCENE::title)
		{
			switch (wParam)
			{
			case '1':
				selPlayer = (int)SPRITE::Izuna;
				break;

			case '2':
				selPlayer = (int)SPRITE::GenAn;
				break;

			case '3':
				selPlayer = (int)SPRITE::Hinagiku;
				break;

			case '4':
				selPlayer = (int)SPRITE::Ichika;
				break;

			case '5':
				selPlayer = (int)SPRITE::Kagen;
				break;

			case '6':
				selPlayer = (int)SPRITE::Mitsumoto;
				break;

			case '7':
				selPlayer = (int)SPRITE::Shino;
				break;

			case '8':
				selPlayer = (int)SPRITE::Sizune;
				break;
			}
		}

		// Lobby packet test
		if (scene == SCENE::lobby)
		{
			switch (wParam)
			{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
			case VK_LEFT:
				SendMovePacket((char)DIR::E);
				break;
			case VK_RIGHT:
				SendMovePacket((char)DIR::W);
				break;
			case VK_UP:
				SendMovePacket((char)DIR::N);
				break;
			case VK_DOWN:
				SendMovePacket((char)DIR::S);
				break;
			case VK_SPACE:
				cs_packet_shoot_bullet sendPacket;
				sendPacket.packetSize = sizeof(sendPacket);
				sendPacket.packetType = CS_PACKET_SHOOT_BULLET;
				Coordinate pos = gameObject[playerID]->GetPos();
				sendPacket.dir = (char)gameObject[playerID]->GetDir();
				Send(&sendPacket);
				break;
			}
		}
	}
	break;

	case WM_KEYUP:
	{
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
void ConnectServer()
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	//socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket");

	//connect
	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	ServerAddr.sin_port = htons(SERVERPORT);

	// 연결 실패하면 재연결하도록 바꾸자
	retval = connect(sock, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	HANDLE hThread;
	hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)sock, 0, NULL);

	if (hThread == NULL)closesocket(sock);

	cs_packet_login packet;
	packet.packetSize = sizeof(cs_packet_login);
	packet.packetType = CS_PACKET_LOGIN;
	packet.playerSkin = selPlayer;
	Send(&packet);
}

void SendMovePacket(char dir)
{
	cs_packet_player_move sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = CS_PACKET_PLAYER_MOVE;
	sendPacket.dir = dir;
	Send(&sendPacket);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime)
{
	// idEvent에 메세지가 담겨져서 옴 근데 안쓸 예정
	if (scene == SCENE::title)
	{
		selTimer -= GetTickCount64() - TIMER;
		if (selTimer < 0)
		{
			selAnimation++;
			selAnimation %= 4;
			selTimer = 100;
		}
	}

	TIMER = GetTickCount64();
	InvalidateRect(hWnd, NULL, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GameObject::GameObject()
{
	pos = { 0, 0 };
	sprite = 0;
	width = 0;
	height = 0;
	direction = DIR::N;
}

void GameObject::LoginOk(void* pk)
{
	sc_packet_login_ok* recvPacket = (sc_packet_login_ok*)pk;

	isActive = true;
	pos.x = recvPacket->x;
	pos.y = recvPacket->y;
	width = recvPacket->width;
	height = recvPacket->height;
	playerID = recvPacket->playerID;
	sprite = selPlayer;
}

void GameObject::ObjMove(void* pk)
{
	sc_packet_move_obj* recvPacket = (sc_packet_move_obj*)pk;

	direction = (DIR)recvPacket->lookDir;
	pos.x = (int)recvPacket->x;
	pos.y = (int)recvPacket->y;
}

void GameObject::PutObj(void* pk)
{
	sc_packet_put_obj* recvPacket = (sc_packet_put_obj*)pk;

	sprite = (int)recvPacket->sprite;
	pos.x = (int)recvPacket->x;
	pos.y = (int)recvPacket->y;
	width = (int)recvPacket->width;
	height = (int)recvPacket->height;
}

void GameObject::RemoveObj()
{
	isActive = false;
}

void GameObject::Render(HDC& hdc)
{
	if (isActive)
		sprites[sprite].Draw(hdc, pos.x - (width / 2), pos.y + (height / 2), width, height);

	// 번외, 총알은 어케 출력할 것 인가?
	// 1안 총알은 방향별로 저장해서 위에꺼 그대로 사용 => 이거로 할꺼
	// 2안 총알 이미지를 한번에 모두 저장해서 따로 방향에 맞춰서 처리 => 폐기
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

void Player::Render(HDC& hdc)
{
	if (isActive)
		sprites[sprite].Draw(hdc, pos.x - (width / 2), pos.y - (height / 2), width, height, 
			char_move_sprite_rect[(int)direction][animFrame].left, char_move_sprite_rect[(int)direction][animFrame].top, 18, 30);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Send(void* Packet)
{

	int retval = send(sock, reinterpret_cast<char*>(Packet), reinterpret_cast<packet*>(Packet)->packetSize, 0);
	std::cout << "[TCP 서버]" << retval << "바이트 보냈습니다\n";

}

void Recv(SOCKET sock) {

	packet pkSize;
	int retval = recv(sock, reinterpret_cast<char*>(&pkSize), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return;
	}

	switch (pkSize.packetType)
	{
	case SC_PACKET_LOGIN_OK:
	{
		sc_packet_login_ok recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		playerID = (int)recvPacket.playerID;
		gameObject[playerID]->LoginOk(&recvPacket);
		isLoginOk = true;
	}
	break;
	case SC_PACKET_PUT_OBJ:
	{
		sc_packet_put_obj recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		gameObject[(int)recvPacket.objectID]->PutObj(&recvPacket);
	}
	break;
	case SC_PACKET_MOVE_OBJ:
	{
		sc_packet_move_obj recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		gameObject[(int)recvPacket.objectID]->ObjMove(&recvPacket);
	}
	break;
	case SC_PACKET_REMOVE_OBJ:
	{
		sc_packet_remove_obj recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		gameObject[(int)recvPacket.objectID]->RemoveObj();
	}
	break;
	default:
		break;
	}
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	// 전역 Sock이 있는데 이거 쓰는거 맞나?
	//SOCKET sock = reinterpret_cast<SOCKET>(arg);
	//서버와 데이터 통신
	int len;

	while (1) {

		Recv(sock);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GameObject::test()
{
	isActive = true;
	width = 30;
	height = 40;
	sprite = (int)SPRITE::box;
	pos.x = 850;
	pos.y = 900;
}

void Player::test()
{

}