#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <random>
#include <time.h>
#include <math.h>
#include <atlimage.h>


HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("agari!");

double PI = 3.1415926535897 / 180;

#define character_width 38
#define character_height 60


enum direction  //���� N=0,NE=1,E=2,SE=3,S=4,SW=5,W=6,NW=7
{
	N = 0, NE = 1, E = 2, SE = 3, S = 4, SW = 5, W = 6, NW = 7
};
enum page       //������
{
	start_page, game_page, end_page
};
enum weapom     //�����ȣ pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5
{
	pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5
};
enum timer      //Ÿ�̸� �ѹ��� ���� ��� �̰� ���ݽô� 
{
	rest_time,
	move_player, atk_player,
	spawn_monster, move_monster,
	spawn_itembox, sht_player,
	start_page_character
};

typedef struct CHARACTER   //�÷��̾�, ����, �������� ����
{
	int health;            //���� ĳ���� ü��  
	int max_health;        //ĳ���� �ִ� ü��
	int x;                 //���� ĳ���� x��ǥ 
	int y;                 //���� ĳ���� y��ǥ 
	int direction;         //���� ĳ���Ͱ� �ٶ󺸴� ���� enum direction���� ���
	int sprite_num;        //ĳ���� ��������Ʈ num
	bool ismoving;         //�̵������� (Ÿ�̸� ���ư����ִ���)
	bool isattacking;      //���������� (Ÿ�̸� ���ư����ִ���)
	bool isshooting;       //�Ѿ��� ���󰬴���

	CHARACTER* next;       //����, �������Ϳ� next��ǥ
}CHARACTER;


typedef struct WEAPON       //���� ����
{
	bool open;              //���� ���� ����
	int bullet;             //���⺰ ���� ź��
	int max_bullet;         //���⺰ �ִ� ź��, ������ �ڽ��� Ȯ�������� �ִ�ź�� Ȯ�� ����
	int damage;             //���⺰ ������        
	int range;              //���⺰ �����Ÿ�
	int delay;              //���� ��� ������ 
	CImage image;			//���� �̹���
}WEAPON;


typedef struct ROCKET
{
	bool launch;
	int x, y;
	int direction;
	int speed;

}ROCKET;


typedef struct BLOCK         //50*50ũ�� ���
{
	bool iswall;              //���� �ִ�?
	bool isinvinciblewall;    //�ı� �Ұ����� ���� �ִ�?
	bool isbarrel;            //������ �ִ�?
	bool isitembox;           //�����۹ڽ��� �ִ�?
	bool isempty;             //����ִ�? == ���� 4 ������Ʈ�� �ƹ��͵� ����߸� true!
	bool isnew;               //��ݸ���Ŵ�? == true�� �������!

	int hp;                   //wall�ϰ�� ü�� ����

	RECT rect;                //��� ��ǥ 
}BLOCK;

/************************************************�Լ� ����********************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime);
void Game_start_setting(HWND hWnd);  //���� ���� ����(���� �ʱ�ȭ ���)
void Reset_weapon_setting();
void Reset_weapon_upgrade();
void Stage_start(HWND hWnd);
bool Spawn_itembox();
bool Aquire_itembox();      //������ �ڽ��� �Ծ�η���!
void Spawn_monster();
void Spawn_boss();
bool Crash_check_character2object(int speed, int direction);
bool Crash_check_monster2object(int speed, CHARACTER* monster);
bool Char_Deathcheck(HWND hWnd);
bool Remaining_bullet_check();  //���� �Ѿ� Ȯ��  �������� false ������ true
void Crash_check_bullet2object(double x, double y, int* dx, int* dy, double addX, double addY, HWND hWnd);
bool Crash_check_bullet2monster(int x, int y, CHARACTER* prev, CHARACTER* p, int type, HWND hWnd);
void bomb_barrel(int x, int y, HWND hWnd);
/*********************************************void()�Լ� �ְ�*****************************************************/

/*********************************************����մϴ� ��������*****************************************************/

int win_x_size = 900;      //������ x������
int win_y_size = 800;      //������ y������
int block_size = 50;       //��� ũ��

BLOCK block[32][36];       //36*32 ��� 

int mx;                    //���콺 x��ġ��	
int my;                    //���콺 y��ġ��

int current_page;          //���� ������  enum page�� ���
int stage;                 //���� ��������
int score;                 //���� ���ھ�


CHARACTER* monster_head;  //head���� �������� �ʰ� ����ϱ�
int monster_num;          //���� ���� ����(player.isattacking == true)
int monster_dmg;          //���� ������

CHARACTER* boss_head;     //head���� �������� �ʰ� ����ϱ�
int boss_num;             //���� �������� ����
int boss_dmg;             //�������� ������

int itembox_num;           //�ʿ� �����ϴ� ������ �ڽ� ����
int aquired_itembox_num;   //������� ������ ������ �ڽ� ����


CHARACTER player;          //�÷��̾�
WEAPON weapon[6];          //���� 6�� enum weapon���� ���
int selected_weapon;       //���� �������� ���� enum weapon���� ���

int rocket_x_size = 40;
int rocket_y_size = 40;
ROCKET rocket_bullet;

bool cheatmode;

/////////////////////////////////////���� ���׷��̵� bool����/////////////////////////////////////
//�����ȣ pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5
bool pistol_range_up1;
bool pistol_delay_down1;
//pistol �Ѿ� ����

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

//rocket �����Ÿ� ����
bool rocket_delay_down1;
bool rocket_maxammo_up1;
bool rocket_maxammo_up2;

////////////////////////////////////////////////////////////////////////////////////////////////

RECT char_move_sprite_rect[8][4] = //��������Ʈ ��ǥ
{ { { 0,0,18,30 },{ 32,0,50,30 },{ 64,0,82,30 },{ 96,0,114,30 } }/*N*/,{ { 133,0,151,30 },{ 165,0,173,30 },{ 197,0,215,30 },{ 229,0,247,30 } }/*NE*/,
{ { 264,0,282,30 },{ 296,0,314,30 },{ 328,0,346,30 },{ 360,0,388,30 } }/*E*/,{ { 397,0,415,30 },{ 428,0,446,30 },{ 460,0,488,30 },{ 492,0,510,30 } }/*SE*/,
{ { 0,36,18,66 },{ 32,36,50,66 },{ 64,36,82,66 },{ 96,36,114,66 } }/*S*/,{ { 133,36,151,66 },{ 165,36,173,66 },{ 197,36,215,66 },{ 229,36,247,66 } }/*SW*/,
{ { 264,36,282,66 },{ 296,36,314,66 },{ 328,36,346,66 },{ 360,36,388,66 } }/*W*/,{ { 397,36,415,66 },{ 428,36,446,66 },{ 460,36,488,66 },{ 492,36,510,66 } }/*NW*/ };

RECT char_atk_sprite_rect[8][2] =  //���ݸ�� ��������Ʈ ��ǥ
{ {{0,0,30,32}, {30,0,60,32} }/*N*/, {{ 60,0,90,32 }, {90,0,120,32} }/*NE*/,{{120,0,150,32},{150,0,180,32}}/*E*/,{{180,0,210,32},{210,0,240,32}}/*SE*/,
{ { 0,32,30,64 },{ 30,32,60,64 } }/*S*/,{ { 60,32,90,64 },{ 90,32,120,64 } }/*SW*/,{ { 120,32,150,64 },{ 150,32,180,64 } }/*W*/,{ { 180,32,210,64 },{ 210,32,240,64 }/*NW*/ } };

RECT char_weapon_sprite_rect[8][2] = //���� ��������Ʈ ��ǥ
{ {{0,0,50,52},{50,0,100,52}}/*N*/, {{100,0,150,52},{150,0,200,52}}/*NE*/ ,{ {200,0,250,52},{250,0,300,52}}/*E*/, {{300,0,350,52},{350,0,400,52}}/*SE*/,
{ { 0,52,50,104 },{ 50,52,100,104 } }/*S*/,{ { 100,52,150,104 },{ 150,52,200,104 } }/*SW*/ ,{ { 200,52,250,104 },{ 250,52,300,104 } }/*W*/,{ { 300,52,350,104 },{ 350,52,400,104 } }/*NW*/ };

RECT monster_atk_sprite_rect[8][2] =  //���ݸ�� ��������Ʈ ��ǥ
{ { { 0,0,34,37 },{ 34,0,68,37 } }/*N*/,{ { 68,0,102,37 },{ 102,0,136,37 } }/*NE*/,{ { 136,0,170,37 },{ 170,0,204,37 } }/*E*/,{ { 204,0,238,37 },{ 238,0,272,37 } }/*SE*/,
 { { 0,37,34,74 },{ 34,37,68,74 } }/*S*/,{ { 68,37,102,74 },{ 102,37,136,74 } }/*SW*/,{ { 136,37,170,74 },{ 170,37,204,74 } }/*W*/,{ { 204,37,238,74 },{ 238,37,272,74 } }/*NW*/ };

RECT logo_rect = { 120,120,750,370 };          //�ΰ� ��ġ      (��ŸƮȭ��)
RECT play_button_rect = { 650,480,800,550 };   //play��ư ��ġ  (��ŸƮȭ��)
RECT exit_button_rect = { 650,600,800,670 };   //exit��ư ��ġ  (��ŸƮȭ��)

RECT weapon_image_rect[6] = { { 220,730,270,780 },{ 300,730,350,780 },{ 380,730,430,780 },{ 460,730,510,780 },{ 540,730,590,780 },{ 620,730,670,780 } };  //���� 6�� �̹��� ��ġ

RECT GAMEOVER_rect = { 220,50,620,150 };       //���ӿ��� ��ư ��ġ  (���ӿ���ȭ��)
RECT rankingbox_rect = { 100,200,745,600 };    //��ŷ�ڽ� ��ġ       (���ӿ���ȭ��)
RECT replay_button_rect = { 150,640,300,710 }; //���÷��� ��ư ��ġ  (���ӿ���ȭ��)
RECT exit2_button_rect = { 530,640,680,710 };  //exit��ư ��ġ       (���ӿ���ȭ��)    //exit��ư�� �� ������°� ���ݾ����� �̹� �ʾ��� ������ exit2

/*********************************************����մϴ� ���尡��������������*****************************************/


/*********************************************��Ʈ�� �̹���*****************************************************/


CImage start_page_bk_img;   //����ȭ�� ���
CImage game_page_bk_img;    //����ȭ�� ���
CImage end_page_bk_img;     //����ȭ�� ���

CImage play_button_img;     //play��ư
CImage exit_button_img;     //exit��ư
CImage replay_button_img;	//���÷��� ��ư

CImage itembox_img;
CImage wall_img;
CImage barrel_img;
CImage rocket_bullet_img;

int current_char_num;       //ĳ���� ���ÿ�... ����� ������

//////
CImage charac_sprite[8];
CImage charac_atk_sprite[8];
CImage charac_weapon_sprite[6];	//���ݸ�� ���� ��������Ʈ

CImage monster_sprite;
CImage monster_atk_sprite;

CImage boss_sprite;
CImage boss_atk_sprite;
//////

CImage monster_move_sprite[8][4]; //���� �̵� ��������Ʈ [����][�̵� ��� 4��(?)]


