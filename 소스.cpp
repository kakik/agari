#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <random>
#include <time.h>
#include <atlimage.h>


HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("agari!");

enum direction  //방향 N=0,NE=1,E=2,SE=3,S=4,SW=5,W=6,NW=7
{
	N = 0, NE = 1, E = 2, SE = 3, S = 4, SW = 5, W = 6, NW = 7
};
enum page       //페이지
{
	start_page, game_page, end_page
};
enum weapom     //무기번호 pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5
{
	pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5
};
enum timer      //타이머 넘버에 숫자 대신 이걸 써줍시다 
{
	rest_time,
	move_player,
	spawn_monster, move_monster,
	spawn_itembox,
	pistol_delay, uzi_delay, shotgun_delay, rocket_delay
};

typedef struct CHARACTER   //플레이어, 몬스터, 보스몬스터 정보
{
	int health;            //현재 캐릭터 체력  
	int max_health;        //캐릭터 최대 체력
	int x;                 //현재 캐릭터 x좌표 
	int y;                 //현재 캐릭터 y좌표 
	int direction;         //현재 캐릭터가 바라보는 방향 enum direction으로 사용

	int sprite_num;
	bool ismoving;         //이동중인지 (타이머 돌아가고있는지)
	bool isattacking;      //공격중인지 (타이머 돌아가고있는지)
	 
	CHARACTER* next;       //몬스터, 보스몬스터용 next좌표
}CHARACTER;


typedef struct WEAPON       //무기 정보
{
	bool open;              //무기 습득 여부
	int bullet;             //무기별 남은 탄약
	int max_bullet;         //무기별 최대 탄약, 아이템 박스로 확률적으로 최대탄약 확장 가능
	int damage;             //무기별 데미지        
	int range;              //무기별 사정거리
	int delay;              //무기 사용 딜레이 
	bool isdelay;           //현재 딜레이 여부

	CImage img;              //무기 이미지
}WEAPON;





typedef struct BLOCK         //50*50크기 블록
{
	bool iswall;              //벽이 있닝?
	bool isinvinciblewall;    //파괴 불가능한 벽이 있닝?
	bool isbarrel;            //베럴이 있닝?
	bool isitembox;           //아이템박스가 있닝?

	bool isempty;             //비어있니? == 위의 4 오브젝트중 아무것도 없어야만 true!

	RECT rect;                //블록 좌표 
}BLOCK;

/************************************************함수 선언********************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime);
void Game_start_setting(HWND hWnd);  //게임 시작 셋팅(변수 초기화 등등)
void Aquire_itembox();      //아이템 박스를 먹어부려쪄!
void Spawn_itembox();
void Char_Deathcheck(HWND hWnd);
void Stage_start(HWND hWnd);
void Reset_weapon_setting();
void Reset_weapon_upgrade();
void Spawn_monster();
void Spawn_boss();
/*********************************************void()함수 최고*****************************************************/

/*********************************************사랑합니다 전역변수*****************************************************/

int win_x_size = 900;      //윈도우 x사이즈
int win_y_size = 800;      //윈도우 y사이즈
int block_size = 50;       //블록 크기

BLOCK block[32][36];       //36*32 블록 

int mx;                    //마우스 x위치값	
int my;                    //마우스 y위치값

int current_page;          //현재 페이지  enum page로 사용
int stage;                 //현재 스테이지
int score;                 //현재 스코어


CHARACTER player;          //플레이어
WEAPON weapon[6];          //무기 6개 enum weapon으로 사용
int selected_weapon;       //현재 선택중인 무기 enum weapon으로 사용

/******************무기 업그레이드 bool변수*******************/
//무기번호 pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5
bool pistol_range_up1;
bool pistol_delay_down1;
//pistol 총알 무한

bool uzi_range_up1;
bool uzi_delay_down1;
bool uzi_maxammo_up1;
bool uzi_maxammo_up2;

bool shotgun_range_up1;
bool shotgun_delay_down1;
bool shotgun_maxammo_up1;
bool shotgun_maxammo_up2;

