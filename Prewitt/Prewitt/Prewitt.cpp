// Prewitt.cpp : Definiuje punkt wejścia dla aplikacji.
//

/////////////////////////////////////////////
//
// Temat:			Filtr Prewitta
// Opis algorytmu:	Wczytana bitmapa najpierw jest przeksztalcana na skale szarosci, a nastepnie z wykorzystaniem wybranej  
//					bilioteki dll przesuwa sie specjalna maska w postaci macierzy 3x3 wzdluz osi X i Y wczytanej grafiki 
//					
// Rok:				3
// Semestr:			5
// Autor:			Marek Michali
// Wersja:			1
// 
//////////////////////////////////////////////


#include "framework.h"
#include <chrono> 
#include "Prewitt.h"
#include <commdlg.h>
#include <string>
#include <thread>
#include "Pack.h"
#include "Bitmap.h"
#include <fstream>
#include <commctrl.h>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#define MAX_LOADSTRING 100
#define ID_PRZYCISK1 501
#define ID_PRZYCISK2 502
#define ID_BOX 503
#define ID_PRZYCISK3 510
#define ID_PRZYCISK4 514
#define ID_PRZYCISK11 511
#define ID_PRZYCISK12 512
#define ID_PRZYCISK13 513
#define ID_PRZYCISK14 514
#define ID_PRZYCISK15 515
#define ID_PRZYCISK16 516
#define ID_PRZYCISK17 517
#define ID_BOX2 520
#define ID_PROGRES 600
#define ID_PRZYCISK21 521
// Zmienne globalne:
HINSTANCE hInst;                                // bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                  // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];            // nazwa klasy okna głównego
HWND hProgressBar;								// progressbar
HINSTANCE hinstLib;								// uchwyt do dll w c++						
HINSTANCE hinstLibASM;							// uchwyt do dll w masm x64
wchar_t g_imagePath[1000];						// przechowuje sciezke do grafiki

typedef void(__cdecl* MYPROCTH)(int, int, int, int, unsigned char*, unsigned char*);

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:


/**
* Rejestruje klasę okna. (WinAPI)
*
*/
ATOM MyRegisterClass(HINSTANCE hInstance);

/**
*   PRZEZNACZENIE: Zapisuje dojście wystąpienia i tworzy okno główne (WinAPI)
*
*   KOMENTARZE:
*
*        W tej funkcji dojście wystąpienia jest zapisywane w zmiennej globalnej i
*        jest tworzone i wyświetlane okno główne programu.
*
*/
BOOL InitInstance(HINSTANCE, int);


/**  
*  PRZEZNACZENIE: Przetwarza komunikaty dla okna głównego. (WinAPI)
*
*  WM_COMMAND  - przetwarzaj menu aplikacji
*  WM_PAINT    - Maluj okno główne
*  WM_DESTROY  - opublikuj komunikat o wyjściu i wróć
*
*/
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


/**
*  Dzieli wczytana grafike na paczki danych w liczbie rownej wybranej ilosci watkow do przetworzenia grafiki
* 
*  @param pack wektor przechowujacy paczki danych do przetworzenia przez watki
*  @param height wysokosc grafiki
*  @param threads liczba watkow na ktore podzielic grafike
*  @param size rozmiar grafiki
*/
void fillPack(std::vector<Pack*>& pack, int height, int threads, int size);

/**
*  Odpowiada za sterowanie watkami przetwarzajacymi grafike, i sklejenie w calosc przetworzonych paczek 
* 
*  @param image bitmapa
*  @param threadCount liczba watkow, ktore nalezy uzyc do przetworzenia grafiki
*  @param hWnd uchwyt do glownego okna
*  @param prewitt funkcja do przetworzenia grafiki z wybranej przez uzytkownika biblioteki dll
*/
void processImage(Bitmap &image, int threadCount, HWND hWnd, MYPROCTH prewitt);



void fillPack(std::vector<Pack*>& pack, int height, int threads, int size) {
	std::ofstream threadSplit;
	threadSplit.open("threadSplit.txt");
	for (int i = 0; i < threads; i++) {
		pack.push_back(new Pack);
		if (i == 0) {
			pack[i]->startRow = 0;
		}
		else {
			pack[i]->startRow = pack[i - 1]->endRow - 2;
		}
		if (i != threads - 1) {
			pack[i]->endRow = height * (i + 1) / threads + 1;
		}
		else {
			pack[i]->endRow = height;
		}
		threadSplit << "Start row: " << pack[i]->startRow << "	end row: " << pack[i]->endRow << "	rows to proccess: " << pack[i]->endRow - pack[i]->startRow << std::endl;
	}
	pack[0]->target = new unsigned char[size];
	threadSplit.close();
}



