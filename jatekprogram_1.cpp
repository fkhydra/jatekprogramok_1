#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib, "d2d1")

ID2D1Factory* pD2DFactory = NULL;
ID2D1HwndRenderTarget* pRT = NULL;

/*globalai valtozok*/
#define HIBA_00 TEXT("Error:Program initialisation process.")
#define IDC_STATIC -1
#define WINSTYLE_DIALOG (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU)

HINSTANCE hInstGlob;
int SajatiCmdShow;
POINT MousePos;

HWND Form1;
LRESULT CALLBACK WndProc0(HWND, UINT, WPARAM, LPARAM);
char szClassName[] = "WindowsApp";

//*******************
//a játék változói
//*******************
int elso_lepes;
int lepeskoz;
int offset;
int tabla[10][10];
POINT pontok;
int gyozelem;//0-senki,1-játékos,2-CPU,3-döntetlen

void init();
void hatter_kirajzol();
void kattintas(POINT mouse);
void bejeloles(int x, int y, int azonosito);
void tabla_kirajzol();
int kereses_gyozelem(int azonosito, int max_num);
int keres_fuggatlo_felfele(int azonosito, int max_num);
int keres_fuggatlo_lefele(int azonosito, int max_num);
int keres_fugg(int azonosito, int max_num);
int keres_horiz(int azonosito, int max_num);
int get_random_free_cells();
int keres_szabad_horiz(int azonosito, int max_num);
int keres_szabad_fugg(int azonosito, int max_num);
int keres_szabad_fuggatlo_lefele(int azonosito, int max_num);
int keres_szabad_fuggatlo_felfele(int azonosito, int max_num);
int kiegeszit_felkesz(int azonosito, int max_num, int tipus);
int gyors_ellenorzes(int azonosito, int max_num);
int MI_tamad();
int MI_blokkol();
void MI_kovetkezo_lepes();
int ellenoriz_dontetlen();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("StdWinClassName");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass0;
	SajatiCmdShow = iCmdShow;
	hInstGlob = hInstance;

	wndclass0.style = CS_HREDRAW | CS_VREDRAW;
	wndclass0.lpfnWndProc = WndProc0;
	wndclass0.cbClsExtra = 0;
	wndclass0.cbWndExtra = 0;
	wndclass0.hInstance = hInstance;
	wndclass0.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass0.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass0.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndclass0.lpszMenuName = NULL;
	wndclass0.lpszClassName = TEXT("WIN0");

	if (!RegisterClass(&wndclass0))
	{
		MessageBox(NULL, HIBA_00, TEXT("Program Start"), MB_ICONERROR); return 0;
	}

	Form1 = CreateWindow(TEXT("WIN0"),
		TEXT("Amõba by Krisztián Fehér"),
		WINSTYLE_DIALOG,
		50,
		50,
		500,
		500,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(Form1, SajatiCmdShow);
	UpdateWindow(Form1);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc0(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
		pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(
				hwnd, D2D1::SizeU(500, 500)),
			&pRT);
		init();
		tabla_kirajzol();
		return 0;
		//*********************************
		//Képernyõvillogás elkerülése
		//*********************************
	case WM_ERASEBKGND:
		return (LRESULT)1;
	case WM_NOTIFY: {
		return 0; }
	case WM_LBUTTONDOWN:
		//*******************
		//felhasználói kattintás
		//*******************
		MousePos.x = LOWORD(lParam);
		MousePos.y = HIWORD(lParam);
		kattintas(MousePos);
		return 0;
	case WM_SIZE:
		return 0;
	case WM_PAINT:
		if (elso_lepes == 0) tabla_kirajzol();
		return 0;
	case WM_CHAR:
		//*******************
		//játék újraindítása
		//*******************
		switch (wParam)
		{
		case 27:// 'Esc' billentyû
			init();
			tabla_kirajzol();
			return 0;
		}
	case WM_CLOSE:
		pRT->Release();
		pD2DFactory->Release();
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

//*******************
//a játék inicializálása
//*******************
void init()
{
	int i, j;
	gyozelem = 0;
	offset = 10;
	lepeskoz = 46;
	elso_lepes = 0;
	for (i = 0; i < 10; ++i)
		for (j = 0; j < 10; ++j)
		{
			tabla[i][j] = 0;
		}
	SetWindowTextA(Form1, "Amõba by Krisztián Fehér!");
}

//*******************
//a négyzetháló kirajzolása
//*******************
void hatter_kirajzol()
{
	int i;
	ID2D1SolidColorBrush* hBrush;

	pRT->BeginDraw();
	pRT->Clear(D2D1::ColorF(D2D1::ColorF(0.8F, 0.8F, 0.8F, 1.0F)));
	pRT->CreateSolidColorBrush(D2D1::ColorF(0.0F, 0.0F, 0.0F, 1.0F), &hBrush);

	if (hBrush != NULL)
	{
		for (i = 0; i < 11; ++i)
		{
			pRT->DrawLine(D2D1::Point2F(i * lepeskoz + offset, offset),
				D2D1::Point2F(i * lepeskoz + offset, lepeskoz * 10 + offset),
				hBrush,
				4.0f);
		}
		for (i = 0; i < 11; ++i)
		{
			pRT->DrawLine(D2D1::Point2F(offset, i * lepeskoz + offset),
				D2D1::Point2F(lepeskoz * 10 + offset, i * lepeskoz + offset),
				hBrush,
				4.0f);
		}
	}

	pRT->EndDraw();
}

//*******************
//a játékos kattintásának kezelése
//*******************
void kattintas(POINT mouse)
{
	if (gyozelem > 0) return;
	//int x_adat = round(float((float)mouse.x / (float)lepeskoz));
	//int y_adat = round(float((float)mouse.y / (float)lepeskoz));
	int x_adat = (mouse.x / lepeskoz);
	int y_adat = (mouse.y / lepeskoz);

	if (x_adat < 10 && y_adat < 10)
	{
		if (tabla[y_adat][x_adat] > 0) return;
		else
		{
			bejeloles(x_adat, y_adat, 1);
			if (kereses_gyozelem(1, 5) == 1)
			{
				SetWindowTextA(Form1, "Ön nyert!");
				gyozelem = 1;
				return;
			}
		}
	}
	MI_kovetkezo_lepes();
}

//*******************
//a játékos vagy a CPU lépésének érvényesítése
//*******************
void bejeloles(int x, int y, int azonosito)
{
	tabla[y][x] = azonosito;
	tabla_kirajzol();
}

//*******************
//a bejelölt cellák kirajzolása
//*******************
void tabla_kirajzol()
{
	int x;
	int y;
	int kor_szelesseg;

	hatter_kirajzol();
	kor_szelesseg = lepeskoz / 2 - offset;

	ID2D1SolidColorBrush* hBrush1;
	pRT->BeginDraw();
	pRT->CreateSolidColorBrush(D2D1::ColorF(0.13F, 0.8F, 0.8F, 1.0F), &hBrush1);
	D2D1_ELLIPSE kor = D2D1::Ellipse(D2D1::Point2F(0, 0), kor_szelesseg, kor_szelesseg);

	for (y = 0; y < 10; ++y)
		for (x = 0; x < 10; ++x)
		{
			kor.point.x = x * lepeskoz + offset + (lepeskoz / 2);
			kor.point.y = y * lepeskoz + offset + (lepeskoz / 2);

			if (tabla[y][x] > 0)
			{
				if (tabla[y][x] == 1) hBrush1->SetColor(D2D1::ColorF(0.13F, 0.8F, 0.8F, 1.0F));
				else if (tabla[y][x] == 2) hBrush1->SetColor(D2D1::ColorF(0.8F, 0.8F, 0.13F, 1.0F));

				pRT->FillEllipse(kor, hBrush1);

				hBrush1->SetColor(D2D1::ColorF(0.0F, 0.0F, 0.0F, 1.0F));
				pRT->DrawEllipse(kor, hBrush1, 3.0f);
			}
		}

	pRT->EndDraw();
}

//*******************
//vízszintes irányú, adott számú bejelölés ellenõrzése
//*******************
int keres_horiz(int azonosito, int max_num)
{
	int x;
	int y;
	int x2;
	int szamlalo;

	for (y = 0; y < 10; ++y)
		for (x = 0; x < 10; ++x)
		{
			if (tabla[y][x] == azonosito)
			{
				pontok.x = x;
				pontok.y = y;
				szamlalo = 0;									for (x2 = x; x2 < 10; ++x2)
				{
					if (tabla[y][x2] == azonosito) ++szamlalo;
					else break;
					if (szamlalo == max_num) return 1;
				}
			}
		}
	return -1;
}

//*******************
//függõleges irányú, adott számú bejelölés ellenõrzése
//*******************
int keres_fugg(int azonosito, int max_num)
{
	int y;
	int x;
	int y2;
	int szamlalo;

	for (x = 0; x < 10; ++x)
		for (y = 0; y < 10; ++y)
		{
			if (tabla[y][x] == azonosito)//itt csere
			{
				pontok.x = x;
				pontok.y = y;
				szamlalo = 0;
				for (y2 = y; y2 < 10; ++y2)
				{
					if (tabla[y2][x] == azonosito) ++szamlalo;
					else break;
					if (szamlalo == max_num) return 1;
				}
			}
		}
	return -1;
}

//*******************
//átlós irányú, adott számú bejelölés ellenõrzése
//*******************
int keres_fuggatlo_lefele(int azonosito, int max_num)
{
	int y;
	int x;
	int y2;
	int x2;
	int szamlalo;

	for (x = 0; x < 10; ++x)
		for (y = 0; y < 10; ++y)
		{
			if (tabla[y][x] == azonosito)//itt csere
			{
				pontok.x = x;
				pontok.y = y;
				x2 = x;
				szamlalo = 0;
				for (y2 = y; y2 < 10; ++y2, ++x2)
				{
					if (tabla[y2][x2] == azonosito) ++szamlalo;
					else break;
					if (szamlalo == max_num) return 1;
				}
			}
		}
	return -1;
}

//*******************
//átlós irányú, adott számú bejelölés ellenõrzése
//*******************
int keres_fuggatlo_felfele(int azonosito, int max_num)
{
	int y;
	int x;
	int y2;
	int x2;
	int szamlalo;

	for (x = 0; x < 10; ++x)
		for (y = 0; y < 10; ++y)
		{
			if (tabla[y][x] == azonosito)//itt csere
			{
				pontok.x = x;
				pontok.y = y;
				x2 = x;
				szamlalo = 0;
				for (y2 = y; y2 >= 0; --y2, ++x2)
				{
					if (tabla[y2][x2] == azonosito) ++szamlalo;
					else break;
					if (szamlalo == max_num) return 1;
				}
			}
		}
	return -1;
}

//*******************
//gyõzelmi felétételek ellenõrzése
//*******************
int kereses_gyozelem(int azonosito, int max_num)
{
	if (keres_horiz(azonosito, max_num) == 1) return 1;
	if (keres_fugg(azonosito, max_num) == 1) return 1;
	if (keres_fuggatlo_lefele(azonosito, max_num) == 1) return 1;
	if (keres_fuggatlo_felfele(azonosito, max_num) == 1) return 1;
	return -1;
}

//**************************
//mesterséges intelligencia
//**************************

//*******************
//MI egy szabad cella keresése
//*******************
int get_random_free_cells()
{
	int i;
	int j = 0;//jelzi, ha nincs üres cella
	int x = 0;
	int y = 0;

	for (y = 0; y < 10; ++y)
		for (y = 0; y < 10; ++y)
		{
			if (tabla[y][x] == 0)
			{
				pontok.x = x;
				pontok.y = y;
				j = 1;
			}
		}

	if (j == 0) return -1;

	x = rand() % 9;
	y = rand() % 9;
	pontok.x = x;
	pontok.y = y;

	while (tabla[x][y] != 0)
	{
		x = rand() % 9;
		y = rand() % 9;
		pontok.x = x;
		pontok.y = y;
	}
	return 1;
}

//*******************
//MI vízszintes szabad cella keresés
//*******************
int keres_szabad_horiz(int azonosito, int max_num)
{
	int x;
	int y;
	int x2;
	int szamlalo;

	for (y = 0; y < 10; ++y)
		for (x = 0; x < 10; ++x)
		{
			if (tabla[y][x] == azonosito)
			{
				szamlalo = 0;									
				for (x2 = x; x2 < 10; ++x2)
				{
					if (szamlalo == max_num) break;//felesleges körök kiiktatása
					if (tabla[y][x2] == azonosito) ++szamlalo;
					else break;
					if (szamlalo == max_num)
					{
						if (x > 0)
						{
							if (tabla[y][x - 1] == 0)
							{
								pontok.x = x;
								pontok.y = y;
								return 1;
							}
						}
						if (x < (10 - max_num))
						{
							if (tabla[y][x + max_num] == 0)
							{
								pontok.x = x;
								pontok.y = y;
								return 2;
							}
						}
					}
				}
			}
		}
	return -1;
}

//*******************
//MI függõleges szabad cella keresés
//*******************
int keres_szabad_fugg(int azonosito, int max_num)
{
	int y;
	int x;
	int y2;
	int szamlalo;

	for (x = 0; x < 10; ++x)
		for (y = 0; y < 10; ++y)
		{
			if (tabla[y][x] == azonosito)
			{
				szamlalo = 0;
				for (y2 = y; y2 < 10; ++y2)
				{
					if (szamlalo == max_num) break;
					if (tabla[y2][x] == azonosito) ++szamlalo;
					else break;
					if (szamlalo == max_num)
					{
						if (y > 0)
						{
							if (tabla[y - 1][x] == 0)
							{
								pontok.x = x;
								pontok.y = y;
								return 1;
							}
						}
						if (y < (10 - max_num))
						{
							if (tabla[y + max_num][x] == 0)
							{
								pontok.x = x;
								pontok.y = y;
								return 2;
							}
						}
					}
				}
			}
		}
	return -1;
}

//*******************
//MI átlós szabad cella keresés
//*******************
int keres_szabad_fuggatlo_lefele(int azonosito, int max_num)
{
	int y;
	int x;
	int y2;
	int x2;
	int szamlalo;

	for (x = 0; x < 10; ++x)
		for (y = 0; y < 10; ++y)
		{
			if (tabla[y][x] == azonosito)
			{
				x2 = x;
				szamlalo = 0;
				for (y2 = y; y2 < 10; ++y2, ++x2)
				{
					if (szamlalo == max_num) break;
					if (tabla[y2][x2] == azonosito) ++szamlalo;
					else break;
					if (szamlalo == max_num)
					{
						if ((y > 0) && (x > 0))
						{
							if (tabla[y - 1][x - 1] == 0)
							{
								pontok.x = x;
								pontok.y = y;
								return 1;
							}
						}
						if ((y < (10 - max_num)) && (x < (10 - max_num)))
						{
							if (tabla[y + max_num][x + max_num] == 0)
							{
								pontok.x = x;
								pontok.y = y;
								return 2;
							}
						}
					}
				}
			}
		}
	return -1;
}

//*******************
//MI átlós szabad cella keresés
//*******************
int keres_szabad_fuggatlo_felfele(int azonosito, int max_num)
{
	int y;
	int x;
	int y2;
	int x2;
	int szamlalo;

	for (x = 0; x < 10; ++x)
		for (y = 0; y < 10; ++y)
		{
			if (tabla[y][x] == azonosito)
			{
				x2 = x;
				szamlalo = 0;
				for (y2 = y; y2 >= 0; --y2, ++x2)
				{
					if (szamlalo == max_num) break;
					if (tabla[y2][x2] == azonosito) ++szamlalo;
					else break;
					if (szamlalo == max_num)
					{
						if ((y > (max_num - 1)) && (x < (10 - max_num)))
						{
							if (tabla[y - max_num][x + max_num] == 0)
							{
								pontok.x = x;
								pontok.y = y;
								return 1;
							}
						}
						if ((y < 9) && (x > 0))
						{
							if (tabla[y + 1][x - 1] == 0)
							{
								pontok.x = x;
								pontok.y = y;
								return 2;
							}
						}
					}
				}
			}
		}
	return -1;
}

//*******************
//MI építkezési lépés
//*******************
int kiegeszit_felkesz(int azonosito, int max_num, int tipus)
{
	int eredmeny;

	if (tipus == 1)
	{
		eredmeny = keres_szabad_horiz(azonosito, max_num);
		if (eredmeny == 1) { bejeloles(pontok.x - 1, pontok.y, 2); return eredmeny; }
		else if (eredmeny == 2) { bejeloles(pontok.x + max_num, pontok.y, 2); return eredmeny; }
	}
	else if (tipus == 2)
	{
		eredmeny = keres_szabad_fugg(azonosito, max_num);
		if (eredmeny == 1) { bejeloles(pontok.x, pontok.y - 1, 2); return eredmeny; }
		else if (eredmeny == 2) { bejeloles(pontok.x, pontok.y + max_num, 2); return eredmeny; }
	}
	else if (tipus == 3)
	{
		eredmeny = keres_szabad_fuggatlo_lefele(azonosito, max_num);
		if (eredmeny == 1) { bejeloles(pontok.x - 1, pontok.y - 1, 2); return eredmeny; }
		else if (eredmeny == 2) { bejeloles(pontok.x + max_num, pontok.y + max_num, 2); return eredmeny; }
	}
	else if (tipus == 4)
	{
		eredmeny = keres_szabad_fuggatlo_felfele(azonosito, max_num);
		if (eredmeny == 1) { bejeloles(pontok.x + max_num, pontok.y - max_num, 2); return eredmeny; }
		else if (eredmeny == 2) { bejeloles(pontok.x - 1, pontok.y + 1, 2); return eredmeny; }
	}
	return -1;
}

//*******************
//MI építkezési lehetõség keresése
//*******************
int gyors_ellenorzes(int azonosito, int max_num)
{//-1:nincsen szabad, 1-van szabad
	if (kiegeszit_felkesz(azonosito, max_num, 1) == -1)
	{
		if (kiegeszit_felkesz(azonosito, max_num, 2) == -1)
		{
			if (kiegeszit_felkesz(azonosito, max_num, 3) == -1)
			{
				if (kiegeszit_felkesz(azonosito, max_num, 4) == -1) return -1;
				else return 1;
			}
			else return 1;
		}
		else return 1;
	}
	else return 1;
}

//*******************
//MI építkezõ-támadó lépése, ha lehetséges
//*******************
int MI_tamad()
{
	int i;
	int eredmeny;//1 lepes tortent,-1-nem tortent lepes, lehet blokkolni

	eredmeny = gyors_ellenorzes(2, 4);
	return eredmeny;
}

//*******************
//MI védekezése
//*******************
int MI_blokkol()
{
	int i;
	int eredmeny = 1;//-1 lepes tortent,1-nem tortent lepes, lehet tamadni

	for (i = 4; i > 0; --i)
	{
		if (kiegeszit_felkesz(1, i, 1) == -1)
		{
			if (kiegeszit_felkesz(1, i, 2) == -1)
			{
				if (kiegeszit_felkesz(1, i, 3) == -1)
				{
					if (kiegeszit_felkesz(1, i, 4) != -1) { eredmeny = -1; break; }
				}
				else { eredmeny = -1; break; }
			}
			else { eredmeny = -1; break; }
		}
		else { eredmeny = -1; break; }
	}
	return eredmeny;
}

//*******************
//MI védekezés, vagy támadás eldöntése
//*******************
void MI_kovetkezo_lepes()
{
	if (elso_lepes == 0)
	{
		//életszerûbb viselkedés
		elso_lepes = 1;
		if (get_random_free_cells() == 1) bejeloles(pontok.x, pontok.y, 2);
	}
	else
	{
		if (MI_tamad() == -1) MI_blokkol();
	}

	if (kereses_gyozelem(2, 5) == 1)
	{
		SetWindowTextA(Form1, "A CPU nyert!");
		gyozelem = 2;
		return;
	}
}

//*******************
//Döntetlen ellenõrzése
//*******************
int ellenoriz_dontetlen()
{
	int i, j, szamlalo;
	for (i = szamlalo = 0; i < 10; ++i)
		for (j = 0; j < 10; ++j)
		{
			if (tabla[i][j] != 0) ++szamlalo;
		}

	if (szamlalo == 100) return 1;
	else - 1;
}