bool barrel_maxammo_up1;
bool barrel_maxammo_up2;

bool wall_maxammo_up1;
bool wall_maxammo_up2;

//rocket 사정거리 무한
bool rocket_delay_down1;
bool rocket_maxammo_up1;
bool rocket_maxammo_up2;
/*************************************************************/

int itembox_num;           //맵에 존재하는 아이템 박스 개수
int aquired_itembox_num;   //현재까지 습득한 아이템 박스 개수

bool isrest_time;           


CHARACTER* monster_head;  //head노드는 삭제하지 않고 사용하기
CHARACTER* boss_head;     //head노드는 삭제하지 않고 사용하기

int monster_num;          //현재 몬스터 숫자
int boss_num;             //현재 보스몬스터 숫자



RECT logo_rect = { 120,120,750,370 };          //로고 위치      (스타트화면)
RECT play_button_rect = { 600,480,750,550 };   //play버튼 위치  (스타트화면)
RECT exit_button_rect = { 600,600,750,670 };   //exit버튼 위치  (스타트화면)
											   
RECT weapon_image_rect[6] = { { 220,680,270,730 },{ 300,680,350,730 },{ 380,660,430,710 },{ 460,680,510,730 },{ 540,680,590,730 },{ 620,680,670,730 }};  //무기 6개 이미지 위치

RECT GAMEOVER_rect = { 220,50,620,150 };       //게임오버 버튼 위치  (게임엔드화면)
RECT rankingbox_rect = { 100,200,745,600 };    //랭킹박스 위치       (게임엔드화면)
RECT replay_button_rect = { 150,640,300,710 }; //리플레이 버튼 위치  (게임엔드화면)
RECT exit2_button_rect = { 530,640,680,710 };  //exit버튼 위치       (게임엔드화면)    //exit버튼이 두 종류라는걸 깨닫았을땐 이미 늦었다 정답은 exit2

/*********************************************사랑합니다 에드가니이이이이이임*****************************************/


/*********************************************비트맵 이미지*****************************************************/


CImage start_page_bk_img;   //시작화면 배경
CImage game_page_bk_img;    //게임화면 배경
CImage end_page_bk_img;     //엔드화면 배경

CImage logo_img;            //로고
CImage play_button_img;     //play버튼
CImage exit_button_img;     //exit버튼


						/*         무기 이미지는 WEAPON 구조체 안에 있음!       */

CImage char_move_sprite[10][8][4]; //캐릭터 이동 스프라이트 [캐릭터 번호][방향][이동 모션 4개]
CImage char_atk_sprite[10][8][2];  //캐릭터 공격 스프라이트 [캐릭터 번호][방향][공격 모션 2개]
int current_char_num;              //선택한 캐릭터 번호  char_move_sprite[current_char_num][direction][n]같이 사용

CImage monster_move_sprite[8][4]; //몬스터 이동 스프라이트 [방향][이동 모션 4개(?)]
CImage monster_atk_sprite[8][2];  //보스 공격 스프라이트[방향][공격 모션 2개(?)]