void processImage(Bitmap &image, int threadCount, HWND hWnd, MYPROCTH prewitt) {
	int rowSize = (image.getWidth() * 3 + ((4 - (image.getWidth() * 3) % 4) % 4));
	int size = (image.getWidth() * 3 + ((4 - (image.getWidth() * 3) % 4) % 4)) * image.getHeight();
	std::vector<Pack*> pack;
	SendMessage(hProgressBar, PBM_SETPOS, (WPARAM)0, 0);

	SendMessage(hProgressBar, PBM_SETRANGE, 0, (LPARAM)MAKELONG(0, 2 * threadCount));
	SendMessage(hProgressBar, PBM_SETPOS, (WPARAM)0, 0);
	fillPack(pack, image.getHeight(), threadCount, size);

	
	std::vector<std::thread> threads;
	std::chrono::duration<double> timeSummary = std::chrono::nanoseconds::zero();
	auto start = std::chrono::high_resolution_clock::now();
	for (int j = 0; j < threadCount; j++) {
		threads.push_back(std::thread(prewitt, image.getWidth(), image.getHeight(), pack[j]->startRow, pack[j]->endRow, image.getSourceImageAddress(), pack[0]->target));
		//pack[j]->startRow++;
	}

	for (auto& th : threads) {
		SendMessage(hProgressBar, PBM_DELTAPOS, (WPARAM)threadCount, 0);
		th.join();
		SendMessage(hProgressBar, PBM_DELTAPOS, (WPARAM)threadCount, 0);
		
	}

	threads.erase(threads.begin(), threads.end());

	auto finish = std::chrono::high_resolution_clock::now();
	timeSummary = finish - start;
	std::string str = "Czas dzialania biblioteki dll: ";
	str.append(std::to_string(timeSummary.count()));
	str.append(" s");
	std::wstring stemp = std::wstring(str.begin(), str.end());
	LPCWSTR sw = stemp.c_str();

	delete[] image.sourceImage;
	image.sourceImage = pack[0]->target;

	for (int i = 0; i < threadCount; i++) {
		delete pack[i];
	}
	MessageBox(hWnd, sw, L"Czas", MB_ICONINFORMATION);
}





