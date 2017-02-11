#include "stdafx.h"


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
//		CApplication::m_hWnd = hWnd;
		lpcs = (LPCREATESTRUCT)lParam;
		pApp = (CApplication*)(lpcs->lpCreateParams);
		pApp->handle_create(hWnd, (LPCREATESTRUCT*)lParam);
		pApp->m_hWnd = hWnd;
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_MOUSEMOVE:
		pApp->handle_mousemove(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case WM_LBUTTONDOWN:
		if (false == pApp->handle_lbuttondown(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			break;
		return 0;

	case WM_RBUTTONDOWN:
		if (false == pApp->handle_rbuttondown(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			break;
		return 0;

	case WM_RBUTTONUP:
		if (false == pApp->handle_rbuttonup(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			break;
		return 0;

	case WM_LBUTTONUP:
		if (false == pApp->handle_lbuttonup(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			break;
		return 0;

	case WM_COMMAND:
		if (false == pApp->handle_command(hWnd, LOWORD(wParam), HIWORD(wParam), lParam))
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
		if (pApp->handle_lbuttondoubleclick(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
			return 0;
		break;

	case WM_MOUSEWHEEL:
		if (pApp->handle_mousewheel(
				hWnd, 
				GET_KEYSTATE_WPARAM(wParam), // virtyal keys pressed
				GET_WHEEL_DELTA_WPARAM(wParam), // number of 'ticks' of rotation
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam)))
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