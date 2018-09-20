// SlotMachine.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//
// 슬롯머신
#include "stdafx.h"
#include "SlotMachine.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
HWND g_hWnd, hEdit, hStatic;
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    Dialog_WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Warning_Dialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL	CALLBACK	MoneySet_DlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_	 LPWSTR    lpCmdLine,
	_In_	 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_SLOTMACHINE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SLOTMACHINE));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 200, 0));
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SLOTMACHINE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindow(szWindowClass, szTitle,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
		WS_BORDER | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
		NULL, (HMENU)NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, Warning_Dialog) == IDOK) {
		UpdateWindow(hWnd);
		ShowWindow(hWnd, nCmdShow);
		PlaySound(MAKEINTRESOURCE(IDR_START), NULL, SND_RESOURCE | SND_ASYNC | SND_LOOP);
	}

	return TRUE;
}

#define value(n) (560 + (n * 45)) // Rule_Sahow() 함수의 TextOut()의 위치를 정합니다.
#define SPEED 8 // 슬롯머신의 Rill의 회전 속도입니다.
#define Y_SET 20 // 비트맵의 위치를 정하기 위한 값
#define BMP_SIZE 128 // 기본적인 비트맵사이즈입니다.

int Money; // 초기금액의 저자하는 변수
int betting; // 베팅액을 저정하는 변수
int Text_time = 0; // 텍스트가 깜빡이는 것을 표현하기 위한 변수
int rill[3][9]; // 슬롯머신의 3개의 릴에 랜덤으로 그림을 저장하기 위한 변수
int luck[3]; // 3개의 행운 그림의 난수를 저장하기 위한 변수
int done_num; // Result가 구조체에 존재하는데 
int prizemoney = 0; // 해당하는 그림에 대조하오ㅕ 당첨이 되었을 때 당첨금액을 저장하는 변수
int ran_index = 0; // 릴이 한개 씩 멈출 때마다 새로운 난수를 생성하여 다음 릴을 멈추기위해 쓰는 변수

BOOL RootMode = FALSE; // 키 입력을 제어하기 위한 변수
TCHAR MoneyStr[256]; // 초기금액을 TextOut으로 보여줄 때 쓰는 문자형 배열
TCHAR BettingStr[256]; // 베팅금액을 ~~
TCHAR PrizeStr[256];  // 게임이 끝나면 MessageBox에 당첨이 되었는지 표시를 하는데 그 때 상금을 표시하는 문자형 배열

enum Game_Main { START, WAIT, ING, END }; // 게임의 진행상태를 표현하기 위한 열거형
enum Slot_Status { PLAY, STOP }; // 슬롯의 릴의 진행상태를 보기위함
enum Result { DO, DONE }; // Rill의 작업이 끝났는지 결정합니다.
enum Rill_Down {ON, OFF}; // Rill들이 회전하기 위해서 필요한 열거형입니다.

POINT location[3]; // 3개의 릴의 위치를 저장하기 위한 변수
POINT size[3]; // 3개의 릴이 비트맵에 찍힐 때 크기
POINT memsize[3]; // 3개의 릴이 memDC에 찍힐 때 크기

HBITMAP hBit[9]; // 그림 비트맵을 가지고 있음
HBITMAP titlebit, emptybmp, warningbmp; // 바이트 비트맵과	
HBRUSH mybrush, oldbrush; 
HFONT hfont, oldfont;
 // 주 hdc입니다. 

struct slot { // 슬롯의 3개의 릴의 상태를 저장하는 구조체
	int bmp_num;
	BOOL stop;
	Result result;
};
slot sl[3]; // 3개의 Rill의 정보를 가지고 있습니다.

Game_Main gm; // 게임의 상태를 나타냅니다.
Slot_Status status = STOP; // 슬롯머신의 진행상태를 나
Rill_Down rd[3];