int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	// Inicjuj ciągi globalne
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PREWITT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Wykonaj inicjowanie aplikacji:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PREWITT));

	//wczytanie bibliotek dll
	hinstLib = LoadLibrary(TEXT("PrewittFilter.dll"));
	hinstLibASM = LoadLibrary(TEXT("PrewittFilterASM.dll"));
	MSG msg;

	// Główna pętla komunikatów:
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PREWITT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PREWITT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 900, 300, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd); 

	//przyciski i ramki w gui
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"Wybierz dll:", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 10, 10, 350, 100, hWnd, (HMENU)ID_BOX, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"C++", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 20, 50,330, 20, hWnd, (HMENU)ID_PRZYCISK1, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"ASM", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 20, 75, 330, 20, hWnd, (HMENU)ID_PRZYCISK2, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"Ile watkow uzyc?:", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 510, 10, 350, 200, hWnd, (HMENU)ID_BOX2, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"1", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 520, 30, 300, 20, hWnd, (HMENU)ID_PRZYCISK11, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"2", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 520, 55, 300, 20, hWnd, (HMENU)ID_PRZYCISK12, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"4", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 520, 80, 300, 20, hWnd, (HMENU)ID_PRZYCISK13, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"8", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 520, 105, 300, 20, hWnd, (HMENU)ID_PRZYCISK14, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"16", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 520, 130, 300, 20, hWnd, (HMENU)ID_PRZYCISK15, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"32", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 520, 155, 300, 20, hWnd, (HMENU)ID_PRZYCISK16, hInst, NULL);
	CreateWindowEx(WS_EX_WINDOWEDGE, L"BUTTON", L"64", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 520, 180, 300, 20, hWnd, (HMENU)ID_PRZYCISK17, hInst, NULL);
	CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Przeksztalc", WS_CHILD | WS_VISIBLE | WS_BORDER, 200, 140, 150, 30, hWnd, (HMENU)ID_PRZYCISK3, hInst, NULL);
	CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Wczytaj BMP", WS_CHILD | WS_VISIBLE | WS_BORDER, 20, 140, 150, 30, hWnd, (HMENU)ID_PRZYCISK21, hInst, NULL);


	CheckRadioButton(hWnd, ID_PRZYCISK1, ID_PRZYCISK2, ID_PRZYCISK1);
	CheckRadioButton(hWnd, ID_PRZYCISK11, ID_PRZYCISK13, ID_PRZYCISK11);

	//tworzenie progressbara
	InitCommonControls();
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icc);
	hProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 80, 190, 200, 15, hWnd, (HMENU)ID_PROGRES, hInstance, NULL);
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{          
		switch (wParam)
		{

		case ID_PRZYCISK3:
			if (IsDlgButtonChecked(hWnd, ID_PRZYCISK1)) {

				if (hinstLib != NULL)
				{
					MYPROCTH PrewittCPP = (MYPROCTH)GetProcAddress(hinstLib, "PrewittThread");
					if (NULL != PrewittCPP)
					{
						char* str = (char*)malloc(1000);
						size_t i;
						wcstombs_s(&i, str, (size_t)1000, g_imagePath, (size_t)1000 - 1);
						Bitmap image(0, 0);
						int status = image.read(str);
						if (status == -1) {
							MessageBox(hWnd, L"Nie udalo sie przeksztalcic wybranego pliku", L"Blad", MB_ICONINFORMATION);
							free(str);
							return 0;
						}
						else if (status == -2) {
							MessageBox(hWnd, L"Wybrana bitmapa nie jest 24 bitowa", L"Blad", MB_ICONINFORMATION);
							free(str);
							return 0;
						}
						int index = 0;
						int padding = ((4 - (image.getWidth() * 3) % 4) % 4);
						for (int y = 0; y < image.getHeight(); y++) {
							for (int x = 0; x < image.getWidth(); x++) {
								image.getSourceImageAddress()[index] = image.getSourceImageAddress()[index] * 0.299 + image.getSourceImageAddress()[index + 1] * 0.587 + image.getSourceImageAddress()[index + 2] * 0.114;
								image.getSourceImageAddress()[index + 1] = image.getSourceImageAddress()[index];
								image.getSourceImageAddress()[index + 2] = image.getSourceImageAddress()[index];
								index = index + 3;
							}
							index = index + padding;
						}
						free(str);

						if (IsDlgButtonChecked(hWnd, ID_PRZYCISK11)) {
							processImage(image, 1, hWnd, PrewittCPP);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK12)) {
							processImage(image, 2, hWnd, PrewittCPP);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK13)) {
							processImage(image, 4, hWnd, PrewittCPP);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK14)) {
							processImage(image, 8, hWnd, PrewittCPP);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK15)) {
							processImage(image, 16, hWnd, PrewittCPP);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK16)) {
							processImage(image, 32, hWnd, PrewittCPP);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK17)) {
							processImage(image, 64, hWnd, PrewittCPP);
						}
						image.save("convertedImage.bmp");
					}
				}
			}
			else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK2)) {
				if (hinstLibASM != NULL)
				{
					MYPROCTH PrewittASM = (MYPROCTH)GetProcAddress(hinstLibASM, "PrewittThreadASM");
					if (NULL != PrewittASM)
					{
						char* str = (char*)malloc(1000);
						size_t i;
						wcstombs_s(&i, str, (size_t)1000, g_imagePath, (size_t)1000 - 1);
						Bitmap image(0, 0);
						int status = image.read(str);
						if (status == -1) {
							MessageBox(hWnd, L"Nie udalo sie przeksztalcic wybranego pliku", L"Blad", MB_ICONINFORMATION);
							free(str);
							return 0;
						}
						else if (status == -2) {
							MessageBox(hWnd, L"Wybrana bitmapa nie jest 24 bitowa", L"Blad", MB_ICONINFORMATION);
							free(str);
							return 0;
						}
						int z = 0;
						int paddingAmount = ((4 - (image.getWidth() * 3) % 4) % 4);
						for (int y = 0; y < image.getHeight(); y++) {
							for (int x = 0; x < image.getWidth(); x++) {
								image.getSourceImageAddress()[z] = image.getSourceImageAddress()[z] * 0.299 + image.getSourceImageAddress()[z + 1] * 0.587 + image.getSourceImageAddress()[z + 2] * 0.114;
								image.getSourceImageAddress()[z + 1] = image.getSourceImageAddress()[z];
								image.getSourceImageAddress()[z + 2] = image.getSourceImageAddress()[z];
								z = z + 3;
							}
							z = z + paddingAmount;
						}
						free(str);

						if (IsDlgButtonChecked(hWnd, ID_PRZYCISK11)) {
							processImage(image, 1, hWnd, PrewittASM);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK12)) {
							processImage(image, 2, hWnd, PrewittASM);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK13)) {
							processImage(image, 4, hWnd, PrewittASM);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK14)) {
							processImage(image, 8, hWnd, PrewittASM);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK15)) {
							processImage(image, 16, hWnd, PrewittASM);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK16)) {
							processImage(image, 32, hWnd, PrewittASM);
						}
						else if (IsDlgButtonChecked(hWnd, ID_PRZYCISK17)) {
							processImage(image, 64, hWnd, PrewittASM);
						}
						image.save("convertedImage.bmp");
					}
				}
				else {
					MessageBox(hWnd, L"Radio 2 asm", L"Info", MB_ICONINFORMATION);
				}
			}
			break;

		case ID_PRZYCISK21:
			LPSTR filebuff = new char[256];
			OPENFILENAME open = { 0 };
			open.lStructSize = sizeof(OPENFILENAME);
			open.hwndOwner = hWnd;
			open.lpstrFilter = L".bmp\0*.bmp;*\0\0";
			open.lpstrCustomFilter = NULL;
			open.lpstrFile = g_imagePath;
			open.lpstrFile[0] = '\0';
			open.nMaxFile = 256;
			open.nFilterIndex = 1;
			open.lpstrInitialDir = NULL;
			open.lpstrTitle = L"Wybierz bitmape\0";
			open.nMaxFileTitle = strlen("Wybierz bitmape\0");
			open.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
			GetOpenFileName(&open);
			break;
		}
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		FreeLibrary(hinstLib);
		FreeLibrary(hinstLibASM);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}