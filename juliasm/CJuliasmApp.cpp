#include "stdafx.h"

INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


// application initialization
CJuliasmApp::CJuliasmApp() 
{
	Initialize();
};

// application shutdown
CJuliasmApp::~CJuliasmApp() 
{
	if (m_hfInfo != INVALID_HANDLE_VALUE)
	{
		DeleteObject(m_hfInfo);
		m_hfInfo = (HFONT)INVALID_HANDLE_VALUE;
	}
};

// initialize application member variables
void CJuliasmApp::Initialize(void)
{
	// image panel size initialization
	// These are not strictly needed due to processing of the WM_SIZE message
	// that will automatically set them anyway
	m_iMandWidth = FRACTAL_WIDTH_DEFAULT;
	m_iMandHeight = FRACTAL_HEIGHT_DEFAULT;
	m_iJuliaWidth = JULIA_WIDTH_DEFAULT;
	m_iJuliaHeight = JULIA_HEIGHT_DEFAULT;
	m_iJuliaLeft = m_iMandWidth;

	// initialize mandelbrot calculation variables
	// This functionality is packaged into a function so that 
	// it is easily called separately from the program menu
	InitializeMand();

	// initialize julia set calculation variables
	m_jc1_sse = -1.0;
	m_jc2_sse = 1.0;
	m_jd1_sse = -1.0;
	m_jd2_sse = 1.0;

	put_MaxIterationsJulia(1024);
	m_iJMaxIter = 0;
	m_iJMaxThread = 0;

	// initialize thread-handling variables
	m_iMandelbrotThreadCount = 0;
	m_iJuliaThreadCount = 0;

	// initialize status-keeping variables
	for (int i = 0; i < _countof(m_iJuliaReady); ++i)
	{
		m_iJuliaReady[i] = 0;
	}
	m_iCalculatingJulia = 0;
	m_iCalculatingMandelbrot = 0;
	m_szMethod = "Undefined";

	ZeroMemory(m_iJuliaReady, sizeof(m_iJuliaReady));

	//
	// setup the palette
	//
	// setup the default palette

	// setup the red color channel
	m_PaletteDefault.PushColorPoint(0, CPixelPoint(0,0));
	m_PaletteDefault.PushColorPoint(0, CPixelPoint(12, 255));
	m_PaletteDefault.PushColorPoint(0, CPixelPoint(200, 255));
	m_PaletteDefault.PushColorPoint(0, CPixelPoint(255, 0));

	// setup the green color channel
	m_PaletteDefault.PushColorPoint(1, CPixelPoint(0, 0));
	m_PaletteDefault.PushColorPoint(1, CPixelPoint(4, 255));
	m_PaletteDefault.PushColorPoint(1, CPixelPoint(128, 0));
	m_PaletteDefault.PushColorPoint(1, CPixelPoint(192, 255));
	m_PaletteDefault.PushColorPoint(1, CPixelPoint(255, 255));

	// setup the blue color channel
	m_PaletteDefault.PushColorPoint(2, CPixelPoint(0, 0));
	m_PaletteDefault.PushColorPoint(2, CPixelPoint(255 / 6, 255));
	m_PaletteDefault.PushColorPoint(2, CPixelPoint(255 / 3, 0));
	m_PaletteDefault.PushColorPoint(2, CPixelPoint(128, 255));
	m_PaletteDefault.PushColorPoint(2, CPixelPoint(255 * 2 / 3, 0));
	m_PaletteDefault.PushColorPoint(2, CPixelPoint(255 * 5 / 6, 255));
	m_PaletteDefault.PushColorPoint(2, CPixelPoint(255, 0));

	// create a palette from the color points
	m_PaletteDefault.UpdateColors();

	// initialize the high-resolution timing functionality
	QueryPerformanceFrequency(&m_ticksPerSecond);

	// default the calculation "platform": None, x87, SSE, ... etc ...
	put_CalcPlatformMand(CalcPlatform::None);
	put_CalcPlatformJulia(CalcPlatform::None);

	//
	// initialize the font information
	ZeroMemory(&m_ncm, sizeof(m_ncm));
	ZeroMemory(&m_lfInfo, sizeof(m_lfInfo));
	m_hfInfo = (HFONT)INVALID_HANDLE_VALUE;

}

// Initialize variables used for mandelbrot set calculation.
// These are broken out separately so that they can be easily
// reset from the main menu.
void CJuliasmApp::InitializeMand(void)
{

	// bounding box
	m_a1 = -2.0f;
	m_a2 = 2.0f;
	m_b1 = -2.0f;
	m_b2 = 2.0f;

	// per-pixel offsets in the horizontal (da) and vertical (db) directions
	put_MaxIterationsMand(1024);
}

// handle a mouse double-click.
// Used to zoom into an image
//
bool CJuliasmApp::handle_lbuttondoubleclick(HWND hWnd, WPARAM wvKeyDown, WORD x, WORD y)
{
	double height, width, da, db, anew, bnew;

	// only proceed if there are no mandelbrot calculations in the works
	if (m_iCalculatingMandelbrot == 0)
	{
		// adjust the y value for the top of the mandelbrot image
		y = y - JULIA_TOP;

		da = (m_a2 - m_a1) / m_iMandWidth;
		db = (m_b2 - m_b1) / m_iMandHeight;
		width = m_a2 - m_a1;
		height = m_b2 - m_b1;
		anew = m_a1 + da * x;
		bnew = m_b1 + db * y;

		width = width * 0.8;
		height = height * 0.8;
		m_a1 = anew - (width / 2.0);
		m_a2 = m_a1 + width;

		m_b1 = bnew - (height / 2.0);
		m_b2 = m_b1 + height;

		CalculateMandelbrot();
		return true;
	}
	return false;
}