void First_Monitor(HDC hdc); // 게임을 시작하였을 때 화면을 보여줍니다.
void initGame();
void StartGame(HDC hdc);
void DrawBitmap(HDC hdc, int x, int y, int size, int vx, int vy, int DrawType, HBITMAP hBit);
void Rill_Move();
void Random_Create(int i);
void Test_End();
void Rule_Show(HDC hdc);
BOOL Bankruptcy(int money);
void Rill_Setting();
BOOL Result_Check();


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	PAINTSTRUCT ps;
	RECT crt;	
	HDC hdc;
	int wmId;

	switch (message) {
	case WM_CREATE: {
		SetRect(&crt, 0, 0, 64 * 14, 64 * 7);
		AdjustWindowRect(&crt, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
			WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, FALSE); // AdjustWindow는 필요한 크기를 계산을 한다. + 마지막은 메뉴에 관련된 것이고 FALSE를 하면 없다
		SetWindowPos(hWnd, NULL, 0, 0, crt.right - crt.left, crt.bottom - crt.top, // 
			SWP_NOMOVE | SWP_NOZORDER);
		g_hWnd = hWnd;
		
		Money = 0;
		betting = 10000;
		for (int i = 0; i < 9; i++) {
			hBit[i] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1 + i)); // 비트맵을 가져온다.
		}
		for (int i = 0; i < 3; i++) {
			luck[i] = -1;
		}

		titlebit = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP13)); // 타이틀 비트맵을 저장
		emptybmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP14)); // 이미지 저장	
		warningbmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP20));

		srand(GetTickCount());
		SetTimer(hWnd, 4, 500, NULL);
		gm = START;
		Rill_Setting();
		initGame();
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
					//====================== TIMER =============================================
	case WM_TIMER:
		switch (wParam) {
		case 1: // 첫 번째 슬롯
			Rill_Move();
			break;
		case 2:  //				+						★★★★★★★★★★★ 복사본 : 바뀐점
			Random_Create(ran_index);
			ran_index++;
			KillTimer(hWnd, 2);
			break;
		case 4: // TextOut 0.5초마다 사라졌다 나타나게 하는 타이머
			if (Text_time == 0)
				Text_time = 1;
			else if (Text_time == 1)
				Text_time = 0;
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case 10:
			KillTimer(hWnd, 10);
			PlaySound(MAKEINTRESOURCE(IDR_RUN), NULL, SND_RESOURCE | SND_ASYNC | SND_LOOP);
			SetTimer(g_hWnd, 1, 10, NULL);
			break;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
		//====================== KEYDOWN ==========================================
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			if (gm == START) {
				gm = WAIT;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;
		case VK_SPACE:
			if ((gm == WAIT || gm == END) && !RootMode) { // 처음에 시작할 때, 끝났을 때
				if (!Bankruptcy(Money)) {
					KillTimer(hWnd, 4);
					initGame();
					gm = ING;
					RootMode = TRUE;
					Money -= betting;
					PlaySound(MAKEINTRESOURCE(IDR_COIN2), NULL, SND_RESOURCE | SND_ASYNC);
					SetTimer(hWnd, 10, 500, NULL);
					InvalidateRect(hWnd, NULL, FALSE);
				}
			}
			break;
		}
		break;
		//====================== PAINT =============================================
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		hfont = CreateFont(20, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0,
			VARIABLE_PITCH | FF_ROMAN, TEXT("휴먼모음T"));
		oldfont = (HFONT)SelectObject(hdc, hfont);
		SetBkColor(hdc, RGB(255, 0, 0));

		if (status == STOP) {
			Rule_Show(hdc);
		}
		switch (gm) {
		case START:
			First_Monitor(hdc);
			break;
		case WAIT:
			First_Monitor(hdc);
			break;
		case ING:
			StartGame(hdc);
			if (status == STOP)
				status = PLAY;
			break;
		case END:
			StartGame(hdc);
			break;
		}
		
		SelectObject(hdc, oldfont);
		DeleteObject(hfont);
		DrawBitmap(hdc, 80, 10, 90, 0, 0, 1, titlebit);
		EndPaint(hWnd, &ps);
		break;
		//==========================================================================
	case WM_COMMAND: {
		wmId = LOWORD(wParam);
		switch (wmId) {
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_Money_setting:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, MoneySet_DlgProc);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_5000:
			betting = 5000;
			break;
		case ID_10000:
			betting = 10000;
			break;
		case ID_30000:
			betting = 30000;
			break;
		case ID_50000:
			betting = 50000;
			break;
		case ID_100000:
			betting = 100000;
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//====================================== 사 용 자 함 수 =====================================================

void First_Monitor(HDC hdc) {
	mybrush = (HBRUSH)CreateSolidBrush(RGB(255, 0, 0));
	oldbrush = (HBRUSH)SelectObject(hdc, mybrush);
	Rectangle(hdc, 90, 100, 554, 340);

	DrawBitmap(hdc, 80, 10, 90, 0, 0, 1, titlebit);
	for (int i = 0; i < 3; i++) {
		DrawBitmap(hdc, location[i].x + (i * 10), Y_SET * SPEED, 128, 0, 0, 1, hBit[rill[i][sl[i].bmp_num]]);
	}

	switch (gm) {
	case START:
		if (Text_time == 1) {
			TextOut(hdc, 190, 310, TEXT("시작하려면 ENTER키를 눌러주세요."), 20);
		}
		break;
	case WAIT:
		if (Text_time == 1) {
			TextOut(hdc, 190, 310, TEXT("SPACE키를 누르면 게임을 시작합니다."), 22);
		}
		break;
	}
	DeleteObject(SelectObject(hdc, oldbrush));
}

void Rill_Setting() {
	srand(GetTickCount());

	int count = 0;

	for (int i = 0; i < 3; i++) { // 비트맵이 뭔지 기억한다.
		for (int j = 0; j < 9; j++) {
			rill[i][j] = rand() % 9;
			for (int k = 0; k < j; k++) {
				if (rill[i][j] == rill[i][k]) {
					rill[i][j] = rand() % 9;
					j--;
				}
			}
		}
	}
}

void Random_Create(int i) {
	luck[i] = rand() % 40;
	int k;

	for (k = 0; k < 7; k++) {
		if (luck[i] / 5 == k) {
			luck[i] = k;
			break;
		}
	}
	if (k == 7 && luck[i] % 5 < 3)
		luck[i] = 7;
	else if (k == 7 && luck[i] % 5 >= 3)
		luck[i] = 8;

	if (i == 0) {
		RootMode = FALSE;
	}
}

void initGame() { // 게임을 하기 위해서 초기화

	prizemoney = 0;
	done_num = 0;

	for (int i = 0; i < 3; i++) {
		sl[i].stop = FALSE;
		if (gm == END || gm == WAIT)
		if (sl[i].bmp_num == 9) {
			sl[i].bmp_num = 0;
		}
		sl[i].result = DO;

		location[i] = { i * BMP_SIZE + 120, Y_SET * SPEED };
		size[i] = { BMP_SIZE, BMP_SIZE };
		memsize[i] = { 0, 0 };

		rd[i] = ON;
	}
	if(gm == WAIT || gm == END) 
		SetTimer(g_hWnd, 2, 1000, NULL);

	InvalidateRect(g_hWnd, NULL, FALSE);
}

void Rill_Move() { // 슬롯을 움직인다.

	for (int i = 0; i < 3; i++) {
		if (sl[i].stop == FALSE) { // 움직일 수 있다.
			if (rill[i][sl[i].bmp_num] != luck[i]) { // 지금 돌고있는 그림이 행운의 그림이 아닐 경우

				if (memsize[i].y > 0) { // 내려가는 상태
					memsize[i].y -= SPEED;
				}
				else if (memsize[i].y == 0) {
					size[i].y -= SPEED;
					location[i].y += SPEED;
					if (location[i].y == (Y_SET * SPEED + BMP_SIZE)) {
						memsize[i].y = BMP_SIZE;
						size[i].y = BMP_SIZE;
						location[i].y = Y_SET * SPEED;
						sl[i].bmp_num++;
						if (sl[i].bmp_num == 9)
							sl[i].bmp_num = 0;
					}
				}
			}
			else if (rill[i][sl[i].bmp_num] == luck[i]) {
				if (memsize[i].y > 0)
					memsize[i].y -= SPEED;
				else if (memsize[i].y == 0) {
					size[i].y = BMP_SIZE;
					location[i].y = Y_SET * SPEED;
					sl[i].stop = TRUE;
					sl[i].result = DONE;

					if (ran_index < 3)
						SetTimer(g_hWnd, 2, 500, NULL);
				}
			}
		}
	}
	Test_End();
}

void Test_End() {
	for (int i = 0; i < 3; i++) {
		if (sl[i].result == DONE)
			done_num++;
	}
	if (done_num == 3) {
		PlaySound(MAKEINTRESOURCE(IDR_COIN), NULL, SND_RESOURCE | SND_ASYNC);
		KillTimer(g_hWnd, 1);
		ran_index = 0;
		gm = END;
		RootMode = FALSE;
		Result_Check();
		status = STOP;
		for (int i = 0; i < 3; i++) {
			luck[i] = -1;
		}

		Money += prizemoney;
		if (prizemoney == 0)
			wsprintf(PrizeStr, TEXT("당첨되지 않았습니다."), prizemoney);
		else
			wsprintf(PrizeStr, TEXT("당첨금은 %d 원입니다."), prizemoney);
		MessageBox(g_hWnd, PrizeStr, TEXT("당첨금"), MB_OK); // 메세지박스

		done_num = 0;
	}
	
	else {
		done_num = 0;
	}
}

BOOL Result_Check() {
	// 3개가 7일 때
	int same_num = 0;
	int tmp_num = 0;
	int compare = 0;

	if (done_num == 3) {
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 3; j++) {
				if (rill[j][sl[j].bmp_num] == i) {
					tmp_num++;
				}
			}
			if (same_num < tmp_num) {
				same_num = tmp_num;
			}
			tmp_num = 0;
		}
	}
	else
		return 0;
	switch (same_num) {
	case 3:
		if (rill[0][sl[0].bmp_num] == 8) {
			prizemoney = betting * 100;
		}
		else if (rill[0][sl[0].bmp_num] == 7) {
			prizemoney = betting * 50;
		}
		else {
			prizemoney = betting * 20;
		}
		PlaySound(MAKEINTRESOURCE(IDR_NICE), NULL, SND_RESOURCE | SND_ASYNC);
		break;
	case 2:
		if (rill[0][sl[0].bmp_num] == rill[1][sl[1].bmp_num]) {
			if (rill[0][sl[0].bmp_num] == 8) {
				prizemoney = betting * 10;
			}
			else if (rill[0][sl[0].bmp_num] == 7) {
				prizemoney = betting * 7;
			}
			else {
				prizemoney = betting * 3;
			}
			PlaySound(MAKEINTRESOURCE(IDR_NICE), NULL, SND_RESOURCE | SND_ASYNC);
			return 0;
		}
		else if (rill[0][sl[0].bmp_num] == rill[2][sl[2].bmp_num]) {
			if (rill[0][sl[0].bmp_num] == 8) {
				prizemoney = betting * 10;
			}
			else if (rill[0][sl[0].bmp_num] == 7) {
				prizemoney = betting * 7;
			}
			else {
				prizemoney = betting * 3;
			}
			PlaySound(MAKEINTRESOURCE(IDR_NICE), NULL, SND_RESOURCE | SND_ASYNC);
			return 0;
		}
		else if (rill[1][sl[1].bmp_num] == rill[2][sl[2].bmp_num]) {
			if (rill[1][sl[1].bmp_num] == 8) {
				prizemoney = betting * 10;
			}
			else if (rill[1][sl[1].bmp_num] == 7) {
				prizemoney = betting * 7;
			}
			else {
				prizemoney = betting * 3;
			}
			PlaySound(MAKEINTRESOURCE(IDR_NICE), NULL, SND_RESOURCE | SND_ASYNC);
			return 0;
		}
		break;
	case 1:
		for (int i = 0; i < 3; i++) {
			if (rill[i][sl[i].bmp_num] == 8) {
				if (compare < betting * 1) {
					compare = betting * 1;
				}
			}
			else if (rill[i][sl[i].bmp_num] == 7) {
				if (compare < betting / 2) {
					compare = betting / 2;
				}
			}
		}
		prizemoney = compare;
		break;
	}
	return 0;
}

