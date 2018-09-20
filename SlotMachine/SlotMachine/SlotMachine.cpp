// SlotMachine.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//
// ���Ըӽ�
#include "stdafx.h"
#include "SlotMachine.h"

#define MAX_LOADSTRING 100

// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
HWND g_hWnd, hEdit, hStatic;
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.

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
	hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

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

#define value(n) (560 + (n * 45)) // Rule_Sahow() �Լ��� TextOut()�� ��ġ�� ���մϴ�.
#define SPEED 8 // ���Ըӽ��� Rill�� ȸ�� �ӵ��Դϴ�.
#define Y_SET 20 // ��Ʈ���� ��ġ�� ���ϱ� ���� ��
#define BMP_SIZE 128 // �⺻���� ��Ʈ�ʻ������Դϴ�.

int Money; // �ʱ�ݾ��� �����ϴ� ����
int betting; // ���þ��� �����ϴ� ����
int Text_time = 0; // �ؽ�Ʈ�� �����̴� ���� ǥ���ϱ� ���� ����
int rill[3][9]; // ���Ըӽ��� 3���� ���� �������� �׸��� �����ϱ� ���� ����
int luck[3]; // 3���� ��� �׸��� ������ �����ϱ� ���� ����
int done_num; // Result�� ����ü�� �����ϴµ� 
int prizemoney = 0; // �ش��ϴ� �׸��� �����Ͽ��� ��÷�� �Ǿ��� �� ��÷�ݾ��� �����ϴ� ����
int ran_index = 0; // ���� �Ѱ� �� ���� ������ ���ο� ������ �����Ͽ� ���� ���� ���߱����� ���� ����

BOOL RootMode = FALSE; // Ű �Է��� �����ϱ� ���� ����
TCHAR MoneyStr[256]; // �ʱ�ݾ��� TextOut���� ������ �� ���� ������ �迭
TCHAR BettingStr[256]; // ���ñݾ��� ~~
TCHAR PrizeStr[256];  // ������ ������ MessageBox�� ��÷�� �Ǿ����� ǥ�ø� �ϴµ� �� �� ����� ǥ���ϴ� ������ �迭

enum Game_Main { START, WAIT, ING, END }; // ������ ������¸� ǥ���ϱ� ���� ������
enum Slot_Status { PLAY, STOP }; // ������ ���� ������¸� ��������
enum Result { DO, DONE }; // Rill�� �۾��� �������� �����մϴ�.
enum Rill_Down {ON, OFF}; // Rill���� ȸ���ϱ� ���ؼ� �ʿ��� �������Դϴ�.

POINT location[3]; // 3���� ���� ��ġ�� �����ϱ� ���� ����
POINT size[3]; // 3���� ���� ��Ʈ�ʿ� ���� �� ũ��
POINT memsize[3]; // 3���� ���� memDC�� ���� �� ũ��

HBITMAP hBit[9]; // �׸� ��Ʈ���� ������ ����
HBITMAP titlebit, emptybmp, warningbmp; // ����Ʈ ��Ʈ�ʰ�	
HBRUSH mybrush, oldbrush; 
HFONT hfont, oldfont;
 // �� hdc�Դϴ�. 

struct slot { // ������ 3���� ���� ���¸� �����ϴ� ����ü
	int bmp_num;
	BOOL stop;
	Result result;
};
slot sl[3]; // 3���� Rill�� ������ ������ �ֽ��ϴ�.

Game_Main gm; // ������ ���¸� ��Ÿ���ϴ�.
Slot_Status status = STOP; // ���Ըӽ��� ������¸� ��
Rill_Down rd[3];