//
// Handle a repaint of the screen
//
bool CJuliasmApp::handle_paint(HWND hWnd, HDC hdc, LPPAINTSTRUCT ps) 
{ 
	int iLines = 0;		// number of lines drawn to the screen
	char szBuf[256];

	// update the number of ticks per second for the high-resolution timer
	// this is needed because the clock frequency can change dynamically
	QueryPerformanceFrequency(&m_ticksPerSecond);

	// only draw the mandelbrot image if it is not being calculated
	if (0 == m_iCalculatingMandelbrot)
	{
		iLines = m_bmpMandelbrot.Show(hdc, 0, JULIA_TOP);
	}

	// only draw the julia set if it is not being calculated
	if (0 == m_iCalculatingJulia)
	{
		iLines = m_bmpJulia.Show(hdc, m_iMandWidth, JULIA_TOP);
	}

	//
	// update the statistics being displayed on the screen
	//

	// clear the text panel areas
	RECT rc;
	GetClientRect(hWnd, &rc);
	rc.bottom = JULIA_TOP;
	FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

	// clear the area below the mandelbrot display
	GetClientRect(hWnd, &rc);
	rc.top = JULIA_TOP + m_iMandHeight;
	rc.right = m_iMandWidth;
	FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

/*
	//		sprintf(buf, "Mandelbrot: Duration=%u:%u, Ticks Per Clock=%u:%u, da=%e, db=%e", tMandelbrotTotal.HighPart, tMandelbrotTotal.LowPart, ticksPerSecond.HighPart, ticksPerSecond.LowPart, ::da, ::db);
	sprintf_s(szBuf, _countof(szBuf),"Mandelbrot: Duration=%u, Ticks Per Clock=%u:%u, da=%e, db=%e", m_tMandelbrotTotal.QuadPart, m_ticksPerSecond.HighPart, m_ticksPerSecond.LowPart, m_da, m_db);
	TextOut(hdc, 0, 0, szBuf, lstrlen(szBuf));

	sprintf_s(szBuf, _countof(szBuf),"Method=%s, p(1)=(%f, %f), p(2)=(%f, %f)", m_szMethod, m_a1, m_b1, m_a2, m_b2);
	TextOut(hdc, 0, 20, szBuf, lstrlen(szBuf));

	// display the julia set parameters
	sprintf_s(szBuf, _countof(szBuf), "Julia: a=%f, b=%f, %s, maxiter=%d, maxthread=%d                  ", m_ja_sse, m_jb_sse, (m_iCalculatingJulia == 0) ? "Not Calculating" : "Calculating", m_iJMaxIter, m_iJMaxThread);
	TextOut(hdc, 500, 20, szBuf, lstrlen(szBuf));

	// display the julia set parameters
	sprintf_s(szBuf, _countof(szBuf),"Julia: Duration=%u ms  ", m_iTotalTicks.QuadPart);
	TextOut(hdc, 0, 35, szBuf, lstrlen(szBuf));
*/
	//
	// display information
	//
	int iLen;
	HFONT hOldFont = (HFONT)SelectObject(hdc, m_hfInfo);
	int y = JULIA_TOP + m_iMandHeight;
	char szPlatform[16];

	// mandelbrot section header
	iLen = sprintf_s(
				szBuf, 
				_countof(szBuf), 
				"Mandelbrot - %s - %d iter", 
				get_CalcPlatformName(
					szPlatform, 
					_countof(szPlatform), 
					get_CalcPlatformMand()), 
				get_MaxIterationsMand());

	ExtTextOut(hdc, 0, y, ETO_CLIPPED, &rc, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// mandelbrot image bounding box
	iLen = sprintf_s(szBuf, _countof(szBuf), "Box (%02.2f, %02.2f)-(%02.2f, %02.2f)", m_a1, m_b1, m_a2, m_b2);
	ExtTextOut(hdc, m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &rc, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// mouse pointer location
	iLen = sprintf_s(szBuf, _countof(szBuf), "Pointer (%02.4f, %02.4f)", m_ja_sse, m_jb_sse);
	ExtTextOut(hdc, m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &rc, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// last recalc duration
	iLen = sprintf_s(szBuf, _countof(szBuf), "Clock Time %lld ms", 1000 * m_tMandelbrotProcessDurationTotal.QuadPart / m_ticksPerSecond.QuadPart);
	ExtTextOut(hdc, m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &rc, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// last recalc thread working duration
	iLen = sprintf_s(szBuf, _countof(szBuf), "Thread Time %lld ms", 1000 * m_tMandelbrotThreadDurationTotal.QuadPart / m_ticksPerSecond.QuadPart);
	ExtTextOut(hdc, m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &rc, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	//
	// Julia set
	//

	// Julia Set section header
	y += m_tmInfo.tmHeight;
	iLen = sprintf_s(
				szBuf, 
				_countof(szBuf), 
				"Julia - %s - %d iter", 
				get_CalcPlatformName(
					szPlatform, 
					_countof(szPlatform), 
					get_CalcPlatformJulia()), 
				get_MaxIterationsJulia()); 

	ExtTextOut(hdc, 0, y, ETO_CLIPPED, &rc, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// Julia set image bounding box
	iLen = sprintf_s(szBuf, _countof(szBuf), "Box (%02.2f, %02.2f)-(%02.2f, %02.2f)", m_jc1_sse, m_jd1_sse, m_jc2_sse, m_jd2_sse);
	ExtTextOut(hdc, m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &rc, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// last recalc duration
	iLen = sprintf_s(szBuf, _countof(szBuf), "Clock Time %lld ms", 1000 * m_tJuliaProcessDurationTotal.QuadPart / m_ticksPerSecond.QuadPart);
	ExtTextOut(hdc, m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &rc, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// last recalc thread working duration
	iLen = sprintf_s(szBuf, _countof(szBuf), "Thread Time %lld ms", 1000 * m_tJuliaThreadDurationTotal.QuadPart / m_ticksPerSecond.QuadPart);
	ExtTextOut(hdc, m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &rc, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	

	SelectObject(hdc, hOldFont);
	return true;
}

//
// handle main window startup 
//
bool CJuliasmApp::handle_create(HWND hWnd, LPCREATESTRUCT *lpcs) 
{ 
	// 
	// create the font for information display
	//
	m_ncm.cbSize = sizeof(m_ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(m_ncm), &m_ncm, 0);
	m_hfInfo = CreateFontIndirect(&m_ncm.lfMessageFont);
	HDC hdc = GetDC(hWnd);
	GetTextMetrics(hdc, &m_tmInfo);
	ReleaseDC(hWnd, hdc);


/*	// create the julia thread pool
	for (int i = 0; i < MAX_JULIA_THREADS; ++i)
	{
		m_ThreadInfoJulia[i].iThreadIndex = i;
		m_ThreadInfoJulia[i].pApp = this;
		m_hThreadJulia[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateJuliaAVX32, (LPVOID)&m_ThreadInfoJulia[i], 0, &m_dwThreadJuliaID[i]);
	}
*/
	// setup the application defaults
	put_CalcPlatformMand(CalcPlatform::SSE);
	put_MaxIterationsMand(1024);
	put_CalcPlatformJulia(CalcPlatform::AVX32);
	put_MaxIterationsJulia(1024);


	// request a redraw of the mandalbrot image
	PostMessage(hWnd, WM_COMMAND, IDM_RECALCULATE_MAND, 0);
	return true;
}

//
// Initiate a mandelbrot recalculate using x87 math capabilities
//
void CJuliasmApp::StartMandelbrotx87(HWND hWnd)
{

	// set the x87 as the current calculation platform
	if (get_CalcPlatformMand() != CalcPlatform::x87)
		put_CalcPlatformMand(CalcPlatform::x87);


	// only recalculate if there is not an ongoing calculation
	if (m_iCalculatingMandelbrot != 0)
	{
		MessageBeep(-1);
		return;
	}

	// record the proess start time
	QueryPerformanceCounter(&m_tMandelbrotProcessStart);

	// setup performance measurement
	m_tMandelbrotProcessStop.QuadPart = 0;
	m_tMandelbrotProcessDurationTotal.QuadPart = 0;
	m_tMandelbrotThreadDurationTotal.QuadPart = 0;
	SecureZeroMemory(&m_tMandelbrotThreadDuration, sizeof(m_tMandelbrotThreadDuration));

	// save the platform type
	m_szMethod = "x87";

	// create threads to perform the calculation
	SecureZeroMemory(m_hThreadMandelbrot, sizeof(m_hThreadMandelbrot));
	m_iMandelbrotThreadCount = MAX_MAND_THREADS;
	for (int i = 0; i < m_iMandelbrotThreadCount; ++i)
	{
		InterlockedIncrement(&m_iCalculatingMandelbrot);
		m_ThreadInfoMand[i].iThreadIndex = i;
		m_ThreadInfoMand[i].pApp = this;
		m_hThreadMandelbrot[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateMandX87, (LPVOID)&m_ThreadInfoMand[i], 0, NULL);
	}
}

//
// Initiate a mandelbrot recalculate using SSE math capabilities
//
void CJuliasmApp::StartMandelbrotSSE(HWND hWnd)
{
	// only recalculate if there is not an ongoing calculation
	if (m_iCalculatingMandelbrot != 0)
	{
		MessageBeep(-1);
		return;
	}
	if (get_CalcPlatformMand() != CalcPlatform::SSE)
	{
		put_CalcPlatformMand(CalcPlatform::SSE);
	}


	// record the proess start time
	QueryPerformanceCounter(&m_tMandelbrotProcessStart);

	// setup performance measurement
	m_tMandelbrotProcessStop.QuadPart = 0;
	m_tMandelbrotProcessDurationTotal.QuadPart = 0;
	m_tMandelbrotThreadDurationTotal.QuadPart = 0;
	SecureZeroMemory(&m_tMandelbrotThreadDuration, sizeof(m_tMandelbrotThreadDuration));

	// save the platform type
	m_szMethod = "SSE";

	// create threads to perform the calculation
	SecureZeroMemory(m_hThreadMandelbrot, sizeof(m_hThreadMandelbrot));
	m_iMandelbrotThreadCount = MAX_MAND_THREADS;
	for (int i = 0; i < m_iMandelbrotThreadCount; ++i)
	{
		InterlockedIncrement(&m_iCalculatingMandelbrot);
		m_ThreadInfoMand[i].iThreadIndex = i;
		m_ThreadInfoMand[i].pApp = this;
		m_hThreadMandelbrot[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateMandSSE, (LPVOID)&m_ThreadInfoMand[i], 0, NULL);
	}
}

//
// Initiate a mandelbrot recalculate using SSE2 math capabilities
//
void CJuliasmApp::StartMandelbrotSSE2(HWND hWnd)
{
	// only recalculate if there is not an ongoing calculation
	if (m_iCalculatingMandelbrot != 0)
	{
		MessageBeep(-1);
		return;
	}
	if (get_CalcPlatformMand() != CalcPlatform::SSE2)
	{
		put_CalcPlatformMand(CalcPlatform::SSE2);
	}


	// save the calculation platform
	m_szMethod = "SSE2";

	// setup the performance counters
	// record the proess start time
	QueryPerformanceCounter(&m_tMandelbrotProcessStart);

	// setup performance measurement
	m_tMandelbrotProcessStop.QuadPart = 0;
	m_tMandelbrotProcessDurationTotal.QuadPart = 0;
	m_tMandelbrotThreadDurationTotal.QuadPart = 0;
	SecureZeroMemory(&m_tMandelbrotThreadDuration, sizeof(m_tMandelbrotThreadDuration));


	// start the threads
	SecureZeroMemory(m_hThreadMandelbrot, sizeof(m_hThreadMandelbrot));
	m_iMandelbrotThreadCount = MAX_MAND_THREADS;
	for (int i = 0; i < m_iMandelbrotThreadCount; ++i)
	{
		InterlockedIncrement(&m_iCalculatingMandelbrot);
		m_ThreadInfoMand[i].iThreadIndex = i;
		m_ThreadInfoMand[i].pApp = this;
		m_hThreadMandelbrot[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateMandSSE2, (LPVOID)&m_ThreadInfoMand[i], 0, NULL);
	}
}

//
// Initiate a mandelbrot recalculate using SSE2 math capabilities
//
void CJuliasmApp::StartMandelbrotAVX32(HWND hWnd)
{
	// only recalculate if there is not an ongoing calculation
	if (m_iCalculatingMandelbrot != 0)
	{
		MessageBeep(-1);
		return;
	}
	
	if (get_CalcPlatformMand() != CalcPlatform::AVX32)
	{
		put_CalcPlatformMand(CalcPlatform::AVX32);
	}


	// save the calculation platform
	m_szMethod = "AVX 32-bit";

	// setup the performance counters
	// record the proess start time
	QueryPerformanceCounter(&m_tMandelbrotProcessStart);

	// setup performance measurement
	m_tMandelbrotProcessStop.QuadPart = 0;
	m_tMandelbrotProcessDurationTotal.QuadPart = 0;
	m_tMandelbrotThreadDurationTotal.QuadPart = 0;
	SecureZeroMemory(&m_tMandelbrotThreadDuration, sizeof(m_tMandelbrotThreadDuration));


	// start the threads
	SecureZeroMemory(m_hThreadMandelbrot, sizeof(m_hThreadMandelbrot));
	m_iMandelbrotThreadCount = MAX_MAND_THREADS;
	for (int i = 0; i < m_iMandelbrotThreadCount; ++i)
	{
		InterlockedIncrement(&m_iCalculatingMandelbrot);
		m_ThreadInfoMand[i].iThreadIndex = i;
		m_ThreadInfoMand[i].pApp = this;
		m_hThreadMandelbrot[i] = CreateThread(NULL, THREAD_STACK_SIZE * 2, CalculateMandAVX32, (LPVOID)&m_ThreadInfoMand[i], 0, NULL);
	}
}

//
// Initiate a mandelbrot recalculate using SSE2 math capabilities
//
void CJuliasmApp::StartMandelbrotAVX64(HWND hWnd)
{
	// only recalculate if there is not an ongoing calculation
	if (m_iCalculatingMandelbrot != 0)
	{
		MessageBeep(-1);
		return;
	}
	if (get_CalcPlatformMand() != CalcPlatform::AVX64)
	{
		put_CalcPlatformMand(CalcPlatform::AVX64);
	}

	// save the calculation platform
	m_szMethod = "AVX 32-bit";

	// setup the performance counters
	// record the proess start time
	QueryPerformanceCounter(&m_tMandelbrotProcessStart);

	// setup performance measurement
	m_tMandelbrotProcessStop.QuadPart = 0;
	m_tMandelbrotProcessDurationTotal.QuadPart = 0;
	m_tMandelbrotThreadDurationTotal.QuadPart = 0;
	SecureZeroMemory(&m_tMandelbrotThreadDuration, sizeof(m_tMandelbrotThreadDuration));


	// start the threads
	SecureZeroMemory(m_hThreadMandelbrot, sizeof(m_hThreadMandelbrot));
	m_iMandelbrotThreadCount = MAX_MAND_THREADS;
	for (int i = 0; i < m_iMandelbrotThreadCount; ++i)
	{
		InterlockedIncrement(&m_iCalculatingMandelbrot);
		m_ThreadInfoMand[i].iThreadIndex = i;
		m_ThreadInfoMand[i].pApp = this;
		m_hThreadMandelbrot[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateMandAVX64, (LPVOID)&m_ThreadInfoMand[i], 0, NULL);
	}
}

//
// Initiate a mandelbrot recalculate using AVX2 math capabilities
// This function is a placeholder for future functionality.
//
void CJuliasmApp::StartMandelbrotAVX2(HWND hWnd)
{
	MessageBox(hWnd, "Function not implemented.", "Error", MB_ICONINFORMATION | MB_OK);
}


//
// handle command messages 
//
bool CJuliasmApp::handle_command(HWND hWnd, int wmID, int wmEvent)
{ 
	int i;

	switch (wmID)
	{
	case IDM_CALCULATE_MAND_X87:
		StartMandelbrotx87(hWnd);
		break;

	case IDM_CALCULATE_MAND_SSE:
		StartMandelbrotSSE(hWnd);
		break;

	case IDM_CALCULATE_MAND_SSE2:
		StartMandelbrotSSE2(hWnd);
		break;

	case IDM_CALCULATE_MAND_AVX32:
		StartMandelbrotAVX32(hWnd);
		break;

	case IDM_CALCULATE_MAND_AVX64:
		StartMandelbrotAVX64(hWnd);
		break;

	case IDM_CLEAR_MAND:
		m_bmpJulia.Erase(RGB(255, 255, 255));
		m_bmpMandelbrot.Erase(RGB(255, 255, 255));
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case IDM_RESET_MAND:
		if (m_iCalculatingMandelbrot == 0)
		{
			InitializeMand();
			PostMessage(get_hWnd(), WM_COMMAND, IDM_RECALCULATE_MAND, 0);
		}
		break;

	case IDM_RECALCULATE_MAND:
		CalculateMandelbrot();
		break;

	case IDM_CALCULATE_JULIA_X87:
		put_CalcPlatformJulia(CalcPlatform::x87);
		RecalculateJulia();
		return 0;

	case IDM_CALCULATE_JULIA_SSE:	// fall through until SSE Julia is implemented
		put_CalcPlatformJulia(CalcPlatform::SSE);
		RecalculateJulia();
		return 0;

	case IDM_CALCULATE_JULIA_SSE2:	// fall through until SSE2 Julia implemented
		put_CalcPlatformJulia(CalcPlatform::SSE2);
		RecalculateJulia();
		return 0;

	case IDM_CALCULATE_JULIA_AVX32:
		put_CalcPlatformJulia(CalcPlatform::AVX32);
		RecalculateJulia();
		return 0;

	case IDM_CALCULATE_JULIA_AVX64:	// fall through
		put_CalcPlatformJulia(CalcPlatform::AVX64);
		RecalculateJulia();
		return 0;


	case IDM_RECALCULATE_JULIA:
		RecalculateJulia();
		return true;

	case IDM_THREADCOMPLETE:
		if (0 == InterlockedDecrementAcquire(&m_iCalculatingMandelbrot))
		{
			m_tMandelbrotProcessDurationTotal.QuadPart = 0;

			// kill the threads and update the total thread working duration
			for (i = 0; i < m_iMandelbrotThreadCount; ++i)
			{
				CloseHandle(m_hThreadMandelbrot[i]);
				m_tMandelbrotThreadDurationTotal.QuadPart += m_tMandelbrotThreadDuration[i].QuadPart;
			}
			// update the total process duration
			QueryPerformanceCounter(&m_tMandelbrotProcessStop);
			m_tMandelbrotProcessDurationTotal.QuadPart = m_tMandelbrotProcessStop.QuadPart - m_tMandelbrotProcessStart.QuadPart;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case IDM_THREADCOMPLETEJULIA87:
		if (0 == InterlockedDecrementAcquire(&m_iCalculatingJulia))
		{
			m_tJuliaProcessDurationTotal.QuadPart = 0;

			// kill the threads and update the total thread working duration
			for (i = 0; i < m_iJuliaThreadCount; ++i)
			{
				CloseHandle(m_hThreadJulia[i]);
				m_tJuliaThreadDurationTotal.QuadPart += m_tJuliaThreadDuration[i].QuadPart;
			}
			// update the total process duration
			QueryPerformanceCounter(&m_tJuliaProcessStop);
			m_tJuliaProcessDurationTotal.QuadPart = m_tJuliaProcessStop.QuadPart - m_tJuliaProcessStart.QuadPart;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case IDM_THREADCOMPLETEJULIAAVX64:
		if (0 == InterlockedDecrementAcquire(&m_iCalculatingJulia))
		{
			m_tJuliaProcessDurationTotal.QuadPart = 0;

			// kill the threads and update the total thread working duration
			for (i = 0; i < m_iJuliaThreadCount; ++i)
			{
				CloseHandle(m_hThreadJulia[i]);
				m_tJuliaThreadDurationTotal.QuadPart += m_tJuliaThreadDuration[i].QuadPart;
			}
			// update the total process duration
			QueryPerformanceCounter(&m_tJuliaProcessStop);
			m_tJuliaProcessDurationTotal.QuadPart = m_tJuliaProcessStop.QuadPart - m_tJuliaProcessStart.QuadPart;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;


	case IDM_THREADCOMPLETEJULIAAVX32:
		if (0 == InterlockedDecrementAcquire(&m_iCalculatingJulia))
		{
			m_tJuliaProcessDurationTotal.QuadPart = 0;

			// kill the threads and update the total thread working duration
			for (i = 0; i < m_iJuliaThreadCount; ++i)
			{
				CloseHandle(m_hThreadJulia[i]);
				m_tJuliaThreadDurationTotal.QuadPart += m_tJuliaThreadDuration[i].QuadPart;
			}
			// update the total process duration
			QueryPerformanceCounter(&m_tJuliaProcessStop);
			m_tJuliaProcessDurationTotal.QuadPart = m_tJuliaProcessStop.QuadPart - m_tJuliaProcessStart.QuadPart;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case IDM_THREADCOMPLETEJULIASSE:
		if (0 == InterlockedDecrementAcquire(&m_iCalculatingJulia))
		{
			m_tJuliaProcessDurationTotal.QuadPart = 0;

			// kill the threads and update the total thread working duration
			for (i = 0; i < m_iJuliaThreadCount; ++i)
			{
				CloseHandle(m_hThreadJulia[i]);
				m_tJuliaThreadDurationTotal.QuadPart += m_tJuliaThreadDuration[i].QuadPart;
			}
			// update the total process duration
			QueryPerformanceCounter(&m_tJuliaProcessStop);
			m_tJuliaProcessDurationTotal.QuadPart = m_tJuliaProcessStop.QuadPart - m_tJuliaProcessStart.QuadPart;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case IDM_THREADCOMPLETEJULIASSE2:
		if (0 == InterlockedDecrementAcquire(&m_iCalculatingJulia))
		{
			m_tJuliaProcessDurationTotal.QuadPart = 0;

			// kill the threads and update the total thread working duration
			for (i = 0; i < m_iJuliaThreadCount; ++i)
			{
				CloseHandle(m_hThreadJulia[i]);
				m_tJuliaThreadDurationTotal.QuadPart += m_tJuliaThreadDuration[i].QuadPart;
			}
			// update the total process duration
			QueryPerformanceCounter(&m_tJuliaProcessStop);
			m_tJuliaProcessDurationTotal.QuadPart = m_tJuliaProcessStop.QuadPart - m_tJuliaProcessStart.QuadPart;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
/*
		// note: the julia calculation does not kill the threads.  they are reused for subsequent calculations
		if (0 == InterlockedDecrementAcquire(&m_iCalculatingJulia))
		{
			// determine the total thread working duration
			for (i = 0, m_tJuliaThreadDurationTotal.QuadPart = 0; i < MAX_JULIA_THREADS; ++i)
			{
				m_tJuliaThreadDurationTotal.QuadPart += m_tJuliaThreadDuration[i].QuadPart;
			}

			// determine the total process working duration
			QueryPerformanceCounter(&m_tJuliaProcessStop);
			m_tJuliaProcessDurationTotal.QuadPart = m_tJuliaProcessStop.QuadPart - m_tJuliaProcessStart.QuadPart;

			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
*/
	case IDM_ABOUT:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), get_hWnd(), About);
		break;

	case IDM_EXIT:
		DestroyWindow(get_hWnd());
		break;

	case IDM_ITERATIONS_MAND_64:
		put_MaxIterationsMand(64);
		PostRecalcMand(); 
		break;

	case IDM_ITERATIONS_MAND_128: 
		put_MaxIterationsMand(128);
		PostRecalcMand();
		break;

	case IDM_ITERATIONS_MAND_256: 
		put_MaxIterationsMand(256);
		PostRecalcMand();
		break;

	case IDM_ITERATIONS_MAND_512: 
		put_MaxIterationsMand(512);
		PostRecalcMand();
		break;

	case IDM_ITERATIONS_MAND_1024: 
		put_MaxIterationsMand(1024);
		PostRecalcMand();
		break;

	case IDM_ITERATIONS_MAND_2048: 
		put_MaxIterationsMand(2048);
		PostRecalcMand();
		break;

	case IDM_ITERATIONS_MAND_4096:
		put_MaxIterationsMand(4096);
		PostRecalcMand();
		break;

	case IDM_ITERATIONS_MAND_8192: 
		put_MaxIterationsMand(8192);
		PostRecalcMand();
		break;

	case IDM_ITERATIONS_MAND_16384: 
		put_MaxIterationsMand(16384);
		PostRecalcMand();
		break;

	case IDM_ITERATIONS_MAND_32767: 
		put_MaxIterationsMand(32768);
		PostRecalcMand();
		break;

	case IDM_ITERATIONS_JULIA_64:
		put_MaxIterationsJulia(64);
		PostRecalcJulia();
		break;

	case IDM_ITERATIONS_JULIA_128: 
		put_MaxIterationsJulia(128);
		PostRecalcJulia();
		break;

	case IDM_ITERATIONS_JULIA_256: 
		put_MaxIterationsJulia(256);
		PostRecalcJulia();
		break;

	case IDM_ITERATIONS_JULIA_512: 
		put_MaxIterationsJulia(512);
		PostRecalcJulia();
		break;

	case IDM_ITERATIONS_JULIA_1024: 
		put_MaxIterationsJulia(1024);
		PostRecalcJulia();
		break;

	case IDM_ITERATIONS_JULIA_2048: 
		put_MaxIterationsJulia(2048);
		PostRecalcJulia();
		break;

	case IDM_ITERATIONS_JULIA_4096:
		put_MaxIterationsJulia(4096);
		PostRecalcJulia();
		break;

	case IDM_ITERATIONS_JULIA_8192: 
		put_MaxIterationsJulia(8192);
		PostRecalcJulia();
		break;

	case IDM_ITERATIONS_JULIA_16384: 
		put_MaxIterationsJulia(16384);
		PostRecalcJulia();
		break;

	case IDM_ITERATIONS_JULIA_32767: 
		put_MaxIterationsJulia(32768);
		PostRecalcJulia();
		break;


	default:
		return false;
	}
	return true;
}

void CJuliasmApp::PostRecalcMand(void)
{
	PostMessage(get_hWnd(), WM_COMMAND, IDM_RECALCULATE_MAND, 0);
}
void CJuliasmApp::PostRecalcJulia(void)
{
	PostMessage(get_hWnd(), WM_COMMAND, IDM_RECALCULATE_JULIA , 0);
}
void CJuliasmApp::PostRecalcAll(void)
{
	PostRecalcMand();
	PostRecalcJulia();
}

//
// handle keydown messages - not currently needed
//
bool CJuliasmApp::handle_keydown(HWND hWnd, int iVKey) 
{ 
	return false; 
}

//
// handle characters presses.  not currently needed
//
bool CJuliasmApp::handle_char(HWND hWnd, int iChar) 
{ 
	return false; 
}

//
// handle vertical scroll messages.  not currently needed
//
bool CJuliasmApp::handle_vscroll(HWND hWnd, int iScrollRequest, int iScrollPosition) 
{ 
	return false; 
}

//
// handle window resizing
//
bool CJuliasmApp::handle_size(HWND hWnd, HDC hdc, int iSizeType, int iWidth, int iHeight) 
{ 
	// only respond to messages if the right type of resize messages are sent
	if (iSizeType == SIZE_MAXIMIZED || iSizeType == SIZE_MAXSHOW || iSizeType == SIZE_RESTORED)
	{
		// make sure the image with is a multiple of POINTS_CONCURRENT_MAX 
		m_iMandHeight = m_iMandWidth = (iWidth / MANDELBROT_SCREEN_FRACTION) - (iWidth / MANDELBROT_SCREEN_FRACTION % POINTS_CONCURRENT_MAX);
		m_iJuliaHeight = iHeight;
		m_iJuliaWidth = iWidth - m_iMandWidth;

		// RAP: consider adding logic to make sure pixels are square. the delta should be the smaller of da and db

		// resize the bitmap images
		m_bmpMandelbrot.Resize(hWnd, m_iMandWidth, m_iMandHeight);
		m_bmpJulia.Resize(hWnd, m_iJuliaWidth, m_iJuliaHeight);

		// redraw the images
		PostMessage(hWnd, WM_COMMAND, IDM_RECALCULATE_MAND, 0);
		return true;
	}
	return false;
}

// 
// calculates the julia set using current parameters
//
bool CJuliasmApp::RecalculateJulia(void)
{
	// only recalculate if there is not an ongoing calculation
	if (m_iCalculatingJulia != 0)
	{
		MessageBeep(-1);
		return false;
	}

	QueryPerformanceCounter(&m_tJuliaProcessStart);
	m_tJuliaProcessDurationTotal.QuadPart = 0;
	m_tJuliaProcessStop.QuadPart = 0;
	m_tJuliaThreadDurationTotal.QuadPart = 0;
	SecureZeroMemory(&m_tJuliaThreadDuration, sizeof(m_tJuliaThreadDuration));

	if (m_CalcPlatformJulia == CalcPlatform::x87)
	{
		// save the platform type
		m_szMethod = "x87";

		// create threads to perform the calculation
		SecureZeroMemory(m_hThreadJulia, sizeof(m_hThreadJulia));
		m_iJuliaThreadCount = MAX_JULIA_THREADS;
		for (int i = 0; i < m_iJuliaThreadCount; ++i)
		{
			InterlockedIncrement(&m_iCalculatingJulia);
			m_ThreadInfoJuliaX87[i].iThreadIndex = i;
			m_ThreadInfoJuliaX87[i].pApp = this;
			m_hThreadJulia[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateJuliaX87, (LPVOID)&m_ThreadInfoJuliaX87[i], 0, &m_dwThreadJuliaIDX87[i]);
		}

	}
	else if (m_CalcPlatformJulia == CalcPlatform::SSE)
	{
		// save the platform type
		m_szMethod = "SSE";

		// create threads to perform the calculation
		SecureZeroMemory(m_hThreadJulia, sizeof(m_hThreadJulia));
		m_iJuliaThreadCount = MAX_JULIA_THREADS;
		for (int i = 0; i < m_iJuliaThreadCount; ++i)
		{
			InterlockedIncrement(&m_iCalculatingJulia);
			m_ThreadInfoJulia[i].iThreadIndex = i;
			m_ThreadInfoJulia[i].pApp = this;
			m_hThreadJulia[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateJuliaSSE, (LPVOID)&m_ThreadInfoJulia[i], 0, &m_dwThreadJuliaID[i]);
		}

	}
	else if (m_CalcPlatformJulia == CalcPlatform::SSE2)
	{
		// save the platform type
		m_szMethod = "SSE2";

		// create threads to perform the calculation
		SecureZeroMemory(m_hThreadJulia, sizeof(m_hThreadJulia));
		m_iJuliaThreadCount = MAX_JULIA_THREADS;
		for (int i = 0; i < m_iJuliaThreadCount; ++i)
		{
			InterlockedIncrement(&m_iCalculatingJulia);
			m_ThreadInfoJulia[i].iThreadIndex = i;
			m_ThreadInfoJulia[i].pApp = this;
			m_hThreadJulia[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateJuliaSSE2, (LPVOID)&m_ThreadInfoJulia[i], 0, &m_dwThreadJuliaID[i]);
		}

	}
	else if (m_CalcPlatformJulia == CalcPlatform::AVX64)
	{
		// save the platform type
		m_szMethod = "AVX 64-bit";

		// create threads to perform the calculation
		SecureZeroMemory(m_hThreadJulia, sizeof(m_hThreadJulia));
		m_iJuliaThreadCount = MAX_JULIA_THREADS;
		for (int i = 0; i < m_iJuliaThreadCount; ++i)
		{
			InterlockedIncrement(&m_iCalculatingJulia);
			m_ThreadInfoJuliaAVX64[i].iThreadIndex = i;
			m_ThreadInfoJuliaAVX64[i].pApp = this;
			m_hThreadJulia[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateJuliaAVX64, (LPVOID)&m_ThreadInfoJuliaAVX64[i], 0, &m_dwThreadJuliaIDAVX64[i]);
		}

	}
	else
	{
		// save the platform type
		m_szMethod = "AVX 32-bit";

		// create threads to perform the calculation
		SecureZeroMemory(m_hThreadJulia, sizeof(m_hThreadJulia));
		m_iJuliaThreadCount = MAX_JULIA_THREADS;
		for (int i = 0; i < m_iJuliaThreadCount; ++i)
		{
			InterlockedIncrement(&m_iCalculatingJulia);
			m_ThreadInfoJuliaAVX32[i].iThreadIndex = i;
			m_ThreadInfoJuliaAVX32[i].pApp = this;
			m_hThreadJulia[i] = CreateThread(NULL, THREAD_STACK_SIZE, CalculateJuliaAVX32, (LPVOID)&m_ThreadInfoJuliaAVX32[i], 0, &m_dwThreadJuliaIDAVX32[i]);
		}

	}
	return true;
}
//
// handlemounse moves
//
bool CJuliasmApp::handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y) 
{ 
	// don't respond to mouse moves if the image is currently recalculating
	if (m_iCalculatingJulia || m_iCalculatingMandelbrot)
		return false;

	// make sure the correct number of threads is being used
	m_iJuliaThreadCount = MAX_JULIA_THREADS;

	// adjust the location of the display to the correct location
	y = y - JULIA_TOP;

	// determine the numerical (not pixel) location of the mouse pointer
	m_ja_sse = (float)(m_a1 + (m_a2 - m_a1) / m_iMandWidth * x);
	m_jb_sse = (float)(m_b1 + (m_b2 - m_b1) / m_iMandHeight * y);

	return RecalculateJulia();
}

//
// handle mouse wheel events
// The mouse wheel is used to zoom into the mandelbrot set image.
// RAP: consider using the mouse wheel to also zoom into the Julia set image
//
bool CJuliasmApp::handle_mousewheel(HWND hWnd, WORD wvKeys, int iRotationAmount, int x, int y)
{
	double height, width, da, db, anew, bnew;

	// only handle the message if the mandelbrot set is not currently being calculated
	if (m_iCalculatingMandelbrot == 0)
	{
		// determine how far the wheel has rotated
		iRotationAmount = iRotationAmount / WHEEL_DELTA;

		// adjust the location for the top of the image
		y = y - JULIA_TOP;

		// calculate the new image boundaries
		da = (m_a2 - m_a1) / m_iMandWidth;
		db = (m_b2 - m_b1) / m_iMandHeight;
		width = m_a2 - m_a1;
		height = m_b2 - m_b1;
		anew = m_a1 + da * x;
		bnew = m_b1 + db * y;

		// zoom into (or out of) the image
		if (iRotationAmount < 0)
		{
			width = width * 1.1;
			height = height * 1.1;
		}
		else
		{
			width = width * 0.95;
			height = height * 0.95;
		}
		m_a1 = anew - (width / 2.0);
		m_a2 = m_a1 + width;

		m_b1 = bnew - (height / 2.0);
		m_b2 = m_b1 + height;

		// request a redraw with the new parameters
		PostMessage(hWnd, WM_COMMAND, IDM_RECALCULATE_MAND, 0);
	}
	return true;
}

//
// Calculates the mandelbrot image based on the current calculation platform
//
void CJuliasmApp::CalculateMandelbrot(void)
{
	switch (get_CalcPlatformMand())
	{
	case None: // fall through
	case x87:
		StartMandelbrotx87(get_hWnd());
		break;

	case SSE:
		StartMandelbrotSSE(get_hWnd());
		break;

	case SSE2:
		StartMandelbrotSSE2(get_hWnd());
		break;

	case AVX32:
		StartMandelbrotAVX32(get_hWnd());
		break;

	case AVX64:
		StartMandelbrotAVX64(get_hWnd());
		break;

	case FMA:
		StartMandelbrotAVX2(get_hWnd());
		break;
	}
}


//
// Update the platofmr menu to indicate which calculation platform is being used
// for the calculation of Mandelbrot sets
//
void CJuliasmApp::UpdateCalcPlatformMenuMand(void)
{
	// convert the current platform into a menu item
	int iMenuItem = -1;
	switch (get_CalcPlatformMand())
	{
	case x87:	iMenuItem = IDM_CALCULATE_MAND_X87; break;
	case SSE:	iMenuItem = IDM_CALCULATE_MAND_SSE; break;
	case SSE2:	iMenuItem = IDM_CALCULATE_MAND_SSE2; break;
	case AVX32:	iMenuItem = IDM_CALCULATE_MAND_AVX32; break;
	case AVX64:	iMenuItem = IDM_CALCULATE_MAND_AVX64; break;
	case FMA:	iMenuItem = IDM_CALCULATE_MAND_FMA; break;
	default:	iMenuItem = 0; break;
	}

	// uncheck all of the platform menu items
	CheckMenuRadioItem(GetMenu(get_hWnd()), IDM_CALCULATE_MAND_X87, IDM_CALCULATE_MAND_FMA, iMenuItem, MF_BYCOMMAND);

}
//
// Update the platform menu to indicate which calculation platform is being used
// for the calculation of Julia sets
void CJuliasmApp::UpdateCalcPlatformMenuJulia(void)
{
	// convert the current platform into a menu item
	int iMenuItem = -1;
	switch (get_CalcPlatformJulia())
	{
	case x87:	iMenuItem = IDM_CALCULATE_JULIA_X87; break;
	case SSE:	iMenuItem = IDM_CALCULATE_JULIA_SSE; break;
	case SSE2:	iMenuItem = IDM_CALCULATE_JULIA_SSE2; break;
	case AVX32:	iMenuItem = IDM_CALCULATE_JULIA_AVX32; break;
	case AVX64:	iMenuItem = IDM_CALCULATE_JULIA_AVX64; break;
	case FMA:	iMenuItem = IDM_CALCULATE_JULIA_FMA; break;
	default:	iMenuItem = 0; break;
	}

	// uncheck all of the platform menu items
	int iError;
	HMENU hMenu = GetMenu(get_hWnd());
	if (hMenu != NULL)
	{
		if (0 == CheckMenuRadioItem(hMenu, IDM_CALCULATE_JULIA_X87, IDM_CALCULATE_JULIA_FMA, iMenuItem, MF_BYCOMMAND))
		{
			iError = GetLastError();		
		}
	}

}

void CJuliasmApp::put_MaxIterationsMand(int iMaxIterationsMand)
{
	// convert the current platform into a menu item
	int iMenuItem = -1;
	switch (iMaxIterationsMand)
	{
	case 64:	iMenuItem = IDM_ITERATIONS_MAND_64; break;
	case 128:	iMenuItem = IDM_ITERATIONS_MAND_128; break;
	case 256:	iMenuItem = IDM_ITERATIONS_MAND_256; break;
	case 512:	iMenuItem = IDM_ITERATIONS_MAND_512; break;
	case 1024:	iMenuItem = IDM_ITERATIONS_MAND_1024; break;
	case 2048:	iMenuItem = IDM_ITERATIONS_MAND_2048; break;
	case 4096:	iMenuItem = IDM_ITERATIONS_MAND_4096; break;
	case 8192:	iMenuItem = IDM_ITERATIONS_MAND_8192; break;
	case 16384:	iMenuItem = IDM_ITERATIONS_MAND_16384; break;
	case 32768:	iMenuItem = IDM_ITERATIONS_MAND_32767; break;
	default:	iMaxIterationsMand = 1024; iMenuItem = IDM_ITERATIONS_MAND_1024; break;
	}

	m_iMaxIterationsMand = iMaxIterationsMand;

	// uncheck all of the platform menu items
	CheckMenuRadioItem(GetMenu(get_hWnd()), IDM_ITERATIONS_MAND_64, IDM_ITERATIONS_MAND_32767, iMenuItem, MF_BYCOMMAND);
}
void CJuliasmApp::put_MaxIterationsJulia(int iMaxIterationsJulia)
{
	// convert the current platform into a menu item
	int iMenuItem = -1;
	switch (iMaxIterationsJulia)
	{
	case 64:	iMenuItem = IDM_ITERATIONS_JULIA_64; break;
	case 128:	iMenuItem = IDM_ITERATIONS_JULIA_128; break;
	case 256:	iMenuItem = IDM_ITERATIONS_JULIA_256; break;
	case 512:	iMenuItem = IDM_ITERATIONS_JULIA_512; break;
	case 1024:	iMenuItem = IDM_ITERATIONS_JULIA_1024; break;
	case 2048:	iMenuItem = IDM_ITERATIONS_JULIA_2048; break;
	case 4096:	iMenuItem = IDM_ITERATIONS_JULIA_4096; break;
	case 8192:	iMenuItem = IDM_ITERATIONS_JULIA_8192; break;
	case 16384:	iMenuItem = IDM_ITERATIONS_JULIA_16384; break;
	case 32768:	iMenuItem = IDM_ITERATIONS_JULIA_32767; break;
	default:	iMaxIterationsJulia = 1024; iMenuItem = IDM_ITERATIONS_JULIA_1024; break;
	}

	m_iMaxIterationsJulia = iMaxIterationsJulia;

	// uncheck all of the platform menu items
	CheckMenuRadioItem(GetMenu(get_hWnd()), IDM_ITERATIONS_JULIA_64, IDM_ITERATIONS_JULIA_32767, iMenuItem, MF_BYCOMMAND);
}

char *CJuliasmApp::get_CalcPlatformName(char *szBuf, size_t iLen, CalcPlatform cp)
{
	char *ptr;

	switch (cp)
	{
	case None: ptr = "None"; break;
	case x87: ptr = "x87"; break;
	case SSE: ptr = "SSE"; break;
	case SSE2: ptr = "SSE2"; break;
	case AVX32: ptr = "AVX 32-bit"; break;
	case AVX64: ptr = "AVX 64-bit"; break;
	case FMA: ptr = "AVX2"; break;
	default: ptr = "Undefined"; break;
	}
	strcpy_s(szBuf, iLen, ptr);
	return szBuf;
}

// Message handler for about box.
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