BOOL Bankruptcy(int money) {
	if (money < betting) {
		MessageBox(g_hWnd, TEXT("사용할 수 있는 돈이 부족합니다."), TEXT("게임시작 불가"), MB_OK);
		return TRUE;
	}
	return FALSE;
}

void Rule_Show(HDC hdc) {
	for (int i = 0; i < 3; i++) {
		DrawBitmap(hdc, value(i), 0, 0, 0, 0, 2, hBit[8]);
	}
	for (int i = 0; i < 3; i++) {
		DrawBitmap(hdc, value(i), 50, 0, 0, 0, 2, hBit[7]);
	}
	for (int i = 0; i < 3; i++) {
		DrawBitmap(hdc, value(i), 100, 0, 0, 0, 2, emptybmp);
	}
	for (int i = 0; i < 2; i++) {
		DrawBitmap(hdc, value(i), 150, 0, 0, 0, 2, hBit[8]);
	}
	for (int i = 0; i < 2; i++) {
		DrawBitmap(hdc, value(i), 200, 0, 0, 0, 2, hBit[7]);
	}
	for (int i = 0; i < 2; i++)
		DrawBitmap(hdc, value(i), 250, 0, 0, 0, 2, emptybmp);

	DrawBitmap(hdc, value(0), 300, 0, 0, 0, 2, hBit[8]);
	DrawBitmap(hdc, value(0), 350, 0, 0, 0, 2, hBit[7]);

	SetBkColor(hdc, RGB(0, 200, 0));
	TextOut(hdc, value(3), 10, TEXT("Triple 7  100배"), 15);
	TextOut(hdc, value(3), 60, TEXT("Triple win 50배"), 15);
	TextOut(hdc, value(3), 110, TEXT("Triple any 20배"), 15);
	TextOut(hdc, value(3), 160, TEXT("Double 7  10배"), 14);
	TextOut(hdc, value(3), 210, TEXT("Double win 7배"), 14);
	TextOut(hdc, value(3), 260, TEXT("Double any 3배"), 14);
	TextOut(hdc, value(3), 310, TEXT("Single    7 1배"), 15);
	TextOut(hdc, value(3), 360, TEXT("Single win 0.5배"), 16);
	wsprintf(MoneyStr, TEXT("게임금액 : %d 원     "), Money);
	TextOut(hdc, 100, 380, MoneyStr, lstrlen(MoneyStr));
	wsprintf(BettingStr, TEXT("베팅금액 : %d 원     "), betting);
	TextOut(hdc, 300, 380, BettingStr, lstrlen(BettingStr));
}

