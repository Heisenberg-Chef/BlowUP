// BlowUP.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "BlowUP.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BLOWUP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BLOWUP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_BLOWUP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

void InvertBlock(HWND hwndScr, HWND hwnd, POINT ptBeg, POINT ptEnd)
{
	HDC hdc;
	hdc = GetDCEx(hwndScr, NULL, DCX_CACHE | DCX_LOCKWINDOWUPDATE);
	ClientToScreen(hwnd, &ptBeg);
	ClientToScreen(hwnd, &ptEnd);
	PatBlt(hdc, ptBeg.x, ptBeg.y, ptEnd.x - ptBeg.x, ptEnd.y - ptBeg.y, DSTINVERT);
	ReleaseDC(hwndScr, hdc);
}

HBITMAP CopyBitmap(HBITMAP hBitmapSrc)
{
	BITMAP bitmap;
	HBITMAP hBitmapDst;
	HDC hdcSrc, hdcDst;
	GetObject(hBitmapSrc, sizeof(BITMAP), &bitmap);
	hBitmapDst = CreateBitmapIndirect(&bitmap);
	hdcSrc = CreateCompatibleDC(NULL);
	hdcDst = CreateCompatibleDC(NULL);
	SelectObject(hdcSrc, hBitmapSrc);
	SelectObject(hdcDst, hBitmapDst);
	BitBlt(hdcDst, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcSrc, 0, 0, SRCCOPY);

	DeleteDC(hdcSrc);
	DeleteDC(hdcDst);
	return hBitmapDst;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL bCapturing, bBlocking;
	static HBITMAP hBitmap;
	static HWND hwndScr;
	static POINT ptBeg, ptEnd;
	BITMAP hBitmapClip;
	HDC hdc, hdcMem;
	int iEnable;
	PAINTSTRUCT ps;
	RECT rc;


    switch (message)
    {
	case WM_LBUTTONDOWN:
		if (!bCapturing)
		{
			if (LockWindowUpdate(hwndScr = GetDesktopWindow()))
			{
				bCapturing = TRUE;
				SetCapture(hWnd);
				SetCursor(LoadCursor(NULL, IDC_CROSS));
			}
			else
				MessageBeep(0);
		}
		return 0;
	case WM_RBUTTONDOWN:
		if (bCapturing)
		{
			bBlocking = TRUE;
			ptBeg.x = LOWORD(lParam);
			ptBeg.y = HIWORD(lParam);
			ptEnd = ptBeg;
			InvertBlock(hwndScr, hWnd, ptBeg, ptEnd);
		}
		return 0;
	case WM_MOUSEMOVE:
		if (bBlocking)
		{
			InvertBlock(hwndScr, hWnd, ptBeg, ptEnd);
			ptEnd.x = LOWORD(lParam);
			ptBeg.y = HIWORD(lParam);
			InvertBlock(hwndScr, hWnd, ptBeg, ptEnd);
		}
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		if (bBlocking)
		{
			InvertBlock(hwndScr, hWnd, ptBeg, ptEnd);
			ptEnd.x = LOWORD(lParam);
			ptEnd.y = HIWORD(lParam);
			
			if (hBitmap)
			{
				DeleteObject(hBitmap);
				hBitmap = NULL;
			}
			hdc = GetDC(hWnd);
			hdcMem = CreateCompatibleDC(hdc);
			hBitmap = CreateCompatibleBitmap(hdc, abs(ptEnd.x - ptBeg.x), abs(ptEnd.y - ptBeg.y));

			SelectObject(hdcMem, hBitmap);
			StretchBlt(hdcMem, 0, 0, abs(ptEnd.x - ptBeg.x), abs(ptEnd.y - ptBeg.y), 
				hdc, ptBeg.x, ptBeg.y, ptEnd.x - ptBeg.x, ptEnd.y - ptBeg.y, SRCCOPY);
			DeleteDC(hdcMem);
			ReleaseDC(hWnd,hdc);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		if (bBlocking || bCapturing)
		{
			bBlocking = bCapturing = TRUE;

		}

		
    case WM_COMMAND:
      
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