void First_Monitor(HDC hdc); // ������ �����Ͽ��� �� ȭ���� �����ݴϴ�.
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
			WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, FALSE); // AdjustWindow�� �ʿ��� ũ�⸦ ����� �Ѵ�. + �������� �޴��� ���õ� ���̰� FALSE�� �ϸ� ����
		SetWindowPos(hWnd, NULL, 0, 0, crt.right - crt.left, crt.bottom - crt.top, // 
			SWP_NOMOVE | SWP_NOZORDER);
		g_hWnd = hWnd;
		
		Money = 0;
		betting = 10000;
		for (int i = 0; i < 9; i++) {
			hBit[i] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1 + i)); // ��Ʈ���� �����´�.
		}
		for (int i = 0; i < 3; i++) {
			luck[i] = -1;
		}

		titlebit = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP13)); // Ÿ��Ʋ ��Ʈ���� ����
		emptybmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP14)); // �̹��� ����	
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
		case 1: // ù ��° ����
			Rill_Move();
			break;
		case 2:  //				+						�ڡڡڡڡڡڡڡڡڡڡ� ���纻 : �ٲ���
			Random_Create(ran_index);
			ran_index++;
			KillTimer(hWnd, 2);
			break;
		case 4: // TextOut 0.5�ʸ��� ������� ��Ÿ���� �ϴ� Ÿ�̸�
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
			if ((gm == WAIT || gm == END) && !RootMode) { // ó���� ������ ��, ������ ��
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
			VARIABLE_PITCH | FF_ROMAN, TEXT("�޸ո���T"));
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

//====================================== �� �� �� �� �� =====================================================

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
			TextOut(hdc, 190, 310, TEXT("�����Ϸ��� ENTERŰ�� �����ּ���."), 20);
		}
		break;
	case WAIT:
		if (Text_time == 1) {
			TextOut(hdc, 190, 310, TEXT("SPACEŰ�� ������ ������ �����մϴ�."), 22);
		}
		break;
	}
	DeleteObject(SelectObject(hdc, oldbrush));
}

void Rill_Setting() {
	srand(GetTickCount());

	int count = 0;

	for (int i = 0; i < 3; i++) { // ��Ʈ���� ���� ����Ѵ�.
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

void initGame() { // ������ �ϱ� ���ؼ� �ʱ�ȭ

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

void Rill_Move() { // ������ �����δ�.

	for (int i = 0; i < 3; i++) {
		if (sl[i].stop == FALSE) { // ������ �� �ִ�.
			if (rill[i][sl[i].bmp_num] != luck[i]) { // ���� �����ִ� �׸��� ����� �׸��� �ƴ� ���

				if (memsize[i].y > 0) { // �������� ����
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
			wsprintf(PrizeStr, TEXT("��÷���� �ʾҽ��ϴ�."), prizemoney);
		else
			wsprintf(PrizeStr, TEXT("��÷���� %d ���Դϴ�."), prizemoney);
		MessageBox(g_hWnd, PrizeStr, TEXT("��÷��"), MB_OK); // �޼����ڽ�

		done_num = 0;
	}
	
	else {
		done_num = 0;
	}
}

BOOL Result_Check() {
	// 3���� 7�� ��
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
		MessageBox(g_hWnd, TEXT("����� �� �ִ� ���� �����մϴ�."), TEXT("���ӽ��� �Ұ�"), MB_OK);
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
	TextOut(hdc, value(3), 10, TEXT("Triple 7  100��"), 15);
	TextOut(hdc, value(3), 60, TEXT("Triple win 50��"), 15);
	TextOut(hdc, value(3), 110, TEXT("Triple any 20��"), 15);
	TextOut(hdc, value(3), 160, TEXT("Double 7  10��"), 14);
	TextOut(hdc, value(3), 210, TEXT("Double win 7��"), 14);
	TextOut(hdc, value(3), 260, TEXT("Double any 3��"), 14);
	TextOut(hdc, value(3), 310, TEXT("Single    7 1��"), 15);
	TextOut(hdc, value(3), 360, TEXT("Single win 0.5��"), 16);
	wsprintf(MoneyStr, TEXT("���ӱݾ� : %d ��     "), Money);
	TextOut(hdc, 100, 380, MoneyStr, lstrlen(MoneyStr));
	wsprintf(BettingStr, TEXT("���ñݾ� : %d ��     "), betting);
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
		BitBlt(hdc, x, y, bx, size, MemDC, vx, vy, SRCCOPY); // 64���� �����ؼ� 0> 0���� �ǰ� by�� �پ��� y�� ����
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