#include <Windows.h>
#include "resource.h"
#include <time.h>

#define MAX_SIZE_SI 2048
#define RADIUS 22
#define COUNT 10000
#define STEP 10

//прототип функции обработки сообщений
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//функция централизации окна
void CenterWindow(HWND hwnd) {
	int x = GetSystemMetrics(SM_CXSCREEN) / 4;
	int y = GetSystemMetrics(SM_CYSCREEN) / 4;
	int w = GetSystemMetrics(SM_CXSCREEN) - 2 * x;
	int h = GetSystemMetrics(SM_CYSCREEN) - 2 * y;
	SetWindowPos(hwnd, NULL, x, y, w, h, NULL);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	LPCTSTR szClass = TEXT("wedcfgnvjhyhn");
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = szClass;
	wc.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) { return -1; }

	HMENU mainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_SCROLL));
	EnableMenuItem(mainMenu, IDM_SQUARE, MF_GRAYED);

	HWND mainWnd = CreateWindow(szClass, "Главное окно", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, mainMenu, hInstance, NULL);
	if (!mainWnd) { return -1; }

	srand(int(time(nullptr)));
	POINT points[COUNT];
	for (int i = 0; i < COUNT; i++) {
		points[i].x = RADIUS + rand() % (MAX_SIZE_SI - 2 * RADIUS);
		points[i].y = RADIUS + rand() % (MAX_SIZE_SI - 2 * RADIUS);
	}
	SetWindowLongPtrA(mainWnd, GWLP_USERDATA, (LONG_PTR)(&points));

	CenterWindow(mainWnd);
	ShowWindow(mainWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_COMMAND: {
		HMENU mainMenu = GetMenu(hwnd);
		switch (wParam)
		{
		case IDM_EXIT:
			DestroyWindow(hwnd);
			break;

		case IDM_SQUARE:
			EnableMenuItem(mainMenu, IDM_SQUARE, MF_GRAYED);
			EnableMenuItem(mainMenu, IDM_CIRCLE, MF_ENABLED);
			InvalidateRect(hwnd, NULL, TRUE);
			break;

		case IDM_CIRCLE:
			EnableMenuItem(mainMenu, IDM_CIRCLE, MF_GRAYED);
			EnableMenuItem(mainMenu, IDM_SQUARE, MF_ENABLED);
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		}
		return 0;
	}
	case WM_SIZE:
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_ALL };
		GetScrollInfo(hwnd, SB_VERT, &si);
		si.nMax = MAX_SIZE_SI - rc.bottom;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_HORZ, &si);
		si.nMax = MAX_SIZE_SI - rc.right;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	}
	case WM_PAINT:
	{
		POINT* points = (POINT*)(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hwnd, &ps);

		HBRUSH hBrush = CreateSolidBrush(RGB(100, 150, 200));
		HPEN hPen = CreatePen(PS_SOLID, 1, BLACK_PEN);
		SelectObject(hDC, hBrush);
		SelectObject(hDC, hPen);

		SCROLLINFO siv = { sizeof(SCROLLINFO), SIF_ALL };
		GetScrollInfo(hwnd, SB_VERT, &siv);
		SCROLLINFO sih = { sizeof(SCROLLINFO), SIF_ALL };
		GetScrollInfo(hwnd, SB_HORZ, &sih);

		HMENU menu = GetMenu(hwnd);

		for (int i = 0; i < COUNT; i++) {
			if (!(points[i].x - RADIUS - sih.nPos > ps.rcPaint.right) || !(points[i].x + RADIUS - sih.nPos < ps.rcPaint.left) ||
				!(points[i].y - RADIUS - siv.nPos > ps.rcPaint.bottom) || !(points[i].y + RADIUS - siv.nPos < ps.rcPaint.top))
			{
				if (GetMenuState(menu, IDM_SQUARE, MF_BYCOMMAND)) {
					Rectangle(hDC, points[i].x - RADIUS - sih.nPos, points[i].y - RADIUS - siv.nPos, points[i].x + RADIUS - sih.nPos, points[i].y + RADIUS - siv.nPos);
				}
				else {
					Ellipse(hDC, points[i].x - RADIUS - sih.nPos, points[i].y - RADIUS - siv.nPos, points[i].x + RADIUS - sih.nPos, points[i].y + RADIUS - siv.nPos);
				}
			}
		}
		return 0;
	}
	case WM_VSCROLL:
	{
		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_ALL };
		GetScrollInfo(hwnd, SB_VERT, &si);
		int startPos = si.nPos;

		switch (LOWORD(wParam)) {
		case SB_LINEUP: {
			si.nPos -= STEP;
			if (si.nPos < si.nMin) si.nPos = si.nMin;
			break;
		}
		case SB_LINEDOWN: {
			si.nPos += STEP;
			if (si.nPos > si.nMax) si.nPos = si.nMax;
			break;
		}
		case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
		}
		int offset = startPos - si.nPos;
		if (offset == 0) return 0;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		RECT paintRect;
		GetClientRect(hwnd, &paintRect);
		if (offset > 0) {
			paintRect.top = paintRect.bottom;
			paintRect.bottom += offset;
		}
		else {
			paintRect.bottom = paintRect.top;
			paintRect.top += offset;
		}
		ScrollWindow(hwnd, 0, offset, NULL, NULL);
		InvalidateRect(hwnd, &paintRect, FALSE);
		return 0;
	}
	case WM_HSCROLL:
	{
		SCROLLINFO si = { sizeof(SCROLLINFO), SIF_ALL };
		GetScrollInfo(hwnd, SB_HORZ, &si);
		int startPos = si.nPos;

		switch (LOWORD(wParam)) {
		case SB_LINELEFT: {
			si.nPos -= STEP;
			if (si.nPos < si.nMin) si.nPos = si.nMin;
			break;
		}
		case SB_LINERIGHT: {
			si.nPos += STEP;
			if (si.nPos > si.nMax) si.nPos = si.nMax;
			break;
		}
		case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
		}

		int offset = startPos - si.nPos;
		if (offset == 0) return 0;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		RECT paintRect;
		GetClientRect(hwnd, &paintRect);
		if (offset > 0) {
			paintRect.left = paintRect.right;
			paintRect.right += offset;
		}
		else {
			paintRect.right = paintRect.left;
			paintRect.left += offset;
		}
		ScrollWindow(hwnd, offset, 0, NULL, NULL);
		InvalidateRect(hwnd, &paintRect, TRUE);
		return 0;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_UP: SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0); break;
		case VK_DOWN: SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0); break;
		case VK_RIGHT: SendMessage(hwnd, WM_HSCROLL, SB_LINERIGHT, 0); break;
		case VK_LEFT: SendMessage(hwnd, WM_HSCROLL, SB_LINELEFT, 0); break;
		}
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

