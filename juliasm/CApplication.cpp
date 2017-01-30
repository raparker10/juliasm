#include "stdafx.h"

HWND CApplication::m_hWnd;


// constructor
CApplication::CApplication()
{

}

// destructor
CApplication::~CApplication()
{

}

// windows message handling
LRESULT CALLBACK CApplication::MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	int iScrollRequest, iScrollPosition;
	static CApplication *pApp = NULL;
	LPCREATESTRUCT lpcs;
	bool bRet;

	switch (message)
	{
	case WM_CREATE:
		CApplication::m_hWnd = hWnd;
		lpcs = (LPCREATESTRUCT)lParam;
		pApp = (CApplication*)(lpcs->lpCreateParams);
		pApp->handle_create(hWnd, (LPCREATESTRUCT*)lParam);
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_MOUSEMOVE:
		pApp->handle_mousemove(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	case WM_COMMAND:
		if (false == pApp->handle_command(hWnd, LOWORD(wParam), HIWORD(wParam)))
			return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_KEYDOWN:
		if (false == pApp->handle_keydown(hWnd, wParam))
			break;
		return 0;

	case WM_CHAR:
		if (false == pApp->handle_char(hWnd, wParam))
			break;
		return 0;

	case WM_VSCROLL:
		iScrollRequest = LOWORD(wParam);
		if (iScrollRequest == SB_THUMBPOSITION || iScrollRequest == SB_THUMBTRACK)
		{
			iScrollPosition = HIWORD(wParam);
		}
		else
		{
			iScrollPosition = 0;
		}
		if (false == pApp->handle_vscroll(hWnd, iScrollRequest, iScrollPosition))
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;

	case WM_SIZE:
		pApp->handle_size(hWnd, GetDC(hWnd), wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		bRet = pApp->handle_paint(hWnd, hdc, &ps);
		EndPaint(hWnd, &ps);
		// return if the application processed WM_PAINT, otherwise fall through
		if (bRet == true)
			return 0;
		break;

	case WM_LBUTTONDBLCLK:
		if (pApp->handle_lbuttondoubleclick(hWnd, wParam, LOWORD(lParam), HIWORD(lParam)))
			return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}