void StartGame(HDC hdc) {

	for (int i = 0; i < 3; i++) {
		DrawBitmap(hdc, location[i].x + (i * 10), location[i].y, size[i].y, 0, memsize[i].y, 1, hBit[rill[i][sl[i].bmp_num]]);
	}

	SetBkColor(hdc, RGB(255, 0, 0));
	if (gm == END)
		TextOut(hdc, 190, 310, TEXT("SPACE Key is Start            "), 30);
	else if (gm == ING)
		TextOut(hdc, 190, 310, TEXT("                              "), 30);

}

void DrawBitmap(HDC hdc, int x, int y, int size, int vx, int vy, int DrawType, HBITMAP hBit)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	int bx, by;
	BITMAP bit;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	switch (DrawType) {
	case 1:
		BitBlt(hdc, x, y, bx, size, MemDC, vx, vy, SRCCOPY); // 64부터 시작해서 0> 0으로 되고 by가 줄어들면 y는 증가
		break;
	case 2:
		StretchBlt(hdc, x, y, 40, 40, MemDC, 0, 0, bx, by, SRCCOPY);
		break;
	}

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}

BOOL CALLBACK Warning_Dialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hDlg, IDOK);
			return TRUE;
		case IDCANCEL:
			DestroyWindow(g_hWnd);
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


BOOL CALLBACK MoneySet_DlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage) {
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			Money += GetDlgItemInt(hDlg, IDC_EDIT, NULL, FALSE);
			EndDialog(hDlg, IDOK);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}