/*********************************************��Ʈ�� �̹���*****************************************************/
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
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, _T("agari!"), WS_OVERLAPPEDWINDOW, 0, 0, win_x_size + 14, win_y_size + 36, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	srand(GetTickCount());  //��ƽī��Ʈ! �����õ�~

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

	static bool play_button = false;  //����ȭ�� start��ư
	static bool exit_button = false;  //����ȭ�� exit��ư

	static bool replay_button = false;  //����ȭ�� replay��ư
	static bool exit2_button = false;  //����ȭ�� exit��ư


	switch (uMsg) {
	case WM_CREATE:
	{
		current_char_num = 0;
		cheatmode = false;
		player.direction = S;
		player.sprite_num = 0;
		/*********************************************�̹��� �ε�*****************************************************/

		play_button_img.Load(TEXT("..\\agari\\resource\\PLAY.png"));
		exit_button_img.Load(TEXT("..\\agari\\resource\\EXIT.png"));
		replay_button_img.Load(TEXT("..\\agari\\resource\\REPLAY.png"));
		start_page_bk_img.Load(TEXT("..\\agari\\resource\\startBack.png"));
		game_page_bk_img.Load(TEXT("..\\agari\\resource\\stage.png"));
		end_page_bk_img.Load(TEXT("..\\agari\\resource\\endBack.png"));

		charac_sprite[0].Load(TEXT("..\\agari\\resource\\Izuna_move.png"));
		charac_sprite[1].Load(TEXT("..\\agari\\resource\\Gen-An_move.png"));
		charac_sprite[2].Load(TEXT("..\\agari\\resource\\Hinagiku_move.png"));
		charac_sprite[3].Load(TEXT("..\\agari\\resource\\Ichika_move.png"));
		charac_sprite[4].Load(TEXT("..\\agari\\resource\\Kagen_move.png"));
		charac_sprite[5].Load(TEXT("..\\agari\\resource\\Mitsumoto_move.png"));
		charac_sprite[6].Load(TEXT("..\\agari\\resource\\Shino_move.png"));
		charac_sprite[7].Load(TEXT("..\\agari\\resource\\Sizune_move.png"));

		charac_atk_sprite[0].Load(TEXT("..\\agari\\resource\\Izuna_attack.png"));
		charac_atk_sprite[1].Load(TEXT("..\\agari\\resource\\Gen-An_attack.png"));
		charac_atk_sprite[2].Load(TEXT("..\\agari\\resource\\Hinagiku_attack.png"));
		charac_atk_sprite[3].Load(TEXT("..\\agari\\resource\\Ichika_attack.png"));
		charac_atk_sprite[4].Load(TEXT("..\\agari\\resource\\Kagen_attack.png"));
		charac_atk_sprite[5].Load(TEXT("..\\agari\\resource\\Mitsumoto_attack.png"));
		charac_atk_sprite[6].Load(TEXT("..\\agari\\resource\\Shino_attack.png"));
		charac_atk_sprite[7].Load(TEXT("..\\agari\\resource\\Sizune_attack.png"));

		charac_weapon_sprite[pistol].Load(TEXT("..\\agari\\resource\\attack_pistol.png"));
		charac_weapon_sprite[uzi].Load(TEXT("..\\agari\\resource\\attack_uzi.png"));
		charac_weapon_sprite[shotgun].Load(TEXT("..\\agari\\resource\\attack_shotgun.png"));
		charac_weapon_sprite[rocket].Load(TEXT("..\\agari\\resource\\attack_rocket.png"));


		monster_sprite.Load(TEXT("..\\agari\\resource\\Monster_move.png"));
		monster_atk_sprite.Load(TEXT("..\\agari\\resource\\Monster_attack.png"));

		weapon[pistol].image.Load(TEXT("..\\agari\\resource\\ui_pistol.png"));
		weapon[uzi].image.Load(TEXT("..\\agari\\resource\\ui_uzi.png"));
		weapon[shotgun].image.Load(TEXT("..\\agari\\resource\\ui_shotgun.png"));
		weapon[barrel].image.Load(TEXT("..\\agari\\resource\\ui_barrel.png"));
		weapon[wall].image.Load(TEXT("..\\agari\\resource\\ui_wall.png"));
		weapon[rocket].image.Load(TEXT("..\\agari\\resource\\ui_rocket.png"));

		itembox_img.Load(TEXT("..\\agari\\resource\\cube.png"));
		wall_img.Load(TEXT("..\\agari\\resource\\wall.png"));
		barrel_img.Load(TEXT("..\\agari\\resource\\barrel.png"));
		rocket_bullet_img.Load(TEXT("..\\agari\\resource\\Rocket.png"));

		/*********************************************�̹��� �ε�*****************************************************/

		current_page = start_page;  //���� �������� ������������

		for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 36; j++)
			{
				block[i][j].rect = { 50 * j,50 * i,50 * j + 50,50 * i + 50 };  //36*32 ��� ��ǥ �Է�
			}
		}

		monster_head = (CHARACTER*)malloc(sizeof(CHARACTER));
		monster_head->next = NULL;
		boss_head = (CHARACTER*)malloc(sizeof(CHARACTER));
		boss_head->next = NULL;

		SetTimer(hWnd, start_page_character, 100, TimerProc);
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		if (current_page == start_page)
		{
			dc.Create(win_x_size, win_y_size, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();					// == CreateComaptibleDC

			start_page_bk_img.Draw(memdc1, 0, 0, win_x_size, win_y_size);
			play_button_img.Draw(memdc1, play_button_rect);
			exit_button_img.Draw(memdc1, exit_button_rect);

			charac_sprite[current_char_num].Draw(memdc1, 680, 100, character_width * 2, character_height * 2,
				char_move_sprite_rect[player.direction][player.sprite_num].left, char_move_sprite_rect[player.direction][player.sprite_num].top, 18, 30);

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);	// �Ʒ��� Bitblt�� ����
			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����

								// �Ʒ��� ���� �۾��� (���صǸ� ������)
								//BitBlt(hdc, 0, 0, win_x_size, win_y_size, memdc1, 0, 0, SRCCOPY);
								//DeleteObject(membitmap1);
								//DeleteObject(memdc1);
		}
		else if (current_page == game_page)
		{
			dc.Create(win_x_size * 2, win_y_size * 2, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();							// == CreateComaptibleDC

			//////////////////////////////////////////��� ���///////////////////////////////////////////////

			/*
			**��� ����ǥ**
			hbrush = CreateSolidBrush(RGB(200, 255, 255));
			oldbrush = (HBRUSH)SelectObject(memdc1, hbrush);
			Rectangle(memdc1, 0, 0, win_x_size * 2, win_y_size * 2);
			SelectObject(memdc1, oldbrush);
			DeleteObject(hbrush);
			*/

			game_page_bk_img.BitBlt(memdc1, 0, 0, SRCCOPY); //���

			for (int i = 0; i < 33; i++)                    //��� ���м�
			{
				MoveToEx(memdc1, 0, i * 50, NULL);
				LineTo(memdc1, win_x_size * 2, i * 50);
			}
			for (int i = 0; i < 37; i++)
			{
				MoveToEx(memdc1, i * 50, 0, NULL);
				LineTo(memdc1, i * 50, win_y_size * 2);
			}

			//////////////////////////////////////////���� ������Ʈ ���///////////////////////////////////////////////
			for (int i = 0; i < 32; i++)
			{
				for (int j = 0; j < 36; j++)
				{
					if (block[i][j].isitembox)
					{

						itembox_img.Draw(memdc1, block[i][j].rect.left + 10, block[i][j].rect.top + 4, 30, 41, 0, 0, 30, 41);
					}
					else if ((block[i][j].iswall) || (block[i][j].isinvinciblewall))
					{
						wall_img.Draw(memdc1, block[i][j].rect.left, block[i][j].rect.top, 50, 50, 0, 0, 22, 30);
					}
					else if (block[i][j].isbarrel)
					{
						barrel_img.Draw(memdc1, block[i][j].rect.left, block[i][j].rect.top, 50, 50, 0, 0, 544, 720);
					}
				}
			}

			/////////////////////////////////////////���� ���////////////////////////////////////////
			if (rocket_bullet.launch == true)
			{
				rocket_bullet_img.Draw(memdc1, rocket_bullet.x - (rocket_x_size / 2), rocket_bullet.y - (rocket_y_size / 2), rocket_x_size, rocket_y_size);
			}

			//////////////////////////////////////////ĳ���� ���///////////////////////////////////////////////
			if (player.isattacking == false)  //������ �ƴҶ�
			{
				charac_sprite[current_char_num].Draw(memdc1, player.x - character_width / 2, player.y - character_height / 2, character_width, character_height,
					char_move_sprite_rect[player.direction][player.sprite_num].left, char_move_sprite_rect[player.direction][player.sprite_num].top, 18, 30);
			}
			else                              //�������϶�
			{
				charac_atk_sprite[current_char_num].Draw(memdc1, player.x - 24, player.y - 32, 64, 64,
					char_atk_sprite_rect[player.direction][player.sprite_num].left, char_atk_sprite_rect[player.direction][player.sprite_num].top, 30, 32);

				charac_weapon_sprite[selected_weapon].Draw(memdc1, player.x - 34, player.y - 42, 84, 84,
					char_weapon_sprite_rect[player.direction][player.sprite_num].left, char_weapon_sprite_rect[player.direction][player.sprite_num].top, 50, 52);

				if ((selected_weapon == pistol || selected_weapon == uzi) && player.isshooting)
				{
					hpen = CreatePen(PS_SOLID, 2, BLACK_PEN);
					oldpen = (HPEN)SelectObject(memdc1, hpen);
					int dx, dy;
					switch (player.direction)
					{
					case N:
						dx = player.x;
						dy = (player.y - 32) - weapon[selected_weapon].range;
						Crash_check_bullet2object(player.x, player.y - 32, &dx, &dy, 0, -1, hWnd);
						MoveToEx(memdc1, player.x, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case NE:
						dx = (player.x + 40) + (weapon[selected_weapon].range*cos(45 * PI));
						dy = (player.y - 32) - (weapon[selected_weapon].range*sin(45 * PI));
						Crash_check_bullet2object(player.x + 40, player.y - 32, &dx, &dy, cos(45 * PI), sin(45 * PI)*-1, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case E:
						dx = (player.x + 40) + weapon[selected_weapon].range;
						dy = player.y;
						Crash_check_bullet2object(player.x + 40, player.y, &dx, &dy, 1, 0, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case SE:
						dx = (player.x + 40) + (weapon[selected_weapon].range*cos(45 * PI));
						dy = (player.y + 32) + (weapon[selected_weapon].range*sin(45 * PI));
						Crash_check_bullet2object(player.x + 40, player.y + 32, &dx, &dy, cos(45 * PI), sin(45 * PI), hWnd);
						MoveToEx(memdc1, player.x + 40, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case S:
						dx = player.x;
						dy = (player.y + 32) + weapon[selected_weapon].range;
						Crash_check_bullet2object(player.x, player.y + 32, &dx, &dy, 0, 1, hWnd);
						MoveToEx(memdc1, player.x, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case SW:
						dx = (player.x - 40) - (weapon[selected_weapon].range*cos(45 * PI));
						dy = (player.y + 32) + (weapon[selected_weapon].range*sin(45 * PI));
						Crash_check_bullet2object(player.x - 40, player.y + 32, &dx, &dy, cos(45 * PI)*-1, sin(45 * PI), hWnd);
						MoveToEx(memdc1, player.x - 40, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case W:
						dx = (player.x - 40) - weapon[selected_weapon].range;
						dy = player.y;
						Crash_check_bullet2object(player.x - 40, player.y, &dx, &dy, -1, 0, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case NW:
						dx = (player.x - 40) - (weapon[selected_weapon].range*cos(45 * PI));
						dy = (player.y - 32) - (weapon[selected_weapon].range*sin(45 * PI));
						Crash_check_bullet2object(player.x - 40, player.y - 32, &dx, &dy, cos(45 * PI)*-1, sin(45 * PI) - 1, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);
						break;
					}
					SelectObject(memdc1, oldpen);
					DeleteObject(hpen);

				}
				if ((selected_weapon == shotgun) && player.isshooting)
				{
					hpen = CreatePen(PS_SOLID, 2, BLACK_PEN);
					oldpen = (HPEN)SelectObject(memdc1, hpen);
					int dx, dy;
					switch (player.direction)
					{
					case N:
						dx = player.x;
						dy = (player.y - 32) - weapon[selected_weapon].range;
						Crash_check_bullet2object(player.x, player.y - 32, &dx, &dy,
							(double)(dx - (player.x)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y - 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = player.x - 40;
						dy = (player.y - 32) - weapon[selected_weapon].range;
						Crash_check_bullet2object(player.x, player.y - 32, &dx, &dy,
							(double)(dx - (player.x)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y - 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = player.x + 40;
						dy = (player.y - 32) - weapon[selected_weapon].range;
						Crash_check_bullet2object(player.x, player.y - 32, &dx, &dy,
							(double)(dx - (player.x)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y - 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case NE:
						dx = (player.x + 40) + (weapon[selected_weapon].range*cos(45 * PI));
						dy = (player.y - 32) - (weapon[selected_weapon].range*sin(45 * PI));
						Crash_check_bullet2object(player.x + 40, player.y - 32, &dx, &dy,
							(double)(dx - (player.x + 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y - 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x + 40) + (weapon[selected_weapon].range*cos(45 * PI)) - 30;
						dy = (player.y - 32) - (weapon[selected_weapon].range*sin(45 * PI)) - 30;
						Crash_check_bullet2object(player.x + 40, player.y - 32, &dx, &dy,
							(double)(dx - (player.x + 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y - 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x + 40) + (weapon[selected_weapon].range*cos(45 * PI)) + 30;
						dy = (player.y - 32) - (weapon[selected_weapon].range*sin(45 * PI)) + 30;
						Crash_check_bullet2object(player.x + 40, player.y - 32, &dx, &dy,
							(double)(dx - (player.x + 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y - 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case E:
						dx = (player.x + 40) + weapon[selected_weapon].range;
						dy = player.y;
						Crash_check_bullet2object(player.x + 40, player.y, &dx, &dy,
							(double)(dx - (player.x + 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x + 40) + weapon[selected_weapon].range;
						dy = player.y - 40;
						Crash_check_bullet2object(player.x + 40, player.y, &dx, &dy,
							(double)(dx - (player.x + 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x + 40) + weapon[selected_weapon].range;
						dy = player.y + 40;
						Crash_check_bullet2object(player.x + 40, player.y, &dx, &dy,
							(double)(dx - (player.x + 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case SE:
						dx = (player.x + 40) + (weapon[selected_weapon].range*cos(45 * PI));
						dy = (player.y + 32) + (weapon[selected_weapon].range*sin(45 * PI));
						Crash_check_bullet2object(player.x + 40, player.y + 32, &dx, &dy,
							(double)(dx - (player.x + 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y + 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x + 40) + (weapon[selected_weapon].range*cos(45 * PI)) - 30;
						dy = (player.y + 32) + (weapon[selected_weapon].range*sin(45 * PI)) + 30;
						Crash_check_bullet2object(player.x + 40, player.y + 32, &dx, &dy,
							(double)(dx - (player.x + 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y + 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x + 40) + (weapon[selected_weapon].range*cos(45 * PI)) + 30;
						dy = (player.y + 32) + (weapon[selected_weapon].range*sin(45 * PI)) - 30;
						Crash_check_bullet2object(player.x + 40, player.y + 32, &dx, &dy,
							(double)(dx - (player.x + 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y + 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x + 40, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case S:
						dx = player.x;
						dy = (player.y + 32) + weapon[selected_weapon].range;
						Crash_check_bullet2object(player.x, player.y + 32, &dx, &dy,
							(double)(dx - (player.x)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y + 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = player.x - 40;
						dy = (player.y + 32) + weapon[selected_weapon].range;
						Crash_check_bullet2object(player.x, player.y + 32, &dx, &dy,
							(double)(dx - (player.x)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y + 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = player.x + 40;
						dy = (player.y + 32) + weapon[selected_weapon].range;
						Crash_check_bullet2object(player.x, player.y + 32, &dx, &dy,
							(double)(dx - (player.x)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y + 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case SW:
						dx = (player.x - 40) - (weapon[selected_weapon].range*cos(45 * PI));
						dy = (player.y + 32) + (weapon[selected_weapon].range*sin(45 * PI));
						Crash_check_bullet2object(player.x - 40, player.y + 32, &dx, &dy,
							(double)(dx - (player.x - 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y + 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x - 40) - (weapon[selected_weapon].range*cos(45 * PI)) - 30;
						dy = (player.y + 32) + (weapon[selected_weapon].range*sin(45 * PI)) - 30;
						Crash_check_bullet2object(player.x - 40, player.y + 32, &dx, &dy,
							(double)(dx - (player.x - 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y + 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x - 40) - (weapon[selected_weapon].range*cos(45 * PI)) + 30;
						dy = (player.y + 32) + (weapon[selected_weapon].range*sin(45 * PI)) + 30;
						Crash_check_bullet2object(player.x - 40, player.y + 32, &dx, &dy,
							(double)(dx - (player.x - 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y + 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y + 32, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case W:
						dx = (player.x - 40) - weapon[selected_weapon].range;
						dy = player.y;
						Crash_check_bullet2object(player.x - 40, player.y, &dx, &dy,
							(double)(dx - (player.x - 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x - 40) - weapon[selected_weapon].range;
						dy = player.y - 40;
						Crash_check_bullet2object(player.x - 40, player.y, &dx, &dy,
							(double)(dx - (player.x - 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x - 40) - weapon[selected_weapon].range;
						dy = player.y + 40;
						Crash_check_bullet2object(player.x - 40, player.y, &dx, &dy,
							(double)(dx - (player.x - 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y, NULL);
						LineTo(memdc1, dx, dy);
						break;

					case NW:
						dx = (player.x - 40) - (weapon[selected_weapon].range*cos(45 * PI));
						dy = (player.y - 32) - (weapon[selected_weapon].range*sin(45 * PI));
						Crash_check_bullet2object(player.x - 40, player.y - 32, &dx, &dy,
							(double)(dx - (player.x - 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y - 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x - 40) - (weapon[selected_weapon].range*cos(45 * PI)) + 30;
						dy = (player.y - 32) - (weapon[selected_weapon].range*sin(45 * PI)) - 30;
						Crash_check_bullet2object(player.x - 40, player.y - 32, &dx, &dy,
							(double)(dx - (player.x - 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y - 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);

						dx = (player.x - 40) - (weapon[selected_weapon].range*cos(45 * PI)) - 30;
						dy = (player.y - 32) - (weapon[selected_weapon].range*sin(45 * PI)) + 30;
						Crash_check_bullet2object(player.x - 40, player.y - 32, &dx, &dy,
							(double)(dx - (player.x - 40)) / (double)weapon[selected_weapon].range, (double)(dy - (player.y - 32)) / (double)weapon[selected_weapon].range, hWnd);
						MoveToEx(memdc1, player.x - 40, player.y - 32, NULL);
						LineTo(memdc1, dx, dy);
						break;
					}

					SelectObject(memdc1, oldpen);
					DeleteObject(hpen);
				}
			}

			//////////////////////////////////////////ü�¹� ���///////////////////////////////////////////////
			Rectangle(memdc1, player.x - 20, player.y - 40, player.x + 20, player.y - 33);  //ü�¹�
			hbrush = CreateSolidBrush(RGB(255 - (int((float)player.health / (float)player.max_health*255.0)), int((float)player.health / (float)player.max_health*255.0), 0));   //ü�� �ۼ�Ʈ���� �� �ٸ���
			oldbrush = (HBRUSH)SelectObject(memdc1, hbrush);
			Rectangle(memdc1, player.x - 19, player.y - 39, int(player.x - 19 + ((float)player.health / (float)player.max_health*38.0)), player.y - 34);
			SelectObject(memdc1, oldbrush);
			DeleteObject(hbrush);

			//////////////////////////////////////////���� ���///////////////////////////////////////////////
			CHARACTER* p = monster_head->next;

			while (p != NULL)
			{
				if (p->isattacking == false) //������ �ƴҶ�
				{
					monster_sprite.Draw(memdc1, p->x - character_width / 2, p->y - character_height / 2, character_width, character_height,
						char_move_sprite_rect[p->direction][p->sprite_num].left, char_move_sprite_rect[p->direction][p->sprite_num].top, 18, 30);
				}
				else                         //�������϶�
				{
					monster_atk_sprite.Draw(memdc1, p->x - 24, p->y - 32, 64, 64,
						monster_atk_sprite_rect[p->direction][p->sprite_num].left, monster_atk_sprite_rect[p->direction][p->sprite_num].top, 34, 37);
				}
				p = p->next;
			}

			//////////////////////////////////////////�������� ���///////////////////////////////////////////////

			//CHARACTER* p = boss_head->next;

			//while (p != NULL)
			//{
			//	if (p->isattacking == false) //������ �ƴҶ�
			//	{

			//	}
			//	else                         //�������϶�
			//	{

			//	}
			//	p = p->next;
			//}


			//TextOut(memdc1, player.x - (win_x_size / 2), player.y - (win_y_size / 2), str, _tcslen(str));

			//������Ʈ ���(��, ����)

			//�Ѿ� ���


			//////////////////////////////////////�ϴ� �� ���� ���//////////////////////////////////////////

			dc2.Create(win_x_size, win_y_size, 24);		// ȭ�� ��¿� DC
			memdc2 = dc2.GetDC();						// ȭ�� ��¿� DC

			if (player.x <= (win_x_size / 2))
			{
				play_size_left = 0;
			}
			else if (player.x >= (win_x_size * 2) - (win_x_size / 2))
			{
				play_size_left = (win_x_size * 2) - win_x_size;
			}
			else
			{
				play_size_left = player.x - (win_x_size / 2);
			}

			if (player.y <= (win_y_size / 2))
			{
				play_size_top = 0;
			}
			else if (player.y >= (win_y_size * 2) - (win_y_size / 2))
			{
				play_size_top = (win_y_size * 2) - win_y_size;
			}
			else
			{
				play_size_top = player.y - (win_y_size / 2);
			}

			dc.BitBlt(memdc2, 0, 0, win_x_size, win_y_size, play_size_left, play_size_top);

			//////////////////////////////////////////DEBUG///////////////////////////////////////////////
			int temp_atk;
			if (player.isattacking == true)
				temp_atk = 1;
			else
				temp_atk = 0;

			int temp_mv;
			if (player.ismoving == true)
				temp_mv = 1;
			else
				temp_mv = 0;
			//wsprintf(str, TEXT("stage : %d  score : %d  direction : %d  x : %d  y: %d  weapon : %d  bullet : %d  itembox_num : %d  ak? %d mv? %d"), stage, score, player.direction, player.x, player.y, selected_weapon, weapon[selected_weapon].bullet, itembox_num, temp_atk, temp_mv);
			//TextOut(memdc2, 0, 0, str, _tcslen(str));


			wsprintf(str, TEXT("SCORE  %d"), score);
			hFont = CreateFont(30, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("�������"));
			oldFont = (HFONT)SelectObject(memdc2, hFont);
			TextOut(memdc2, 5, 5, str, _tcsclen(str));
			SelectObject(memdc2, oldFont);
			DeleteObject(hFont);

			wsprintf(str, TEXT("STAGE  %d"), stage);
			hFont = CreateFont(25, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("�������"));
			oldFont = (HFONT)SelectObject(memdc2, hFont);
			TextOut(memdc2, 390, 5, str, _tcsclen(str));
			SelectObject(memdc2, oldFont);
			DeleteObject(hFont);

			//////////////////////////////////////////UI///////////////////////////////////////////////

			TCHAR bullet_num[5] = {};   //�Ѿ˰���
			TCHAR weapon_num[2] = {};   //�����ȣ

			hbrush = CreateSolidBrush(RGB(255, 255, 255));
			oldbrush = (HBRUSH)SelectObject(memdc2, hbrush);
			for (int i = 0; i < 6; ++i)
			{
				RECT print_rect = weapon_image_rect[i];
				wsprintf(bullet_num, TEXT("%d"), weapon[i].bullet);
				wsprintf(weapon_num, TEXT("%d"), i + 1);

				if (selected_weapon == i)  //���õ� ����� 25�ȼ� ���� ���
				{
					print_rect.top -= 25;
					print_rect.bottom -= 25;
				}
				RoundRect(memdc2, print_rect.left, print_rect.top, print_rect.right, print_rect.bottom, 10, 10);

				weapon[i].image.Draw(memdc2, print_rect);

				SetBkMode(memdc2, TRANSPARENT);  //������

				print_rect.top -= 20;
				print_rect.bottom -= 20;
				DrawText(memdc2, bullet_num, _tcslen(bullet_num), &print_rect, DT_CENTER | DT_VCENTER);

				print_rect.top += 70;
				print_rect.bottom += 70;
				DrawText(memdc2, weapon_num, _tcslen(weapon_num), &print_rect, DT_CENTER | DT_VCENTER);
			}
			SelectObject(memdc2, oldbrush);
			DeleteObject(hbrush);

			/*
			**ui ����ǥ**

			if ((win_x_size / 2 <= player.x) && (player.x <= win_x_size / 2 * 3) && (win_y_size / 2 <= player.y) && (player.y <= win_y_size / 2 * 3))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, player.x - (win_x_size / 2) + weapon_image_rect[i].left, player.y - (win_y_size / 2) + weapon_image_rect[i].top,
						player.x - (win_x_size / 2) + weapon_image_rect[i].right, player.y - (win_y_size / 2) + weapon_image_rect[i].bottom, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, player.x - (win_x_size / 2), player.y - (win_y_size / 2), win_x_size, win_y_size);
			}

			else if ((player.x < win_x_size / 2) && (player.y < win_y_size / 2))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left, weapon_image_rect[i].top, weapon_image_rect[i].right, weapon_image_rect[i].bottom, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, 0, 0, win_x_size, win_y_size);
			}
			else if ((win_x_size / 2 * 3 < player.x) && (player.y < win_y_size / 2))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + win_x_size, weapon_image_rect[i].top, weapon_image_rect[i].right + win_x_size, weapon_image_rect[i].bottom, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, win_x_size, 0, win_x_size, win_y_size);
			}
			else if ((win_x_size / 2 * 3 < player.x) && (win_y_size / 2 * 3 < player.y))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + win_x_size, weapon_image_rect[i].top + win_y_size, weapon_image_rect[i].right + win_x_size,
						weapon_image_rect[i].bottom + win_y_size, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, win_x_size, win_y_size, win_x_size, win_y_size);
			}
			else if ((player.x < win_x_size / 2) && (win_y_size / 2 * 3 < player.y))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left, weapon_image_rect[i].top + win_y_size, weapon_image_rect[i].right, weapon_image_rect[i].bottom + win_y_size, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, 0, win_y_size, win_x_size, win_y_size);
			}
			else if (player.x < win_x_size / 2)
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left, weapon_image_rect[i].top + player.y - (win_y_size / 2),
						weapon_image_rect[i].right, weapon_image_rect[i].bottom + player.y - (win_y_size / 2), 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, 0, player.y - (win_y_size / 2), win_x_size, win_y_size);
			}
			else if (player.y < win_y_size / 2)
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + player.x - (win_x_size / 2), weapon_image_rect[i].top,
						weapon_image_rect[i].right + player.x - (win_x_size / 2), weapon_image_rect[i].bottom, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, player.x - (win_x_size / 2), 0, win_x_size, win_y_size);
			}
			else if (win_x_size / 2 * 3 < player.x)
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + win_x_size, weapon_image_rect[i].top + player.y - (win_y_size / 2),
						weapon_image_rect[i].right + win_x_size, weapon_image_rect[i].bottom + player.y - (win_y_size / 2), 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, win_x_size, player.y - (win_y_size / 2), win_x_size, win_y_size);
			}
			else if (win_y_size / 2 * 3 < player.y)
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + player.x - (win_x_size / 2), weapon_image_rect[i].top + win_y_size,
						weapon_image_rect[i].right + player.x - (win_x_size / 2), weapon_image_rect[i].bottom + win_y_size, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, player.x - (win_x_size / 2), win_y_size, win_x_size, win_y_size);
			}
			*/


			dc2.Draw(hdc, 0, 0);

			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����
			dc2.ReleaseDC();
			dc2.Destroy();
		}
		else if (current_page == end_page)
		{

			dc.Create(win_x_size, win_y_size, 24);
			memdc1 = dc.GetDC();
			end_page_bk_img.Draw(memdc1, 0, 0, win_x_size, win_y_size);

			//���ھ�
			wsprintf(str, TEXT("SCORE  %d"), score);
			hFont = CreateFont(100, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("�ü�"));
			oldFont = (HFONT)SelectObject(memdc1, hFont);
			TextOut(memdc1, 380, 100, str, _tcsclen(str));
			SelectObject(memdc1, oldFont);
			DeleteObject(hFont);

			//��������
			wsprintf(str, TEXT("STAGE  %d"), stage);
			hFont = CreateFont(100, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("�ü�"));
			oldFont = (HFONT)SelectObject(memdc1, hFont);
			TextOut(memdc1, 380, 250, str, _tcsclen(str));
			SelectObject(memdc1, oldFont);
			DeleteObject(hFont);
			//replay��ư
			replay_button_img.Draw(memdc1, replay_button_rect);

			exit_button_img.Draw(memdc1, exit2_button_rect);   //exit��ư

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);

			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����
		}

		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:
	{
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		if (current_page == start_page)
		{
			if ((play_button_rect.left <= mx) && (play_button_rect.top <= my) && (mx <= play_button_rect.right) && (my <= play_button_rect.bottom))  //play��ư ����, ������ ���� �۵�
			{
				play_button = true;
				{
					/***********************************

					��ư Ŭ���� ����Ʈ ������ ������ �� ������ �ϴ�..��

					************************************/
				}
			}
			else if ((exit_button_rect.left <= mx) && (exit_button_rect.top <= my) && (mx <= exit_button_rect.right) && (my <= exit_button_rect.bottom)) //exit��ư ����, ������ ���� �۵�
			{
				exit_button = true;
				{
					/***********************************

					��ư Ŭ���� ����Ʈ ������ ������ �� ������ �ϴ�..��

					************************************/
				}
			}
		}
		else if (current_page == game_page)
		{



		}
		else if (current_page == end_page)
		{
			if ((replay_button_rect.left <= mx) && (replay_button_rect.top <= my) && (mx <= replay_button_rect.right) && (my <= replay_button_rect.bottom))  //replay��ư ����, ������ ���� �۵�
			{
				replay_button = true;
				{
					/***********************************

					��ư Ŭ���� ����Ʈ ������ ������ �� ������ �ϴ�..��

					************************************/
				}
			}
			else if ((exit2_button_rect.left <= mx) && (exit2_button_rect.top <= my) && (mx <= exit2_button_rect.right) && (my <= exit2_button_rect.bottom)) //exit2��ư ����, ������ ���� �۵�
			{
				exit2_button = true;
				{
					/***********************************

					��ư Ŭ���� ����Ʈ ������ ������ �� ������ �ϴ�..��

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
				current_page = game_page;   //����ȭ������ �̵�
				play_button = false;

				SetTimer(hWnd, spawn_itembox, 15000, TimerProc);//15�ʸ��� �����۹ڽ� ����
				Game_start_setting(hWnd);                       //���� ���� ����(���� �ʱ�ȭ ���)

				InvalidateRect(hWnd, NULL, false);
			}
			else if (exit_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //���α׷� ����
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
				current_page = game_page;   //����ȭ������ �̵�
				play_button = false;

				SetTimer(hWnd, spawn_itembox, 15000, TimerProc);//15�ʸ��� �����۹ڽ� ����
				Game_start_setting(hWnd);                       //���� ���� ����(���� �ʱ�ȭ ���)


				InvalidateRect(hWnd, NULL, false);
			}
			else if (exit2_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //���α׷� ����
				break;
			}
		}
		break;
	}
	case WM_MOUSEMOVE:

		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		break;
	}
	case WM_KEYDOWN:
	{
		if (current_page == start_page)
		{
			switch (wParam)
			{
			case 'p':
			case 'P':
				cheatmode = true;
				break;

			case '1':
				current_char_num = 0;
				break;

			case '2':
				current_char_num = 1;
				break;

			case '3':
				current_char_num = 2;
				break;

			case '4':
				current_char_num = 3;
				break;

			case '5':
				current_char_num = 4;
				break;

			case '6':
				current_char_num = 5;
				break;

			case '7':
				current_char_num = 6;
				break;

			case '8':
				current_char_num = 7;
				break;
			}
			InvalidateRect(hWnd, NULL, false);
		}

		else if (current_page == game_page)
		{

			switch (wParam)
			{
			case 'w':
			case 'W':
			{
				if ((player.direction == N) || (player.direction == S) || (player.direction == SE) || (player.direction == SW))  //�̵� ������ ������
					player.direction = N;
				else if (player.direction == W)  //�̵� ������ �ϼ���
				{
					if (player.ismoving == true)
						player.direction = NW;
					else
						player.direction = N;
				}
				else if (player.direction == E)  //�̵� ������ �ϵ�����
				{
					if (player.ismoving == true)
						player.direction = NE;
					else
						player.direction = N;
				}

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'a':
			case 'A':
			{
				if ((player.direction == E) || (player.direction == W) || (player.direction == NE) || (player.direction == SE))  //�̵� ������ ����
					player.direction = W;
				else if (player.direction == N)  //�̵� ������ �ϼ���
				{
					if (player.ismoving == true)
						player.direction = NW;
					else
						player.direction = W;
				}
				else if (player.direction == S)  //�̵� ������ ������
				{
					if (player.ismoving == true)
						player.direction = SW;
					else
						player.direction = W;
				}

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 's':
			case 'S':
			{
				if ((player.direction == N) || (player.direction == S) || (player.direction == NE) || (player.direction == NW))  //�̵� ������ ������
					player.direction = S;
				else if (player.direction == W)  //�̵� ������ ������
				{
					if (player.ismoving == true)
						player.direction = SW;
					else
						player.direction = S;
				}
				else if (player.direction == E)  //�̵� ������ ��������
				{
					if (player.ismoving == true)
						player.direction = SE;
					else
						player.direction = S;
				}

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'd':
			case 'D':
			{
				if ((player.direction == E) || (player.direction == W) || (player.direction == NW) || (player.direction == SW))  //�̵� ������ ������
					player.direction = E;
				else if (player.direction == N)  //�̵� ������ �ϵ�����
				{
					if (player.ismoving == true)
						player.direction = NE;
					else
						player.direction = E;
				}
				else if (player.direction == S)  //�̵� ������ ��������
				{
					if (player.ismoving == true)
						player.direction = SE;
					else
						player.direction = E;
				}

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}

			case 'q':  //���� ���� ����
			case 'Q':
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (selected_weapon == pistol)                    //pistol�� ���� ó�� ����
					break;
				else if (weapon[selected_weapon - 1].open == true)     //���� ���⸦ �������ִٸ�
				{
					selected_weapon--;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case 'e':  //���� ���� ����
			case 'E':
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (selected_weapon == rocket)                     //rocket�� ���� ������ ����
					break;
				else if (weapon[selected_weapon + 1].open == true) //���� ���⸦ �������ִٸ�
				{
					selected_weapon++;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case '1':  //pistol ����
			{

				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[pistol].open == true)
				{
					selected_weapon = pistol;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case '2':  //uzi ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[uzi].open == true)
				{
					selected_weapon = uzi;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '3':  //shotgun ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[shotgun].open == true)
				{
					selected_weapon = shotgun;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '4':  //barrel ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[barrel].open == true)
				{
					selected_weapon = barrel;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '5':  //wall ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[wall].open == true)
				{
					selected_weapon = wall;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '6':  //rocket ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[rocket].open == true)
				{
					selected_weapon = rocket;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case VK_SPACE:  //���� ���
			{
				if (Remaining_bullet_check())      //�Ѿ� ���� ���� ���� �ƴ���
				{
					//�޼����� ���ٴ���... �Ѿ��� ������ �˸��� ���� 
					//�����ٸ� �׳� ���� �� ���� break�� Ż�����ڱ�?

				}
				else
				{

					/*   ����! pistol�� �Ѿ��� �����̹Ƿ� pistol �߻�� �Ѿ� �پ��� �ȴ�!            */
					/*   ����! �Ѹ��� �߻� �����̰� ����! �߻��� Ÿ�̸ӷ� ������ �����ϴ°� ��������!  */  // wall�� barrel�� �����̰� ����! 

					if (selected_weapon == pistol)
					{
						if (player.isattacking == false)
						{
							SetTimer(hWnd, atk_player, weapon[selected_weapon].delay, TimerProc);
							player.isshooting = true;
							player.isattacking = true;
						}
					}
					else if (selected_weapon == uzi)
					{
						if (player.isattacking == false)
						{
							SetTimer(hWnd, atk_player, weapon[selected_weapon].delay, TimerProc);
							player.isattacking = true;
						}
					}
					else if (selected_weapon == shotgun)
					{
						if (player.isattacking == false)
						{
							SetTimer(hWnd, atk_player, weapon[selected_weapon].delay, TimerProc);
							player.isattacking = true;
						}
					}
					else if (selected_weapon == barrel)
					{
						/*               ��� �浹üũ                */
						int temp_x;
						int temp_y;

						for (int i = 0; i < 31; i++)
						{
							if (block[i][0].rect.bottom <= player.y + (character_height / 2) && player.y + (character_height / 2) <= block[i + 1][0].rect.bottom)
							{
								temp_y = i + 1;
								break;
							}
						}
						for (int i = 0; i < 36; i++)
						{
							if ((block[0][i].rect.left <= player.x) && (player.x <= block[0][i + 1].rect.right))//x���� ã��
							{
								temp_x = i + 1;
								break;
							}
						}
						if (block[temp_y][temp_x].isempty == false)  //������� �ʴٸ� break;
							break;
						else  //���� ���
						{
							block[temp_y][temp_x].isbarrel = true;
							block[temp_y][temp_x].isempty = false;
							block[temp_y][temp_x].isnew = true;
							weapon[selected_weapon].bullet--;
						}

						InvalidateRect(hWnd, NULL, false);
					}
					else if (selected_weapon == wall)
					{
						/*               ��� �浹üũ                */
						int temp_x;
						int temp_y;

						for (int i = 0; i < 31; i++)
						{
							if (block[i][0].rect.bottom <= player.y + (character_height / 2) && player.y + (character_height / 2) <= block[i + 1][0].rect.bottom)
							{
								temp_y = i + 1;
								break;
							}
						}
						for (int i = 0; i < 36; i++)
						{
							if ((block[0][i].rect.left <= player.x) && (player.x <= block[0][i + 1].rect.right))//x���� ã��
							{
								temp_x = i + 1;
								break;
							}
						}
						if (block[temp_y][temp_x].isempty == false)  //������� �ʴٸ� break;
							break;
						else  //wall ���
						{
							block[temp_y][temp_x].iswall = true;
							block[temp_y][temp_x].isempty = false;
							block[temp_y][temp_x].isnew = true;
							block[temp_y][temp_x].hp = 200;
							weapon[selected_weapon].bullet--;
						}

						InvalidateRect(hWnd, NULL, false);
					}
					else if (selected_weapon == rocket)
					{
						if (player.isattacking == false)
						{
							SetTimer(hWnd, atk_player, weapon[selected_weapon].delay, TimerProc);
							player.isattacking = true;
						}
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
			case 'W':
			{
				if (player.direction == NE)       //�̵� ������ ������
					player.direction = E;
				else if (player.direction == NW)  //�̵� ������ ����
					player.direction = W;
				else if (player.direction == N)   //�̵��� ����
				{
					player.ismoving = false;
					player.sprite_num = 0;
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'a':
			case 'A':
			{
				if (player.direction == NW)       //�̵� ������ ������
					player.direction = N;
				else if (player.direction == SW)  //�̵� ������ ������
					player.direction = S;
				else if (player.direction == W)   //�̵��� ����
				{
					player.ismoving = false;
					player.sprite_num = 0;
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 's':
			case 'S':
			{
				if (player.direction == SE)       //�̵� ������ ������
					player.direction = E;
				else if (player.direction == SW)  //�̵� ������ ����
					player.direction = W;
				else if (player.direction == S)   //�̵��� ����
				{
					player.ismoving = false;
					player.sprite_num = 0;
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'd':
			case 'D':
			{
				if (player.direction == NE)       //�̵� ������ ������
					player.direction = N;
				else if (player.direction == SE)  //�̵� ������ ������
					player.direction = S;
				else if (player.direction == E)   //�̵��� ����
				{
					player.ismoving = false;
					player.sprite_num = 0;
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case VK_SPACE:  //���� ����
			{
				if (player.isattacking == true)
				{
					player.sprite_num = 0;
					player.isattacking = false;
					player.isshooting = false;
					KillTimer(hWnd, atk_player);
					InvalidateRect(hWnd, NULL, false);
				}

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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime)
{
	switch (idEvent)
	{
	case start_page_character:
	{
		player.sprite_num++;
		player.sprite_num %= 4;
		InvalidateRect(hWnd, NULL, false);
		break;
	}


	case rest_time:  //�� �������� ���� �� ���� ������������ ��
	{

		Stage_start(hWnd);


		KillTimer(hWnd, rest_time);
		break;
	}
	case spawn_itembox:
	{
		Spawn_itembox();
		InvalidateRect(hWnd, NULL, false);
		break;
	}
	case atk_player: //���� ���
	{
		if (selected_weapon == pistol)
		{
			//���� �߻�
			SetTimer(hWnd, sht_player, 20, TimerProc);

			//ĳ���� ��������Ʈ ����
		}
		else if (selected_weapon == uzi)
		{

			if (Remaining_bullet_check())   //���� �Ѿ� Ȯ�� -> ������ killtimer, isfiring=false
			{
				player.isattacking = false;
				KillTimer(hWnd, atk_player);
				//ĳ���� ��������Ʈ ����

			}
			else
			{
				//���� �߻�
				SetTimer(hWnd, sht_player, 20, TimerProc);

				//ĳ���� ��������Ʈ ����
				weapon[selected_weapon].bullet--;//�Ѿ� ����
			}
		}
		else if (selected_weapon == shotgun)
		{
			if (Remaining_bullet_check())//���� �Ѿ� Ȯ�� -> ������ killtimer, isfiring=false
			{
				player.isattacking = false;
				KillTimer(hWnd, atk_player);
				//ĳ���� ��������Ʈ ����

			}
			else
			{
				//���� �߻�
				SetTimer(hWnd, sht_player, 20, TimerProc);

				//ĳ���� ��������Ʈ ����
				weapon[selected_weapon].bullet--;//�Ѿ� ����
			}
		}
		else if (selected_weapon == rocket)
		{
			if (Remaining_bullet_check())//���� �Ѿ� Ȯ�� -> ������ killtimer, isfiring=false
			{
				player.isattacking = false;
				KillTimer(hWnd, atk_player);
				//ĳ���� ��������Ʈ ����
			}
			else
			{
				//���� �߻�
				rocket_bullet.launch = true;
				rocket_bullet.direction = player.direction;
				switch (rocket_bullet.direction)
				{
				case N:
					rocket_bullet.x = player.x;
					rocket_bullet.y = player.y - 32;
					break;

				case NE:
					rocket_bullet.x = player.x + 40;
					rocket_bullet.y = player.y - 32;
					break;

				case E:
					rocket_bullet.x = player.x + 40;
					rocket_bullet.y = player.y;
					break;

				case SE:
					rocket_bullet.x = player.x + 40;
					rocket_bullet.y = player.y + 32;
					break;

				case S:
					rocket_bullet.x = player.x;
					rocket_bullet.y = player.y + 32;
					break;

				case SW:
					rocket_bullet.x = player.x - 40;
					rocket_bullet.y = player.y + 32;
					break;

				case W:
					rocket_bullet.x = player.x - 40;
					rocket_bullet.y = player.y;
					break;

				case NW:
					rocket_bullet.x = player.x - 40;
					rocket_bullet.y = player.y - 32;
					break;

				}
				//ĳ���� ��������Ʈ ����
				weapon[selected_weapon].bullet--;//�Ѿ� ����
			}
		}

		if (player.isattacking == true)
		{
			if (player.sprite_num == 0)
				player.sprite_num = 1;
			else
				player.sprite_num = 0;
		}

		InvalidateRect(hWnd, NULL, false);

		break;
	}
	case move_player:
	{
		if (rocket_bullet.launch == true)
		{
			switch (rocket_bullet.direction)
			{
			case N:
				rocket_bullet.y -= rocket_bullet.speed;
				break;

			case NE:
				rocket_bullet.x += rocket_bullet.speed;
				rocket_bullet.y -= rocket_bullet.speed;
				break;

			case E:
				rocket_bullet.x += rocket_bullet.speed;
				break;

			case SE:
				rocket_bullet.x += rocket_bullet.speed;
				rocket_bullet.y += rocket_bullet.speed;
				break;

			case S:
				rocket_bullet.y += rocket_bullet.speed;
				break;

			case SW:
				rocket_bullet.x -= rocket_bullet.speed;
				rocket_bullet.y += rocket_bullet.speed;
				break;

			case W:
				rocket_bullet.x -= rocket_bullet.speed;
				break;

			case NW:
				rocket_bullet.x -= rocket_bullet.speed;
				rocket_bullet.y -= rocket_bullet.speed;
				break;
			}
		}

		if (player.ismoving == false)
		{
			InvalidateRect(hWnd, NULL, false);
			break;
		}

		int x_crash_check;
		int y_crash_check;

		int speed = 5;

		if (player.isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
		{
			x_crash_check = 32;
			y_crash_check = 32;
		}
		else
		{
			x_crash_check = 19;
			y_crash_check = 30;
		}

		if (Crash_check_character2object(speed, player.direction) == false)
		{
			if (player.direction == N)
			{
				if (!(player.y - y_crash_check - speed < 50))  //�� ��� �浹üũ
				{
					player.y -= speed;
				}
			}
			else if (player.direction == NE)
			{
				if (!(win_x_size * 2 < player.x + x_crash_check + speed))  //�� ��� �浹üũ
					player.x += speed;
				if (!(player.y - y_crash_check - speed < 50))  //�� ��� �浹üũ
					player.y -= speed;
			}
			else if (player.direction == E)
			{
				if (!(win_x_size * 2 < player.x + x_crash_check + speed))  //�� ��� �浹üũ
					player.x += speed;
			}
			else if (player.direction == SE)
			{
				if (!(win_x_size * 2 < player.x + x_crash_check + speed))  //�� ��� �浹üũ
					player.x += speed;
				if (!(win_y_size * 2 - 50 < player.y + y_crash_check + speed))  //�� ��� �浹üũ
					player.y += speed;
			}
			else if (player.direction == S)
			{
				if (!(win_y_size * 2 - 50 < player.y + y_crash_check + speed))  //�� ��� �浹üũ
					player.y += speed;
			}
			else if (player.direction == SW)
			{
				if (!(player.x - x_crash_check - speed < 0))  //�� ��� �浹üũ
					player.x -= speed;
				if (!(win_y_size * 2 - 50 < player.y + y_crash_check + speed))  //�� ��� �浹üũ
					player.y += speed;
			}
			else if (player.direction == W)
			{
				if (!(player.x - x_crash_check - speed < 0))  //�� ��� �浹üũ
					player.x -= speed;
			}
			else if (player.direction == NW)
			{
				if (!(player.x - x_crash_check - speed < 0))  //�� ��� �浹üũ
					player.x -= speed;
				if (!(player.y - y_crash_check - speed < 50))  //�� ��� �浹üũ
					player.y -= speed;
			}
		}

		if (player.isattacking == true)
		{
			if (player.sprite_num == 2 || player.sprite_num == 3)
				player.sprite_num = 0;
		}

		else if (player.isattacking == false)
		{
			if (player.sprite_num == 3)
				player.sprite_num = 0;
			else
				player.sprite_num++;
		}

		InvalidateRect(hWnd, NULL, false);
		break;
	}
	case move_monster:
	{
		int monster_speed = 5;          //���� ���ǵ� - �뷱�� ���� �ʿ�
		int monster_attack_range = 7;  //���� �����Ÿ� - �뷱�� ���� �ʿ�

		int x_crash_check;
		int y_crash_check;
		RECT monster_attack_range_rect = {};  //���� �����Ÿ� rect - intersectrect�� ���
		RECT temp_rect = {};             //intersectrect ����� �޾ƿ��� rect. ���ڸ� ����θ� �Լ��� �۵��� ���ؼ� �־����. �����ص���

		RECT player_rect;               //�÷��̾� rect - �÷��̾ �������� ���� �ƴҶ� ũ�Ⱑ �ٸ��� ������ ���
		if (player.isattacking == true) //�÷��̾� rect ����
			player_rect = { player.x - 32,player.y - 32,player.x + 32,player.y + 32 };
		else
			player_rect = { player.x - 32,player.y - 32,player.x + 32,player.y + 32 };

		CHARACTER* p = monster_head->next;
		while (p != NULL)
		{

			if (p->isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
			{
				x_crash_check = 32;
				y_crash_check = 32;
			}
			else
			{
				x_crash_check = 19;
				y_crash_check = 30;
			}
			monster_attack_range_rect = { p->x - x_crash_check - monster_attack_range/**/,p->y - y_crash_check - monster_attack_range/**/,
				p->x + x_crash_check + monster_attack_range /**/,p->y + y_crash_check + monster_attack_range }; //���� �����Ÿ� rect - intersectrect�� ���

			if (p->isattacking == false)
			{

				if (IntersectRect(&temp_rect, &player_rect, &monster_attack_range_rect))          //�̵��� �÷��̾ �����Ÿ� �ȿ� ������ 
				{
					//-> ���ݸ�� ����
					p->sprite_num = 0;
					p->isattacking = true;
					p->ismoving = false;

				}
				else if (abs(player.x - p->x) < character_width)
				{
					if (player.y > p->y)
					{
						p->direction = S;
					}
					else
					{
						p->direction = N;
					}
				}
				else if (abs(player.y - p->y) < character_height)
				{
					if (player.x > p->x)
					{
						p->direction = E;
					}
					else
					{
						p->direction = W;
					}
				}
				else if ((player.y > p->y) && (p->x < player.x))
				{
					p->direction = SE;
				}
				else if ((player.y > p->y) && (p->x > player.x))
				{
					p->direction = SW;
				}
				else if ((player.y < p->y) && (p->x > player.x))
				{
					p->direction = NW;
				}
				else if ((player.y < p->y) && (p->x < player.x))
				{
					p->direction = NE;
				}

				if (p->isattacking == false)  //�����߿� �̵��� ����
				{
					if (Crash_check_monster2object(monster_speed, p) == false)
					{
						if (p->direction == N)
						{
							if (!(p->y - y_crash_check - monster_speed < 0))  //�� ��� �浹üũ
								p->y -= monster_speed;
						}
						else if (p->direction == NE)
						{
							if (!(win_x_size * 2 < p->x + x_crash_check + monster_speed))  //�� ��� �浹üũ
								p->x += monster_speed;
							if (!(p->y - y_crash_check - monster_speed < 0))  //�� ��� �浹üũ
								p->y -= monster_speed;
						}
						else if (p->direction == E)
						{
							if (!(win_x_size * 2 < p->x + x_crash_check + monster_speed))  //�� ��� �浹üũ
								p->x += monster_speed;
						}
						else if (p->direction == SE)
						{
							if (!(win_x_size * 2 < p->x + x_crash_check + monster_speed))  //�� ��� �浹üũ
								p->x += monster_speed;
							if (!(win_y_size * 2 < p->y + y_crash_check + monster_speed))  //�� ��� �浹üũ
								p->y += monster_speed;
						}
						else if (p->direction == S)
						{
							if (!(win_y_size * 2 < p->y + y_crash_check + monster_speed))  //�� ��� �浹üũ
								p->y += monster_speed;
						}
						else if (p->direction == SW)
						{
							if (!(p->x - x_crash_check - monster_speed < 0))  //�� ��� �浹üũ
								p->x -= monster_speed;
							if (!(win_y_size * 2 < p->y + y_crash_check + monster_speed))  //�� ��� �浹üũ
								p->y += monster_speed;
						}
						else if (p->direction == W)
						{
							if (!(p->x - x_crash_check - monster_speed < 0))  //�� ��� �浹üũ
								p->x -= monster_speed;
						}
						else if (p->direction == NW)
						{
							if (!(p->x - x_crash_check - monster_speed < 0))  //�� ��� �浹üũ
								p->x -= monster_speed;
							if (!(p->y - y_crash_check - monster_speed < 0))  //�� ��� �浹üũ
								p->y -= monster_speed;
						}
					}
					if (p->sprite_num == 3)
						p->sprite_num = 0;
					else
						p->sprite_num++;
				}
			}
			else  //���� 
			{
				if (p->sprite_num == 0)
				{
					////////////////////////////////////�÷��̾� �ǰ�����////////////////////////////////////
					if (IntersectRect(&temp_rect, &player_rect, &monster_attack_range_rect)) //�÷��̾ �����Ÿ� �ȿ� ������
					{
						player.health -= monster_dmg;
						if (Char_Deathcheck(hWnd) == false)
						{
							int knockback_distance = 10;

							if (player.isattacking == true)   //�����ϴٰ� ������ ������ ����
							{
								KillTimer(hWnd, atk_player);
								player.isattacking = false;
								player.sprite_num = 0;
							}

							if (Crash_check_character2object(knockback_distance, p->direction) == false)
							{
								player.sprite_num = 0;

								x_crash_check = 19;
								y_crash_check = 30;

								if (p->direction == N)
								{
									if (!(player.y - y_crash_check - knockback_distance < 50))  //�� ��� �浹üũ
									{
										player.y -= knockback_distance;
									}
								}
								else if (p->direction == NE)
								{
									if (!(win_x_size * 2 < player.x + x_crash_check + knockback_distance))  //�� ��� �浹üũ
										player.x += knockback_distance;
									if (!(player.y - y_crash_check - knockback_distance < 50))  //�� ��� �浹üũ
										player.y -= knockback_distance;
								}
								else if (p->direction == E)
								{
									if (!(win_x_size * 2 < player.x + x_crash_check + knockback_distance))  //�� ��� �浹üũ
										player.x += knockback_distance;
								}
								else if (p->direction == SE)
								{
									if (!(win_x_size * 2 < player.x + x_crash_check + knockback_distance))  //�� ��� �浹üũ
										player.x += knockback_distance;
									if (!(win_y_size * 2 - 50 < player.y + y_crash_check + knockback_distance))  //�� ��� �浹üũ
										player.y += knockback_distance;
								}
								else if (p->direction == S)
								{
									if (!(win_y_size * 2 - 50 < player.y + y_crash_check + knockback_distance))  //�� ��� �浹üũ
										player.y += knockback_distance;
								}
								else if (p->direction == SW)
								{
									if (!(player.x - x_crash_check - knockback_distance < 0))  //�� ��� �浹üũ
										player.x -= knockback_distance;
									if (!(win_y_size * 2 - 50 < player.y + y_crash_check + knockback_distance))  //�� ��� �浹üũ
										player.y += knockback_distance;
								}
								else if (p->direction == W)
								{
									if (!(player.x - x_crash_check - knockback_distance < 0))  //�� ��� �浹üũ
										player.x -= knockback_distance;
								}
								else if (p->direction == NW)
								{
									if (!(player.x - x_crash_check - knockback_distance < 0))  //�� ��� �浹üũ
										player.x -= knockback_distance;
									if (!(player.y - y_crash_check - knockback_distance < 50))  //�� ��� �浹üũ
										player.y -= knockback_distance;
								}
							}
						}

					}
					p->sprite_num++;
				}
				else if (p->sprite_num == 1)   //���� ����
				{
					p->sprite_num = 0;
					p->isattacking = false;
					p->ismoving = true;
				}
			}

			p = p->next;
		}

		InvalidateRect(hWnd, NULL, false);
		break;
	}
	case sht_player:
		if (player.isshooting == false)
		{
			player.isshooting = true;
		}
		else
		{
			player.isshooting = false;
			KillTimer(hWnd, sht_player);
		}
		InvalidateRect(hWnd, NULL, false);
		break;

	}



}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Game_start_setting(HWND hWnd)         //���� ���� ����(���� �ʱ�ȭ ���)
{
	KillTimer(hWnd, start_page_character);

	stage = 0;                    //�������� �ʱ�ȭ (�������� ���� �Լ����� 1�� ������Ű�Ƿ� 0���� �ʱ�ȭ)
	score = 0;                    //���ھ� �ʱ�ȭ

	player.max_health = 1000;     //ĳ���� �ִ�ü�� 1000���� ����
	player.health = player.max_health;
	player.x = 900;               //��ġ �߾�
	player.y = 800;
	player.direction = S;         //�Ʒ��� �ٶ�
	player.sprite_num = 0;        //�⺻ �̹���
	player.ismoving = false;
	player.isattacking = false;
	player.isshooting = false;
	SetTimer(hWnd, move_player, 50, TimerProc);   //�÷��̾� �̵�

	monster_num = 0;
	boss_num = 0;

	monster_dmg = 100;
	//boss_dmg=

	rocket_bullet.launch = false;
	rocket_bullet.speed = 30;

	int rand_temp = 0;
	for (int i = 1; i < 31; i++)
		//������ �ʱ�ȭ
	{
		for (int j = 1; j < 35; j++)  //������Ʈ ��������
		{
			if ((15 < j) && (j < 21))
			{

				block[i][j].iswall = false;
				block[i][j].isinvinciblewall = false;       //�ı� �Ұ����� �� (����) �����Ϸ��� ���⼭! ���������� ��������?
				block[i][j].isbarrel = false;
				block[i][j].isitembox = false;
				block[i][j].isempty = true;
				block[i][j].isnew = false;
				block[i][j].hp = 0;
			}
			else
			{
				rand_temp = rand() % 150;

				block[i][j].iswall = false;
				if (rand_temp == 1)
					block[i][j].isinvinciblewall = true;
				else
					block[i][j].isinvinciblewall = false;       //�ı� �Ұ����� �� (����) �����Ϸ��� ���⼭! ���������� ��������?
				if (rand_temp == 2)
					block[i][j].isbarrel = true;
				else
					block[i][j].isbarrel = false;
				block[i][j].isitembox = false;
				if ((rand_temp != 1) && (rand_temp != 2))
					block[i][j].isempty = true;
				else
					block[i][j].isempty = false;
				block[i][j].isnew = false;
				block[i][j].hp = 0;
			}
		}
	}
	itembox_num = 0;
	aquired_itembox_num = 0;      //������ ������ �ڽ� ���� �ʱ�ȭ
	for (int i = 0; i < 5; i++)   //�����۹ڽ� 5�� ���� 
		Spawn_itembox();

	Reset_weapon_setting();



	SetTimer(hWnd, rest_time, 5000, TimerProc);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Reset_weapon_setting()
{
	/*              ���⺰ ����, �뷱�� �ʿ�                            */
	Reset_weapon_upgrade();
	/*pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5*/

	/*             ���⺰ �ִ� ź�� �� ����, �뷱�� �ʿ�                */
	weapon[pistol].max_bullet = 9999;   //pistol�� �Ѿ� ����
	weapon[uzi].max_bullet = 40;
	weapon[shotgun].max_bullet = 20;
	weapon[barrel].max_bullet = 10;
	weapon[wall].max_bullet = 10;
	weapon[rocket].max_bullet = 5;

	/*             ���⺰ ������ ����, �뷱�� �ʿ�                      */
	weapon[pistol].damage = 20;
	weapon[uzi].damage = 15;
	weapon[shotgun].damage = 50;
	weapon[barrel].damage = 300;
	weapon[wall].damage = 0;  //wall ������ ����
	weapon[rocket].damage = 300;

	/*            ���⺰ �����Ÿ� ����, �뷱�� �ʿ�                     */
	weapon[pistol].range = 200;
	weapon[uzi].range = 300;
	weapon[shotgun].range = 100;
	weapon[barrel].range = 0;
	weapon[wall].range = 0;
	weapon[rocket].range = 2000;  //�����Ÿ� ����

							 /*            ���⺰ ������ ����, �뷱�� �ʿ�                     */
	weapon[pistol].delay = 300;
	weapon[uzi].delay = 100;
	weapon[shotgun].delay = 400;
	weapon[barrel].delay = 0;
	weapon[wall].delay = 0;
	weapon[rocket].delay = 1800;

	/*            ���⺰ ����, �뷱�� �ʿ�                              */

	for (int i = 1; i < 6; i++)
	{
		weapon[i].open = false;
		weapon[i].bullet = 0;
		if (cheatmode == true)
		{
			weapon[i].open = true;
			weapon[i].bullet = 9999;
		}
	}

	weapon[pistol].open = true;   //ó���� ���ѹۿ� ����.. ��ĳ��..
	weapon[pistol].bullet = weapon[pistol].max_bullet;

	selected_weapon = pistol;     //���� ����
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Reset_weapon_upgrade()   //���� ���׷��̵� �ʱ�ȭ
{
	pistol_range_up1 = false;
	pistol_delay_down1 = false;
	//pistol �Ѿ� ����

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

	//rocket �����Ÿ� ����
	rocket_delay_down1 = false;
	rocket_maxammo_up1 = false;
	rocket_maxammo_up2 = false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Spawn_boss()
{
	/* ���� �� ���� �뷱�� �ʿ� */
	CHARACTER* p = boss_head;
	int i = 0;

	while (i < stage - 1/*���� �뷱�� ����*/)
	{
		//head���� �������� �ʰ� ����ϱ�
		CHARACTER* temp_character = (CHARACTER*)malloc(sizeof(CHARACTER));

		temp_character->health = 500;
		temp_character->max_health = 500;
		//   temp_character->x = 
		//   temp_character->y = 1
		//   temp_character->direction = 
		temp_character->sprite_num = 0;
		temp_character->ismoving = true;
		temp_character->isattacking = false;
		temp_character->next = NULL;

		p->next = temp_character;

		boss_num++;
		i++;
	}

	//�� ���� - ���Ḯ��Ʈ
	//head���� �������� �ʰ� ����ϱ�
	//�� ��ǥ ���� - ���̵� : HELL
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Stage_start(HWND hWnd)   //�������� ���� ����(���� ����) �� �Լ� �ۼ� ���������� �������´�� ���°ž� �������
{
	stage++;

	Spawn_monster();
	//Spawn_boss();    //�̱��� �ФФФ�
	SetTimer(hWnd, move_monster, 100, TimerProc);
	//�� �ʿ��� �������� �ϴ� Ÿ�̸� ���� - �� ������ҿ��� �ѵθ����� ������, �Ա��� ������ ������ ���Ұ�� ������ �ʵ��� �ؾ���
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Spawn_itembox()     //18*16 ��Ͽ��� ����ִ� ��Ͽ��� �������� ������ �ڽ� ����	
{
	if (itembox_num > 4) //�����۹ڽ��� 5��������! (�뷱�� �ʿ�)
		return 1;
	else
	{
		int temp_x;
		int temp_y;

		while (true)     //����ִ� ĭ�� ã�ƺ��ô�!
		{
			temp_x = rand() % 34 + 1;    //1~34 ���� ����
			temp_y = rand() % 30 + 1;    //1~30 ���� ����

			if (block[temp_y][temp_x].isempty == true)
				break;
		}

		block[temp_y][temp_x].isitembox = true;
		block[temp_y][temp_x].isempty = false;

		itembox_num++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aquire_itembox()             //������ �ڽ��� �Ծ�η���!
{
	/*  ������ �ڽ� ����, itembox_num--���� ó�� �ʿ�  */
	itembox_num--;
	aquired_itembox_num++;

	int opened_weapon_num;        //�߰� ������ ���� ���� (pistol ����)
	for (int i = 5; i > -1; i--)
	{
		if (weapon[i].open == true)
		{
			opened_weapon_num = i;
			break;
		}
	}

	if (2 <= stage)  //2������������ ���� �ϳ��� Ǯ��
	{
		for (int i = 1; i < stage; i++)
		{
			if (weapon[i].open == false)
			{
				weapon[i].open = true;
				weapon[i].bullet = weapon[i].max_bullet;
				return 0;
			}
			if (i == 5)
				break;
		}
	}

	/*             Ư�� ����, �뷱��? ���� ��ȹ �ʿ�              */
	/*                ���� ���׷��̵� �ر� ������                 */
	//if ((aquired_itembox_num == ) || ())
	//{

	//}
	//else if ((aquired_itembox_num == ) || ())
	//{

	//}

	/*             ���׷��̵� �Ұ��ɽ�               */


	int rand_num = rand() % (opened_weapon_num * 2 + 1);   //���̵� ������ �� Ȯ�� �߰��ϸ� ��

	if (rand_num == 0)  //   1/((ȹ���� ���� ����*2)+1)�� Ȯ���� ü��ȸ��
	{
		player.health = player.max_health;
	}
	else if ((rand_num == 1) || (rand_num == 2))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		if ((weapon[uzi].bullet + 20) > weapon[uzi].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
			weapon[uzi].bullet = weapon[uzi].max_bullet;
		weapon[uzi].bullet += 20;
	}
	else if ((rand_num == 3) || (rand_num == 4))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		if ((weapon[shotgun].bullet + 10) > weapon[shotgun].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
			weapon[shotgun].bullet = weapon[shotgun].max_bullet;
		weapon[shotgun].bullet += 10;
	}
	else if ((rand_num == 5) || (rand_num == 6))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		if ((weapon[barrel].bullet + 5) > weapon[barrel].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
			weapon[barrel].bullet = weapon[barrel].max_bullet;
		weapon[barrel].bullet += 5;
	}
	else if ((rand_num == 7) || (rand_num == 8))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		if ((weapon[wall].bullet + 5) > weapon[wall].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
			weapon[wall].bullet = weapon[wall].max_bullet;
		weapon[wall].bullet += 5;
	}
	else if ((rand_num == 9) || (rand_num == 10))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		if ((weapon[rocket].bullet + 3) > weapon[rocket].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
			weapon[rocket].bullet = weapon[rocket].max_bullet;
		weapon[rocket].bullet += 3;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Spawn_monster()
{
	/* ���� �� ���� �뷱�� �ʿ� */
	CHARACTER* p = monster_head;
	int i = 0;
	int spawn_direection = N;
	int temp = 0;
	int tmep = 100;
	while (i < stage * 10 + 1/*���� �뷱�� ����*/)
	{
		if (i == (stage * 10 + 1) / 2)
		{
			spawn_direection = S;
			temp = 0;
		}
		//head���� �������� �ʰ� ����ϱ�
		CHARACTER* temp_character = (CHARACTER*)malloc(sizeof(CHARACTER));

		temp_character->health = 100;
		temp_character->max_health = 100;
		temp_character->x = 850 + tmep;
		if (spawn_direection == S)
			temp_character->y = (1650 + temp);
		else if (spawn_direection == N)
			temp_character->y = (-50 - temp);
		temp_character->direction = E;
		temp_character->sprite_num = 0;
		temp_character->ismoving = false;
		temp_character->isattacking = false;
		temp_character->next = NULL;

		p->next = temp_character;

		monster_num++;
		i++;
		temp += 70;
		if (tmep == 100)
			tmep = 0;
		else if (tmep == 0)
			tmep = 100;

		p = p->next;
	}

	//�� ���� - ���Ḯ��Ʈ
	//head���� �������� �ʰ� ����ϱ�
	//�� ��ǥ ���� - ���̵� : HELL
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Crash_check_character2object(int speed, int direction)    //������Ʈ���� �浹�˻�
{

	////////////////////////////////////////����������Ʈ �浹�˻�////////////////////////////////////////
	RECT character_rect;
	RECT temp_rect = {};  //intersectrect ����� �޾ƿ��� rect. ���ڸ� ����θ� �Լ��� �۵��� ���ؼ� �־����. �����ص���

	if (player.isattacking)
	{
		character_rect = { player.x - 32,player.y - 32,player.x + 32,player.y + 32 };

	}
	else
	{
		character_rect = { player.x - 19,player.y - 30,player.x + 19,player.y + 30 };
	}


	if (direction == N)  //���� ��ǥ ����x �̵� �Ŀ� ����� ��ġ����
	{
		character_rect.top -= speed;
	}
	else if (direction == NE)
	{
		character_rect.top -= speed;
		character_rect.right += speed;

	}
	else if (direction == E)
	{
		character_rect.right += speed;
	}
	else if (direction == SE)
	{
		character_rect.bottom += speed;
		character_rect.right += speed;
	}
	else if (direction == S)
	{
		character_rect.bottom += speed;
	}
	else if (direction == SW)
	{
		character_rect.bottom += speed;
		character_rect.left -= speed;
	}
	else if (direction == W)
	{
		character_rect.left -= speed;
	}
	else if (direction == NW)
	{
		character_rect.top -= speed;
		character_rect.left -= speed;
	}

	for (int i = (player.x / 50 - 1); i <= (player.x / 50 + 1); i++)
	{
		if (i < 0)
			continue;

		for (int j = (player.y / 50 - 1); j <= (player.y / 50 + 1); j++)
		{
			if (j < 0)
				continue;

			if (j > 31)
				break;

			if (block[j][i].isnew)
			{
				if (IntersectRect(&temp_rect, &block[j][i].rect, &character_rect))
				{
					continue;
				}
				else
					block[j][i].isnew = false;

			}

			if (IntersectRect(&temp_rect, &block[j][i].rect, &character_rect))
			{
				if ((block[j][i].isinvinciblewall) || (block[j][i].iswall) || (block[j][i].isbarrel) && !block[j][i].isnew)   //�������� ������
				{
					return true;
				}
				else if (block[j][i].isitembox)
				{
					block[j][i].isitembox = false;
					block[j][i].isempty = true;
					Aquire_itembox();  //�����۹ڽ� ����
				}
			}

		}
	}
	////////////////////////////////////////���� �浹�˻�////////////////////////////////////////
	{
		CHARACTER* p = monster_head->next;
		while (p != NULL)
		{
			RECT monster_rect;

			if (p->isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
				monster_rect = { p->x - 32,p->y - 32,p->x + 32,p->y + 32 };
			else
				monster_rect = { p->x - 19,p->y - 30,p->x + 19,p->y + 30 };

			if (IntersectRect(&temp_rect, &monster_rect, &character_rect))
				return true;

			p = p->next;
		}
	}


	////////////////////////////////////////���� �浹�˻�////////////////////////////////////////
	{
		CHARACTER* p = boss_head->next;
		while (p != NULL)
		{
			RECT monster_rect;

			if (p->isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
				monster_rect = { p->x - 32,p->y - 32,p->x + 32,p->y + 32 };
			else
				monster_rect = { p->x - 19,p->y - 30,p->x + 19,p->y + 30 };

			if (IntersectRect(&temp_rect, &monster_rect, &character_rect))
				return true;

			p = p->next;
		}
	}



	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Crash_check_monster2object(int speed, CHARACTER* monster)    //���Ϳ� ������Ʈ���� �浹�˻�
{
	/***************************************************************/

	// ĳ���� - ���Ͱ��� �浹�� Crash_check_character2object���� �˻���.
	// ���ʹ� �����Ÿ� �ȿ� ĳ���Ͱ� ������ *����*�ؼ� �����ϱ� ������ 
	//���Ͱ� ĳ���Ϳ� �浹�ϴ����� ���� �˻� ���ʿ�

	/****************************************************************/

	////////////////////////////////////////����������Ʈ �浹�˻�////////////////////////////////////////

	RECT monster_rect = { monster->x - 19, monster->y - 30, monster->x + 19, monster->y + 30 };
	RECT temp_rect = {};  //intersectrect ����� �޾ƿ��� rect. ���ڸ� ����θ� �Լ��� �۵��� ���ؼ� �־����. �����ص���

	if (monster->direction == N)  //���� ��ǥ ����x �̵� �Ŀ� ����� ��ġ����
	{
		monster_rect.top -= speed;
	}
	else if (monster->direction == NE)
	{
		monster_rect.top -= speed;
		monster_rect.right += speed;

	}
	else if (monster->direction == E)
	{
		monster_rect.right += speed;
	}
	else if (monster->direction == SE)
	{
		monster_rect.bottom += speed;
		monster_rect.right += speed;
	}
	else if (monster->direction == S)
	{
		monster_rect.bottom += speed;
	}
	else if (monster->direction == SW)
	{
		monster_rect.bottom += speed;
		monster_rect.left -= speed;
	}
	else if (monster->direction == W)
	{
		monster_rect.left -= speed;
	}
	else if (monster->direction == NW)
	{
		monster_rect.top -= speed;
		monster_rect.left -= speed;
	}

	for (int i = (monster->x / 50 - 1); i <= (monster->x / 50 + 1); i++)
	{
		if (i < 0)
			continue;

		for (int j = (monster->y / 50 - 1); j <= (monster->y / 50 + 1); j++)
		{
			if (j < 0)
				continue;

			if (j > 31)
				break;

			if (IntersectRect(&temp_rect, &block[j][i].rect, &monster_rect))
			{
				if ((block[j][i].isinvinciblewall) || (block[j][i].iswall) || (block[j][i].isbarrel))   //�������� ������
				{
					return true;
				}
			}

		}
	}
	////////////////////////////////////////���� �浹�˻�////////////////////////////////////////
	{
		CHARACTER* p = monster_head->next;
		while (p != NULL)
		{
			if (p == monster)  //�ڱ� �ڽŰ��� �浹�˻�� ���ʿ�!
			{
				p = p->next;
				continue;
			}
			RECT p_rect;

			if (p->isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
				p_rect = { p->x - 32,p->y - 32,p->x + 32,p->y + 32 };
			else
				p_rect = { p->x - 19,p->y - 30,p->x + 19,p->y + 30 };

			if (IntersectRect(&temp_rect, &p_rect, &monster_rect))
				return true;

			p = p->next;
		}
	}


	////////////////////////////////////////���� �浹�˻�////////////////////////////////////////
	{
		CHARACTER* p = boss_head->next;
		while (p != NULL)
		{
			if (p == monster)  //�ڱ� �ڽŰ��� �浹�˻�� ���ʿ�!
			{
				p = p->next;
				continue;
			}
			RECT p_rect;

			if (p->isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
				p_rect = { p->x - 32,p->y - 32,p->x + 32,p->y + 32 };
			else
				p_rect = { p->x - 19,p->y - 30,p->x + 19,p->y + 30 };

			if (IntersectRect(&temp_rect, &p_rect, &monster_rect))
				return true;

			p = p->next;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Char_Deathcheck(HWND hWnd)    //ĳ���� ü�� 0�ƴ��� Ȯ�� �Լ� ����߻�, ���� ���ݽÿ� �� �ٸ� �����ָ� �ȴٱ�?? ���� ��� üũ �Լ��� ���� ����Ŷ�, Ī��!
{
	if (player.health <= 0)
	{
		current_page = end_page;

		//Ÿ�̸� �༮��.. ��...�� �׿������ھ�!!! ���ӾƾƾƾƤ����ƾƤ�����!!!
		KillTimer(hWnd, rest_time);
		KillTimer(hWnd, move_player);
		KillTimer(hWnd, atk_player);
		KillTimer(hWnd, spawn_itembox);
		KillTimer(hWnd, move_monster);
		KillTimer(hWnd, spawn_itembox);
		KillTimer(hWnd, sht_player);
		monster_head->next = NULL;
		boss_head->next = NULL;

		InvalidateRect(hWnd, NULL, false);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Remaining_bullet_check()  //���� �Ѿ� Ȯ��  �������� false ������ true
{
	if (weapon[selected_weapon].bullet != 0)
	{
		return false;
	}
	else
		return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Crash_check_bullet2object(double x, double y, int* dx, int* dy, double addX, double addY, HWND hWnd)
{
	if (block[(int)y / block_size][(int)x / block_size].isempty == false)
	{
		if (block[(int)y / block_size][(int)x / block_size].iswall)
		{
			block[(int)y / block_size][(int)x / block_size].hp -= weapon[selected_weapon].damage;
			if (block[(int)y / block_size][(int)x / block_size].hp < 0)
			{
				block[(int)y / block_size][(int)x / block_size].iswall = false;
				block[(int)y / block_size][(int)x / block_size].isempty = true;
			}
			*dx = (int)x;
			*dy = (int)y;
			return;
		}
		else if (block[(int)y / block_size][(int)x / block_size].isbarrel)
		{
			/****************************barrel ���� ����*********************************/
			bomb_barrel(x, y, hWnd);
			/****************************************************************************/
			*dx = (int)x;
			*dy = (int)y;
			return;
		}

		else if (block[(int)y / block_size][(int)x / block_size].isinvinciblewall)
		{
			*dx = (int)x;
			*dy = (int)y;
			return;
		}
	}

	else if (Crash_check_bullet2monster(x, y, NULL, monster_head, selected_weapon, hWnd))
	{
		*dx = (int)x;
		*dy = (int)y;
		return;
	}

	if (block[(int)*dy / block_size][(int)*dx / block_size].rect.left <= x && x <= block[(int)*dy / block_size][(int)*dx / block_size].rect.right
		&& block[(int)*dy / block_size][(int)*dx / block_size].rect.top <= y && y <= block[(int)*dy / block_size][(int)*dx / block_size].rect.bottom)
		return;
	if ((int)x < 0 || (int)x > win_x_size * 2 || (int)y < 0 || (int)y > win_y_size * 2)
		return;

	Crash_check_bullet2object(x + addX, y + addY, dx, dy, addX, addY, hWnd);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Crash_check_bullet2monster(int x, int y, CHARACTER* prev, CHARACTER* p, int type, HWND hWnd)
{
	RECT monster_rect = { p->x - 19, p->y - 30, p->x + 19, p->y + 30 };

	if (monster_rect.left <= x && monster_rect.right >= x && monster_rect.top <= y && monster_rect.bottom >= y)
	{
		p->health -= weapon[type].damage;

		if (p->health <= 0 && p != monster_head)
		{
			if (prev != NULL)
			{
				prev->next = p->next;
			}
			free(p);
			score += 10;
			monster_num--;

			if (monster_num == 0)
			{
				SetTimer(hWnd, rest_time, 5000, TimerProc);
			}

		}
		return true;
	}

	if (p->next != NULL)
		return Crash_check_bullet2monster(x, y, p, p->next, type, hWnd);

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void bomb_barrel(int x, int y, HWND hWnd)
{
	if (block[(y / block_size)][(x / block_size) - 1].isempty == true)
	{
		Crash_check_bullet2monster(block[(y / block_size)][(x / block_size) - 1].rect.left + (block_size / 2), block[(y / block_size)][(x / block_size) - 1].rect.top + (block_size / 2), NULL, monster_head, barrel, hWnd);
		RECT rtP, rtTmp;
		int x_crash_check, y_crash_check;
		if (player.isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
		{
			x_crash_check = 32;
			y_crash_check = 32;
		}
		else
		{
			x_crash_check = 19;
			y_crash_check = 30;
		}

		SetRect(&rtP, player.x - x_crash_check, player.y - y_crash_check, player.x + x_crash_check, player.y + y_crash_check);
		if (IntersectRect(&rtTmp, &block[(y / block_size)][(x / block_size) - 1].rect, &rtP))
		{
			player.health -= weapon[barrel].damage;
		}
	}
	if (block[(y / block_size)][(x / block_size) + 1].isempty == true)
	{
		Crash_check_bullet2monster(block[(y / block_size)][(x / block_size) + 1].rect.left + (block_size / 2), block[(y / block_size)][(x / block_size) + 1].rect.top + (block_size / 2), NULL, monster_head, barrel, hWnd);
		RECT rtP, rtTmp;
		int x_crash_check, y_crash_check;
		if (player.isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
		{
			x_crash_check = 32;
			y_crash_check = 32;
		}
		else
		{
			x_crash_check = 19;
			y_crash_check = 30;
		}

		SetRect(&rtP, player.x - x_crash_check, player.y - y_crash_check, player.x + x_crash_check, player.y + y_crash_check);
		if (IntersectRect(&rtTmp, &block[(y / block_size)][(x / block_size) + 1].rect, &rtP))
		{
			player.health -= weapon[barrel].damage;
		}
	}
	if (block[(y / block_size) - 1][(x / block_size)].isempty == true)
	{
		Crash_check_bullet2monster(block[(y / block_size) - 1][(x / block_size)].rect.left + (block_size / 2), block[(y / block_size) - 1][(x / block_size)].rect.top + (block_size / 2), NULL, monster_head, barrel, hWnd);
		RECT rtP, rtTmp;
		int x_crash_check, y_crash_check;
		if (player.isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
		{
			x_crash_check = 32;
			y_crash_check = 32;
		}
		else
		{
			x_crash_check = 19;
			y_crash_check = 30;
		}

		SetRect(&rtP, player.x - x_crash_check, player.y - y_crash_check, player.x + x_crash_check, player.y + y_crash_check);
		if (IntersectRect(&rtTmp, &block[(y / block_size) - 1][(x / block_size)].rect, &rtP))
		{
			player.health -= weapon[barrel].damage;
		}
	}
	if (block[(y / block_size) + 1][(x / block_size)].isempty == true)
	{
		Crash_check_bullet2monster(block[(y / block_size) + 1][(x / block_size)].rect.left + (block_size / 2), block[(y / block_size) + 1][(x / block_size)].rect.top + (block_size / 2), NULL, monster_head, barrel, hWnd);
		RECT rtP, rtTmp;
		int x_crash_check, y_crash_check;
		if (player.isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
		{
			x_crash_check = 32;
			y_crash_check = 32;
		}
		else
		{
			x_crash_check = 19;
			y_crash_check = 30;
		}

		SetRect(&rtP, player.x - x_crash_check, player.y - y_crash_check, player.x + x_crash_check, player.y + y_crash_check);
		if (IntersectRect(&rtTmp, &block[(y / block_size) + 1][(x / block_size)].rect, &rtP))
		{
			player.health -= weapon[barrel].damage;
		}
	}

	if (block[y / block_size][x / block_size].isbarrel == true)
	{
		block[(y / block_size)][(x / block_size)].isempty = true;
		block[(y / block_size)][(x / block_size)].isbarrel = false;
	}

	else if (block[(y / block_size)][(x / block_size)].iswall == true)
	{
		block[(int)y / block_size][(int)x / block_size].hp -= weapon[barrel].damage;
		if (block[(int)y / block_size][(int)x / block_size].hp < 0)
		{
			block[(int)y / block_size][(int)x / block_size].iswall = false;
			block[(int)y / block_size][(int)x / block_size].isempty = true;
		}
		return;
	}

	if (block[(y / block_size)][(x / block_size) - 1].isempty == false)
	{
		bomb_barrel(block[(y / block_size)][(x / block_size) - 1].rect.left + (block_size / 2), block[(y / block_size)][(x / block_size) - 1].rect.top + (block_size / 2), hWnd);
	}
	if (block[(y / block_size)][(x / block_size) + 1].isempty == false)
	{
		bomb_barrel(block[(y / block_size)][(x / block_size) + 1].rect.left + (block_size / 2), block[(y / block_size)][(x / block_size) + 1].rect.top + (block_size / 2), hWnd);
	}
	if (block[(y / block_size) - 1][(x / block_size)].isempty == false)
	{
		bomb_barrel(block[(y / block_size) - 1][(x / block_size)].rect.left + (block_size / 2), block[(y / block_size) - 1][(x / block_size)].rect.top + (block_size / 2), hWnd);
	}
	if (block[(y / block_size) + 1][(x / block_size)].isempty == false)
	{
		bomb_barrel(block[(y / block_size) + 1][(x / block_size)].rect.left + (block_size / 2), block[(y / block_size) + 1][(x / block_size)].rect.top + (block_size / 2), hWnd);
	}
}