/*********************************************비트맵 이미지*****************************************************/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, _T("agari!"), WS_OVERLAPPEDWINDOW, 0, 0, win_x_size + 25, win_y_size + 25, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	srand(GetTickCount());  //겟틱카운트! 랜덤시드~

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	HDC hdc, memdc1, memdc2;
	PAINTSTRUCT ps;

	CImage img;
	CImage dc;

	TCHAR str[500] = {};


	static bool play_button = false;  //시작화면 start버튼
	static bool exit_button = false;  //시작화면 exit버튼

	static bool replay_button = false;  //엔드화면 replay버튼
	static bool exit2_button = false;  //엔드화면 exit버튼

	switch (uMsg) {
	case WM_CREATE:
		/*********************************************이미지 로드*****************************************************/
		
		play_button_img.Load(TEXT("..\\agari\\resource\\PLAY.png"));
		exit_button_img.Load(TEXT("..\\agari\\resource\\EXIT.png"));
		start_page_bk_img.Load(TEXT("..\\agari\\resource\\startBack.png"));

		/*********************************************이미지 로드*****************************************************/

		current_page = start_page;  //현재 페이지를 시작페이지로

		for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 36; j++)
			{
				block[i][j].rect = { 50 * j,50 * i,50 * j + 50,50 * i + 50 };  //36*32 블록 좌표 입력
			}
		}


		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		if (current_page == start_page)
		{
			dc.Create(win_x_size, win_y_size, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();					// == CreateComaptibleDC

			start_page_bk_img.Draw(memdc1, 0,0,win_x_size,win_y_size);
			play_button_img.Draw(memdc1, play_button_rect);
			exit_button_img.Draw(memdc1, exit_button_rect);

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);	// 아래에 Bitblt랑 동일
			dc.ReleaseDC();		// dc 해제
			dc.Destroy();		// 썼던 dc 삭제

			// 아래랑 같은 작업임 (이해되면 삭제해)
			//BitBlt(hdc, 0, 0, win_x_size, win_y_size, memdc1, 0, 0, SRCCOPY);
			//DeleteObject(membitmap1);
			//DeleteObject(memdc1);
		}
		else if (current_page == game_page)
		{
			dc.Create(win_x_size, win_y_size, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();					// == CreateComaptibleDC
			//배경 출력

			//아이템박스 출력

			//캐릭터, 몬스터 출력

			//체력바 출력

			//오브젝트 출력(벽, 베럴)

			//총알 출력

			//ui 출력

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);	// 아래에 Bitblt랑 동일
			dc.ReleaseDC();		// dc 해제
			dc.Destroy();		// 썼던 dc 삭제
		}
		else if (current_page == end_page)
		{
			//스코어

			//스테이지

			//replay버튼

			//exit버튼
		}

		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:
	{
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		if (current_page == start_page)
		{
			if ((play_button_rect.left <= mx) && (play_button_rect.top <= my) && (mx <= play_button_rect.right) && (my <= play_button_rect.bottom))  //play버튼 선택, 눌렀다 떼야 작동
			{
				play_button = true;
				{
					/***********************************

					버튼 클릭시 이펙트 넣으면 귀찮을 것 같지만 일단..흠

					************************************/
				}
			}
			else if ((exit_button_rect.left <= mx) && (exit_button_rect.top <= my) && (mx <= exit_button_rect.right) && (my <= exit_button_rect.bottom)) //exit버튼 선택, 눌렀다 떼야 작동
			{
				exit_button = true;
				{
					/***********************************

					버튼 클릭시 이펙트 넣으면 귀찮을 것 같지만 일단..흠

					************************************/
				}
			}
		}
		else if (current_page == game_page)
		{



		}
		else if (current_page == end_page)
		{
			if ((replay_button_rect.left <= mx) && (replay_button_rect.top <= my) && (mx <= replay_button_rect.right) && (my <= replay_button_rect.bottom))  //replay버튼 선택, 눌렀다 떼야 작동
			{
				replay_button = true;
				{
					/***********************************

					버튼 클릭시 이펙트 넣으면 귀찮을 것 같지만 일단..흠

					************************************/
				}
			}
			else if ((exit2_button_rect.left <= mx) && (exit2_button_rect.top <= my) && (mx <= exit2_button_rect.right) && (my <= exit2_button_rect.bottom)) //exit2버튼 선택, 눌렀다 떼야 작동
			{
				exit2_button = true;
				{
					/***********************************

					버튼 클릭시 이펙트 넣으면 귀찮을 것 같지만 일단..흠

					************************************/
				}
			}
		}
		break;
	}
	case WM_LBUTTONUP:
	{
		if (current_page == start_page)
		{
			if (play_button == true)
			{
				current_page = game_page;   //게임화면으로 이동
				play_button = false;

				SetTimer(hWnd, spawn_itembox, 15000, TimerProc);//15초마다 아이템박스 생성
				Game_start_setting(hWnd);                       //게임 시작 셋팅(변수 초기화 등등)

				InvalidateRect(hWnd, NULL, false);
			}
			else if (exit_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //프로그램 종료
				break;
			}
		}
		else if (current_page == game_page)
		{



		}
		else if (current_page == end_page)
		{
			if (replay_button == true)
			{
				current_page = game_page;   //게임화면으로 이동
				play_button = false;

				SetTimer(hWnd, spawn_itembox, 15000, TimerProc);//15초마다 아이템박스 생성
				Game_start_setting(hWnd);                       //게임 시작 셋팅(변수 초기화 등등)
			

				InvalidateRect(hWnd, NULL, false);
			}
			else if (exit2_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //프로그램 종료
				break;
			}
		}
		break;
	}
	case WM_MOUSEMOVE:

		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		break;
	case WM_KEYDOWN:
	{
		if (current_page == game_page)
		{
			switch (wParam)
			{
			case 'w':
			{
				if ((player.direction == N) || (player.direction == S))  //이동 방향을 북으로
					player.direction = N;
				else if (player.direction == W)  //이동 방향을 북서로
					player.direction = NW;
				else if (player.direction == E)  //이동 방향을 북동으로
					player.direction = NE;

				if (player.ismoving == false)    //멈춰있었다면
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'a':
			{
				if ((player.direction == E) || (player.direction == W))  //이동 방향을 서로
					player.direction = W;
				else if (player.direction == N)  //이동 방향을 북서로
					player.direction = NW;
				else if (player.direction == S)  //이동 방향을 남서로
					player.direction = SW;

				if (player.ismoving == false)    //멈춰있었다면
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 's':
			{
				if ((player.direction == N) || (player.direction == S))  //이동 방향을 남으로
					player.direction = S;
				else if (player.direction == W)  //이동 방향을 남서로
					player.direction = SW;
				else if (player.direction == E)  //이동 방향을 남동으로
					player.direction = SE;

				if (player.ismoving == false)    //멈춰있었다면
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'd':
			{
				if ((player.direction == E) || (player.direction == W))  //이동 방향을 동으로
					player.direction = E;
				else if (player.direction == N)  //이동 방향을 북동으로
					player.direction = NE;
				else if (player.direction == S)  //이동 방향을 남동으로
					player.direction = SE;

				if (player.ismoving == false)    //멈춰있었다면
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}

			case 'q':  //이전 무기 선택
			{
				if (selected_weapon == pistol)                    //pistol이 제일 처음 무기
					break;
				else if (weapon[selected_weapon - 1].open == true)     //이전 무기를 가지고있다면
				{
					selected_weapon--;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case 'e':  //다음 무기 선택
			{
				if (selected_weapon == rocket)                     //rocket이 제일 마지막 무기
					break;
				else if (weapon[selected_weapon + 1].open == true) //다음 무기를 가지고있다면
				{
					selected_weapon++;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case '1':  //pistol 선택
			{
				if (weapon[pistol].open == true)
				{
					selected_weapon = pistol;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case '2':  //uzi 선택
			{
				if (weapon[uzi].open == true)
				{
					selected_weapon = uzi;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '3':  //shotgun 선택
			{
				if (weapon[shotgun].open == true)
				{
					selected_weapon = shotgun;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '4':  //barrel 선택
			{
				if (weapon[barrel].open == true)
				{
					selected_weapon = barrel;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '5':  //wall 선택
			{
				if (weapon[wall].open == true)
				{
					selected_weapon = wall;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '6':  //rocket 선택
			{
				if (weapon[rocket].open == true)
				{
					selected_weapon = rocket;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case VK_SPACE:  //무기 사용
			{
				if (weapon[selected_weapon].bullet == 0)      //총알 없는 총은 총이 아니제
				{
					//메세지를 띄운다던가... 총알이 없음을 알리는 무언가 
					//귀찮다면 그냥 따로 뭐 없이 break로 탈출하자구?

				}
				else
				{
					if (weapon[selected_weapon].isdelay == true)  //무기 사격 딜레이시 발사 불가!
						break;


					/*   주의! pistol은 총알이 무한이므로 pistol 발사시 총알 줄어들면 안댐!            */
					/*   주의! 총마다 발사 딜레이가 있음! 발사후 타이머로 딜레이 생성하는거 잊지말기!  */  // wall과 barrel은 딜레이가 없다! 

					if (selected_weapon == pistol)
					{
						//무기 발사
						//딜레이 생성
					}
					else if (selected_weapon == uzi)
					{
						//무기 발사
						//딜레이 생성
					}
					else if (selected_weapon == shotgun)
					{
						//무기 발사
						//딜레이 생성
					}
					/*완료*/else if (selected_weapon == barrel)
					{
						/*               블록 충돌체크                */
						int temp_x;
						int temp_y;

						for (int i = 0; i < 31; i++)
						{
							if ((block[i][0].rect.top <= player.y) && (player.y <= block[i + 1][0].rect.bottom))//y범위 찾기
							{
								temp_y = i;
								break;
							}
						}
						for (int i = 0; i < 36; i++)
						{
							if ((block[0][i].rect.left <= player.x) && (player.x <= block[0][i + 1].rect.right))//x범위 찾기
							{
								temp_x = i;
								break;
							}
						}
						if (block[temp_y][temp_x].isempty == false)  //비어있지 않다면 break;
							break;
						else
						{
							block[temp_y][temp_x].isbarrel = true;
							block[temp_y][temp_x].isempty = false;
						}
					
						InvalidateRect(hWnd, NULL, false);
					}
					/*완료*/else if (selected_weapon == wall)
					{
						/*               블록 충돌체크                */
						int temp_x;
						int temp_y;

						for (int i = 0; i < 31; i++)
						{
							if ((block[i][0].rect.top <= player.y) && (player.y <= block[i + 1][0].rect.bottom))//y범위 찾기
							{
								temp_y = i;
								break;
							}
						}
						for (int i = 0; i < 36; i++)
						{
							if ((block[0][i].rect.left <= player.x) && (player.x <= block[0][i + 1].rect.right))//x범위 찾기
							{
								temp_x = i;
								break;
							}
						}
						if (block[temp_y][temp_x].isempty == false)  //비어있지 않다면 break;
							break;
						else
						{
							block[temp_y][temp_x].iswall = true;
							block[temp_y][temp_x].isempty = false;
						}

						InvalidateRect(hWnd, NULL, false);
					}
					else if (selected_weapon == rocket)
					{
						//무기 발사
						//딜레이 생성
					}
					
				}
				break;
			}

			break;
			}
		}
		else if (current_page == end_page)
		{
		
		}
		break;
	}
	case WM_KEYUP:
	{
		if (current_page == game_page)
		{
			switch (wParam)
			{
			case 'w':
			{
				if (player.direction == NE)       //이동 방향을 동으로
					player.direction = E;
				else if (player.direction == NW)  //이동 방향을 서로
					player.direction = W;
				else if (player.direction == N)   //이동을 멈춤
				{
					player.ismoving = false;
					//공격중이 아닐경우 스프라이트를 기본으로 
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'a':
			{
				if (player.direction == NW)       //이동 방향을 북으로
					player.direction = N;
				else if (player.direction == SW)  //이동 방향을 남으로
					player.direction = S;
				else if (player.direction == W)   //이동을 멈춤
				{
					player.ismoving = false;
					//공격중이 아닐경우 스프라이트를 기본으로 
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 's':
			{
				if (player.direction == SE)       //이동 방향을 동으로
					player.direction = E;
				else if (player.direction == SW)  //이동 방향을 서로
					player.direction = W;
				else if (player.direction == S)   //이동을 멈춤
				{
					player.ismoving = false;
					//공격중이 아닐경우 스프라이트를 기본으로 
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'd':
			{
				if (player.direction == NE)       //이동 방향을 북으로
					player.direction = N;
				else if (player.direction == SE)  //이동 방향을 남으로
					player.direction = S;
				else if (player.direction == E)   //이동을 멈춤
				{
					player.ismoving = false;
					//공격중이 아닐경우 스프라이트를 기본으로 
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			break;
			}

		}
		else if (current_page == end_page)
		{


		}

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime)
{
	switch (idEvent)
	{
	case rest_time:  //한 스테이지 종료 후 다음 스테이지까지 텀
	{
		if (isrest_time == false)  //스테이지 종료했을 때
		{
			
			isrest_time = true;
		}
		else    //중간 휴식시간 끝났을 때
		{
			//스테이지 시작
			Stage_start(hWnd);

			isrest_time = false;
			KillTimer(hWnd, rest_time);  
		}
		break;
	}
	case spawn_itembox:
	{
		Spawn_itembox();
		break;
	}

	break;
	}



}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Game_start_setting(HWND hWnd)         //게임 시작 셋팅(변수 초기화 등등)
{
	stage = 0;                    //스테이지 초기화 (스테이지 시작 함수에서 1씩 증가시키므로 0으로 초기화)
	score = 0;                    //스코어 초기화

	player.max_health = 1000;     //캐릭터 최대체력 1000으로 설정
	player.health = player.max_health;
	player.x = 900;  //위치 중앙
	player.y = 800;
	player.direction = S;  //아래쪽 바라봄
	player.sprite_num = 0; //기본 이미지
	player.ismoving = false;
	player.isattacking = false;

	monster_num = 0;
	boss_num = 0;

	for (int i = 0; i < 32; i++)  //구조물 초기화
	{
		for (int j = 0; j < 36; j++)
		{
			block[i][j].iswall = false;
			block[i][j].isinvinciblewall = false;       //파괴 불가능한 벽 (지형) 설정하려면 여기서! 랜덤생성도 괜찮을듯?
			block[i][j].isbarrel = false;
			block[i][j].isitembox = false;
			block[i][j].isempty = true;
		}
	}
	itembox_num = 0;
	aquired_itembox_num = 0;      //습득한 아이템 박스 개수 초기화
	for (int i = 0; i < 5; i++)   //아이템박스 5개 생성 
		Spawn_itembox();

	Reset_weapon_setting();

	isrest_time = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aquire_itembox()             //아이템 박스를 먹어부려쪄!
{
	int opened_weapon_num;        //추가 습득한 무기 개수 (pistol 제외)

	for (int i = 5; i > -1; i--)
	{
		if (weapon[i].open == true)
		{
			opened_weapon_num = i;
			break;
		}
	}

	/*             특수 조건, 밸런싱? ㄴㄴ 기획 필요              */
	/*                무기 업그레이드 해금 조건임                 */
	//if ((aquired_itembox_num == ) || ())
	//{

	//}
	//else if ((aquired_itembox_num == ) || ())
	//{

	//}

	/*             업그레이드 불가능시               */

	int rand_num = rand() % (opened_weapon_num * 2 + 1);   //난이도 조절시 꽝 확률 추가하면 됨

	if (rand_num == 0)  //   1/((획득한 무기 개수*2)+1)의 확률로 체력회복
	{
		player.health = player.max_health;
	}
	else if ((rand_num == 1) || (rand_num == 2))
	{
		/*---------------------------------총알 획득시 최대까지 차는 것이 아닌 일정 수치만 증가 (밸런싱 필요)--------------------------*/

		/*if ((weapon[uzi].bullet +       ) > weapon[uzi].max_bullet)  //최대탄약 개수 이상으로 획득 불가
		weapon[uzi].bullet = weapon[uzi].max_bullet;
		weapon[uzi].bullet +=       ;*/
	}
	else if ((rand_num == 3) || (rand_num == 4))
	{
		/*---------------------------------총알 획득시 최대까지 차는 것이 아닌 일정 수치만 증가 (밸런싱 필요)--------------------------*/

		/*if ((weapon[shotgun].bullet +       ) > weapon[shotgun].max_bullet)  //최대탄약 개수 이상으로 획득 불가
		weapon[shotgun].bullet = weapon[shotgun].max_bullet;
		weapon[shotgun].bullet +=       ;*/
	}
	else if ((rand_num == 5) || (rand_num == 6))
	{
		/*---------------------------------총알 획득시 최대까지 차는 것이 아닌 일정 수치만 증가 (밸런싱 필요)--------------------------*/

		/*if ((weapon[barrel].bullet +       ) > weapon[barrel].max_bullet)  //최대탄약 개수 이상으로 획득 불가
		weapon[barrel].bullet = weapon[barrel].max_bullet;
		weapon[barrel].bullet +=       ;*/
	}
	else if ((rand_num == 7) || (rand_num == 8))
	{
		/*---------------------------------총알 획득시 최대까지 차는 것이 아닌 일정 수치만 증가 (밸런싱 필요)--------------------------*/

		/*if ((weapon[wall].bullet +       ) > weapon[wall].max_bullet)  //최대탄약 개수 이상으로 획득 불가
		weapon[wall].bullet = weapon[wall].max_bullet;
		weapon[wall].bullet +=       ;*/
	}
	else if ((rand_num == 9) || (rand_num == 10))
	{
		/*---------------------------------총알 획득시 최대까지 차는 것이 아닌 일정 수치만 증가 (밸런싱 필요)--------------------------*/

		/*if ((weapon[rocket].bullet +       ) > weapon[rocket].max_bullet)  //최대탄약 개수 이상으로 획득 불가
		weapon[rocket].bullet = weapon[rocket].max_bullet;
		weapon[rocket].bullet +=       ;*/
	}


}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Spawn_itembox()     //18*16 블록에서 비어있는 블록에서 랜덤으로 아이템 박스 생성	
{
	if (itembox_num < 6) //아이템박스는 5개까지만! (밸런싱 필요)
	{
		int temp_x;
		int temp_y;

		while (true)     //비어있는 칸을 찾아봅시다!
		{
			temp_x = rand() % 36;    //0 ~ 가로 블록 개수(36)-1 에서 랜덤
			temp_y = rand() % 32;    //0 ~ 가로 블록 개수(32)-1 에서 랜덤

			if (block[temp_y][temp_x].isempty == true)
				break;
		}

		block[temp_y][temp_x].isitembox = true;
		block[temp_y][temp_x].isempty = false;

		itembox_num++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Char_Deathcheck(HWND hWnd)    //캐릭터 체력 0됐는지 확인 함수 무기발사, 몬스터 공격시에 한 줄만 적어주면 된다구?? 몬스터 사망 체크 함수는 따로 만들거라구, 칭구!
{
	if (player.health == 0)
	{
		current_page = end_page;

		//타이머 녀석들.. 다...다 죽여버리겠어!!! 으앙아아아아ㅏㅏ아아ㅏㅏ아!!!
		//KillTimer(hWnd,);
		//KillTimer(hWnd.);
		//KillTimer(hWnd.);
		//KillTimer(hWnd,);
		KillTimer(hWnd,spawn_itembox);
	
		InvalidateRect(hWnd, NULL, false);
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Stage_start(HWND hWnd)   //스테이지 시작 세팅(몬스터 생성) 흥 함수 작성 순서같은거 생각나는대로 적는거야 데헷페로
{
	stage++;
	SetTimer(hWnd, rest_time, 5000, TimerProc);

	Spawn_monster();
	Spawn_boss();

	//몹 맵에서 나오도록 하는 타이머 생성 - 몹 스폰장소에서 한두마리씩 나오고, 입구가 막혀서 나오지 못할경우 나오지 않도록 해야함
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Spawn_monster()
{
	/* 스폰 몹 숫자 밸런싱 필요 */
	CHARACTER* p = monster_head;
	int i = 0;

	while (i < 50/*숫자 밸런스 조정*/)
	{
		//head노드는 삭제하지 않고 사용하기
		CHARACTER* temp_character = (CHARACTER*)malloc(sizeof(CHARACTER));

		temp_character->health = 100;
		temp_character->max_health = 100;
		//temp_character->x=
		//temp_character->y=
		//temp_character->direction=
		temp_character->sprite_num = 0;
		temp_character->ismoving = false;
		temp_character->isattacking = false;
		temp_character->next = NULL;

		p->next = temp_character;

		monster_num++;
		i++;
	}

	//몹 생성 - 연결리스트
	//head노드는 삭제하지 않고 사용하기
	//몹 좌표 지정 - 난이도 : HELL
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Spawn_boss()
{
	/* 스폰 몹 숫자 밸런싱 필요 */
	CHARACTER* p = boss_head;
	int i = 0;

	while (i < 50/*숫자 밸런스 조정*/)
	{
		//head노드는 삭제하지 않고 사용하기
		CHARACTER* temp_character = (CHARACTER*)malloc(sizeof(CHARACTER));

		temp_character->health = 500;
		temp_character->max_health = 500;
		//temp_character->x=
		//temp_character->y=
		//temp_character->direction=
		temp_character->sprite_num = 0;
		temp_character->ismoving = false;
		temp_character->isattacking = false;
		temp_character->next = NULL;

		p->next = temp_character;

		monster_num++;
		i++;
	}

	//몹 생성 - 연결리스트
	//head노드는 삭제하지 않고 사용하기
	//몹 좌표 지정 - 난이도 : HELL
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Reset_weapon_setting()
{
	/*              무기별 설정, 밸런싱 필요                            */
	Reset_weapon_upgrade();
	/*pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5*/

	/*             무기별 최대 탄약 수 설정, 밸런싱 필요                */
	weapon[pistol].max_bullet = 9999;   //pistol은 총알 무한
	weapon[1].max_bullet = 40;
	weapon[2].max_bullet = 20;
	weapon[3].max_bullet = 10;
	weapon[4].max_bullet = 10;
	weapon[5].max_bullet = 5;

	/*             무기별 데미지 설정, 밸런싱 필요                      */
	weapon[pistol].damage = 20;
	weapon[1].damage = 20;
	weapon[2].damage = 50;
	weapon[3].damage = 300;
	weapon[4].damage = 0;  //wall 데미지 없음
	weapon[5].damage = 300;

	/*            무기별 사정거리 설정, 밸런싱 필요                     */
	weapon[pistol].range = 200;
	weapon[1].range = 400;
	weapon[2].range = 100;
	weapon[3].range = 0;
	weapon[4].range = 0;
	weapon[5].range = 2000;  //사정거리 무한

  /*            무기별 설정, 밸런싱 필요                              */

	for (int i = 1; i < 6; i++)
	{
		weapon[i].open = false;
		weapon[i].bullet = 0;
		weapon[i].isdelay = false;
	}

	weapon[pistol].open = true;   //처음엔 권총밖에 엄서.. 야캐요..
	weapon[pistol].bullet = weapon[pistol].max_bullet;
	weapon[pistol].isdelay = false;

	selected_weapon = pistol;     //권총 선택
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Reset_weapon_upgrade()   //무기 업그레이드 초기화
{
	pistol_range_up1 = false;
	pistol_delay_down1 = false;
	//pistol 총알 무한

	uzi_range_up1 = false;
	uzi_delay_down1 = false;
	uzi_maxammo_up1 = false;
	uzi_maxammo_up2 = false;

	shotgun_range_up1 = false;
	shotgun_delay_down1 = false;
	shotgun_maxammo_up1 = false;
	shotgun_maxammo_up2 = false;

	barrel_maxammo_up1 = false;
	barrel_maxammo_up2 = false;

	wall_maxammo_up1 = false;
	wall_maxammo_up2 = false;

	//rocket 사정거리 무한
	rocket_delay_down1 = false;
	rocket_maxammo_up1 = false;
	rocket_maxammo_up2 = false;
}

