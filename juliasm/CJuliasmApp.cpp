#include "stdafx.h"

/*TThreadInfo CJuliasmApp::m_ThreadInfoMand[MAX_MAND_THREADS];
TThreadInfo CJuliasmApp::m_ThreadInfoJulia[MAX_JULIA_THREADS];
*/
CJuliasmApp::CJuliasmApp() 
{
	Initialize();
};
CJuliasmApp::~CJuliasmApp() 
{
};

void CJuliasmApp::Initialize(void)
{
	m_iMandWidth = FRACTAL_WIDTH_DEFAULT;
	m_iMandHeight = FRACTAL_HEIGHT_DEFAULT;
	m_iJuliaWidth = JULIA_WIDTH_DEFAULT;
	m_iJuliaHeight = JULIA_HEIGHT_DEFAULT;
	m_iJuliaLeft = m_iMandWidth;

	InitializeMand();

	m_jc1_sse = -1.0;
	m_jc2_sse = 1.0;
	m_jd1_sse = -1.0;
	m_jd2_sse = 1.0;

	m_jiMaxIterations = 1024;
	m_iJMaxIter = 0;
	m_iJMaxThread = 0;

	m_iMandelbrotThreads = 0;
	m_iJuliaThreads = 0;

	for (int i = 0; i < _countof(m_iJuliaReady); ++i)
	{
		m_iJuliaReady[i] = 0;
	}


	m_iCalculatingJulia = 0;
	m_iCalculatingMandelbrot = 0;
	m_szMethod = "Undefined";

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

	QueryPerformanceFrequency(&m_ticksPerSecond);

	ZeroMemory(m_iJuliaReady, sizeof(m_iJuliaReady));

	m_CalcPlatformMand = CalcPlatform::None;
}

void CJuliasmApp::InitializeMand(void)
{
	m_bSaveOrbit = 0;
	m_iOrbitIndex = 0;;
	m_iOrbitPoints = 1024;
	//	m_orbit_c_sse[1024];
	//	m_orbit_d_sse[1024];

	// bounding box
	m_a1 = -2.0f;
	m_a2 = 2.0f;
	m_b1 = -2.0f;
	m_b2 = 2.0f;

	// per-pixel offsets in the horizontal (da) and vertical (db) directions
	m_iMaxIterations = 1024;
}

bool CJuliasmApp::handle_lbuttondoubleclick(HWND hWnd, WPARAM wvKeyDown, WORD x, WORD y)
{
	double height, width, da, db, anew, bnew;

	// only proceed if there are no mandelbrot calculations in the works
	if (m_iCalculatingMandelbrot == 0)
	{
		// adjust the y value for the top of the mandelbrot image
		y = y - JULIA_TOP;

		da = (m_a2 - m_a1) / m_iMandWidth;
		db= (m_b2 - m_b1) / m_iMandHeight;
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

/* RAP is this still needed?
if (da < 0.00001)
			PostMessage(hWnd, WM_COMMAND, IDM_CALCULATE_MAND_SSE2, 0);
		else
			PostMessage(hWnd, WM_COMMAND, IDM_CALCULATE_MAND_SSE, 0);
		return true;
		*/
	}
	return false;
}

bool CJuliasmApp::handle_paint(HWND hWnd, HDC hdc, LPPAINTSTRUCT ps) 
{ 
	int iLines = 0;
	char szBuf[256];

	if (m_iCalculatingMandelbrot == 0)
	{
		iLines = m_bmpMandelbrot.Show(hdc, 0, 50);
	}

	if (!m_iCalculatingJulia)
	{
		iLines = m_bmpJulia.Show(hdc, m_iMandWidth, 50);
	}
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

	return true;
}
bool CJuliasmApp::handle_create(HWND hWnd, LPCREATESTRUCT *lpcs) 
{ 
	// create the julia thread pool
	for (int i = 0; i < MAX_JULIA_THREADS; ++i)
	{
		m_ThreadInfoJulia[i].iThreadIndex = i;
		m_ThreadInfoJulia[i].pApp = this;
		m_hThreadJulia[i] = CreateThread(NULL, 1024, CalculateJuliaAVX, (LPVOID)&m_ThreadInfoJulia[i], 0, &m_dwThreadJuliaID[i]);
	}
	return true;

}

void CJuliasmApp::StartMandelbrotx87(HWND hWnd)
{
	m_szMethod = "x87";
	CalculateFractalX87();
	InvalidateRect(hWnd, NULL, FALSE);
}
void CJuliasmApp::StartMandelbrotSSE(HWND hWnd)
{
	if (m_iCalculatingMandelbrot != 0)
	{
		MessageBeep(-1);
		return;
	}

	m_iMandelbrotThreads = MAX_MAND_THREADS;
	SecureZeroMemory(m_hThreadMandelbrotSSE, sizeof(m_hThreadMandelbrotSSE));
	SecureZeroMemory(&m_tMandelbrotStart, sizeof(m_tMandelbrotStart));
	SecureZeroMemory(&m_tMandelbrotStop, sizeof(m_tMandelbrotStop));
	SecureZeroMemory(&m_tMandelbrotTotal, sizeof(m_tMandelbrotTotal));
	QueryPerformanceCounter(&m_tMandelbrotStart);
	m_szMethod = "SSE";

	for (int i = 0; i < m_iMandelbrotThreads; ++i)
	{
		InterlockedIncrement(&m_iCalculatingMandelbrot);
		m_ThreadInfoMand[i].iThreadIndex = i;
		m_ThreadInfoMand[i].pApp = this;
		m_hThreadMandelbrotSSE[i] = CreateThread(NULL, 1024, CalculateFractalSSE, (LPVOID)&m_ThreadInfoMand[i], 0, NULL);
	}
}
void CJuliasmApp::StartMandelbrotSSE2(HWND hWnd)
{
	if (m_iCalculatingMandelbrot != 0)
	{
		MessageBeep(-1);
		return;
	}
	m_szMethod = "SSE2";
	m_iMandelbrotThreads = MAX_MAND_THREADS;
	QueryPerformanceCounter(&m_tMandelbrotStart);
	for (int i = 0; i < m_iMandelbrotThreads; ++i)
	{
		InterlockedIncrement(&m_iCalculatingMandelbrot);
		m_ThreadInfoMand[i].iThreadIndex = i;
		m_ThreadInfoMand[i].pApp = this;
		m_hThreadMandelbrotSSE[i] = CreateThread(NULL, 1024, CalculateFractalSSE2, (LPVOID)&m_ThreadInfoMand[i], 0, NULL);
	}
}
void CJuliasmApp::StartMandelbrotAVX(HWND hWnd)
{
	MessageBox(hWnd, "Function not implemented.", "Error", MB_ICONINFORMATION | MB_OK);
}
void CJuliasmApp::StartMandelbrotAVX2(HWND hWnd)
{
	MessageBox(hWnd, "Function not implemented.", "Error", MB_ICONINFORMATION | MB_OK);
}

bool CJuliasmApp::handle_command(HWND hWnd, int wmID, int wmEvent) 
{ 
	int i;

	switch (wmID)
	{
	case IDM_CALCULATE_MAND_SSE:
		StartMandelbrotSSE(hWnd);
		break;

	case IDM_CALCULATE_MAND_SSE2:
		StartMandelbrotSSE2(hWnd);
		break;

	case IDM_CALCULATE_MAND_X87:
		StartMandelbrotx87(hWnd);
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

	case IDM_THREADCOMPLETE:
		InterlockedDecrement(&m_iCalculatingMandelbrot);
		if (m_iCalculatingMandelbrot == 0)
		{
			// kill the threads
			for (i = 0; i < m_iMandelbrotThreads; ++i)
			{
				CloseHandle(m_hThreadMandelbrotSSE[i]);
			}
			QueryPerformanceCounter(&m_tMandelbrotStop);
			m_tMandelbrotTotal.QuadPart = m_tMandelbrotStop.QuadPart - m_tMandelbrotStart.QuadPart;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case IDM_THREADCOMPLETEJULIA:
		InterlockedDecrement(&m_iCalculatingJulia);
		for (i = 0, m_iTotalTicks.QuadPart = 0; i < MAX_JULIA_THREADS; ++i)
			m_iTotalTicks.QuadPart += m_iTicks[i].QuadPart;

		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case IDM_ABOUT:
	//RAP	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
		break;

	case IDM_EXIT:
		DestroyWindow(get_hWnd());
		break;

	case IDM_ITERATIONS_64: m_jiMaxIterations = m_iMaxIterations = 64; break;
	case IDM_ITERATIONS_128: m_jiMaxIterations = m_iMaxIterations = 128; break;
	case IDM_ITERATIONS_256: m_jiMaxIterations = m_iMaxIterations = 256; break;
	case IDM_ITERATIONS_512: m_jiMaxIterations = m_iMaxIterations = 512; break;
	case IDM_ITERATIONS_1024: m_jiMaxIterations = m_iMaxIterations = 1024; break;
	case IDM_ITERATIONS_2048: m_jiMaxIterations = m_iMaxIterations = 2048; break;
	case IDM_ITERATIONS_4096: m_jiMaxIterations = m_iMaxIterations = 4096; break;
	case IDM_ITERATIONS_8192: m_jiMaxIterations = m_iMaxIterations = 8192; break;
	case IDM_ITERATIONS_16384: m_jiMaxIterations = m_iMaxIterations = 16384; break;
	case IDM_ITERATIONS_32767: m_jiMaxIterations = m_iMaxIterations = 32768; break;

	default:
		return false;
	}
	return true;
}
bool CJuliasmApp::handle_keydown(HWND hWnd, int iVKey) 
{ 
	return false; 
}
bool CJuliasmApp::handle_char(HWND hWnd, int iChar) 
{ 
	return false; 
}
bool CJuliasmApp::handle_vscroll(HWND hWnd, int iScrollRequest, int iScrollPosition) 
{ 
	return false; 
}
bool CJuliasmApp::handle_size(HWND hWnd, HDC hdc, int iSizeType, int iWidth, int iHeight) 
{ 
	if (iSizeType == SIZE_MAXIMIZED || iSizeType == SIZE_MAXSHOW || iSizeType == SIZE_RESTORED)
	{
		m_iMandHeight = m_iMandWidth = (iWidth / 4) - (iWidth / 4 % 4);
		m_iJuliaHeight = iHeight;
		m_iJuliaWidth = iWidth - m_iMandWidth;

		m_bmpMandelbrot.Resize(hWnd, m_iMandWidth, m_iMandHeight);
		m_bmpJulia.Resize(hWnd, m_iJuliaWidth, m_iJuliaHeight);
		return true;
	}
	return false;
}
bool CJuliasmApp::handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y) 
{ 
	int i;

	if (m_iCalculatingJulia || m_iCalculatingMandelbrot)
		return false;

	m_iJuliaThreads = MAX_JULIA_THREADS;

	y = y - JULIA_TOP;

	m_ja_sse = (float)m_a1 + (m_a2 - m_a1) / m_iMandWidth * x;
	m_jb_sse = (float)m_b1 + (m_b2 - m_b1) / m_iMandHeight * y;
	for (i = 0; i < m_iJuliaThreads; ++i)
	{
		InterlockedIncrement(&m_iCalculatingJulia);
		while (m_iJuliaReady[i] == 0)
		{
			Sleep(100);
		}
		if (0 == PostThreadMessage(m_dwThreadJuliaID[i], WM_COMMAND, 1, 0))
		{
			MessageBox(hWnd, "PostThreadMessage failed.", "Calculate Julia", MB_OK);
		}
	}
	return true;
}
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

	case AVX:
		StartMandelbrotAVX(get_hWnd());
		break;

	case AVX2:
		StartMandelbrotAVX2(get_hWnd());
		break;
	}
}

DWORD WINAPI CJuliasmApp::CalculateJuliaAVX(void* pArguments)
{
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;

	int iThread = pThreadInfo->iThreadIndex;

	//
	// wait for a message to calculate the picture
	//

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); // force the thread to create a message queue
	InterlockedIncrement(&pApp->m_iJuliaReady[iThread]);

restart:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		if (msg.message == WM_COMMAND && msg.wParam == 1)
			break;
	}

	if (iThread == 0)
	{
		pApp->m_iJMaxIter = 0;
	}
	LARGE_INTEGER iTicksStart;
	LARGE_INTEGER iTicksEnd;

	QueryPerformanceCounter(&iTicksStart);

	unsigned int* l_ppvBitsJulia = (unsigned int*)pApp->m_bmpJulia.get_bmpBits();


	//	SetThreadAffinityMask(GetCurrentThread(), 4);
	volatile int heightPixels = pApp->m_iJuliaHeight / pApp->m_iJuliaThreads;
	volatile int x, y;
	volatile __declspec(align(32)) float a, b, start_df;
	volatile float fdc = (pApp->m_jc2_sse - pApp->m_jc1_sse) / pApp->m_iJuliaWidth;
	volatile float fdd = (pApp->m_jd2_sse - pApp->m_jd1_sse) / pApp->m_iJuliaHeight;
	volatile float heightNumeric = (pApp->m_jd2_sse - pApp->m_jd1_sse) / pApp->m_iJuliaThreads;

	volatile 	int startY = iThread * heightPixels;
	volatile int stopY = startY + heightPixels;
	int i;


	const __declspec(align(32)) float a_avx[POINTS_CONCURRENT_AVX32] = { pApp->m_ja_sse, pApp->m_ja_sse, pApp->m_ja_sse, pApp->m_ja_sse, pApp->m_ja_sse, pApp->m_ja_sse, pApp->m_ja_sse, pApp->m_ja_sse };
	const __declspec(align(32)) float b_avx[POINTS_CONCURRENT_AVX32] = { pApp->m_jb_sse, pApp->m_jb_sse, pApp->m_jb_sse, pApp->m_jb_sse, pApp->m_jb_sse, pApp->m_jb_sse, pApp->m_jb_sse, pApp->m_jb_sse };
	__declspec(align(32)) float dc_avx[POINTS_CONCURRENT_AVX32] = { 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc };
	__declspec(align(32)) float ic_avx[POINTS_CONCURRENT_AVX32] = { pApp->m_jc1_sse, pApp->m_jc1_sse + 1.0f * fdc, pApp->m_jc1_sse + 2.0f * fdc, pApp->m_jc1_sse + 3.0f * fdc, pApp->m_jc1_sse + 4.0f * fdc, pApp->m_jc1_sse + 5.0f * fdc, pApp->m_jc1_sse + 6.0f * fdc, pApp->m_jc1_sse + 7.0f * fdc };
	__declspec(align(32)) float c_avx[POINTS_CONCURRENT_AVX32];
	__declspec(align(32)) float d_avx[POINTS_CONCURRENT_AVX32];

	const __declspec(align(32)) float maximum_avx[POINTS_CONCURRENT_AVX32] = { 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f };
	__declspec(align(32)) float iterations_avx[POINTS_CONCURRENT_AVX32];
	const __declspec(align(32)) float zero_avx[POINTS_CONCURRENT_AVX32] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	//	const __declspec(align(32)) float increment_avx[POINTS_CONCURRENT_AVX32] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	__declspec(align(32)) float increment_avx[POINTS_CONCURRENT_AVX32];

	a = pApp->m_ja_sse;
	b = pApp->m_jb_sse;


	start_df = pApp->m_jd1_sse + heightNumeric * iThread;

	// load the maximum magnitude into xmm7
	__m256 max = _mm256_load_ps(maximum_avx);

	//	__asm vmovaps ymm7, ymmword ptr[maximum_avx]

	// save the c increment
	__m256 delta_c = _mm256_load_ps(dc_avx);

	// save the initial C
	__m256 initial_c = _mm256_load_ps((const float*)ic_avx);

	// load a into ymm3
	__m256 curr_a = _mm256_load_ps(a_avx);

	// load b into ymm4
	__m256 curr_b = _mm256_load_ps(b_avx);

	// load the zero array
	__m256 zero = _mm256_load_ps(zero_avx);


	__declspec(align(32)) float mag_avx[POINTS_CONCURRENT_AVX32];

	for (y = startY; y < stopY; ++y)
	{
		// load the current d across all _avx values
		__m256 d = _mm256_broadcast_ss((const float*)&start_df);
		__m256 start_d = d;

		// load the initial value of c, and then store it in the current c array
		__m256 start_c = initial_c; // _mm256_load_ps((const float*)ic_avx);
		__m256 c = initial_c;

		for (x = 0; x < pApp->m_iJuliaWidth; x += POINTS_CONCURRENT_AVX32)
		{
			// initilize c, d, and the iteration count
			__m256 iterations = _mm256_setzero_ps();
			for (int ii = 0; ii < POINTS_CONCURRENT_AVX32; ++ii)
			{
				increment_avx[ii] = 4.0;
			}

			for (i = 0; i < pApp->m_jiMaxIterations; ++i)
			{

				// calculate 2 * c * d
				__m256 cd2 = _mm256_mul_ps(c, d);
				cd2 = _mm256_add_ps(cd2, cd2);

				// calculate c^2 + d^2
				__m256 c2 = _mm256_mul_ps(c, c);
				__m256 d2 = _mm256_mul_ps(d, d);
				__m256 mag = _mm256_add_ps(c2, d2);

				// compare the magnitude to the max allowable magnitude (2.0f)
				__m256 cmp = _mm256_cmp_ps(mag, max, _CMP_LT_OQ);
				unsigned int test = _mm256_movemask_ps(cmp);

				// increment the iterations
				__m256 increment = _mm256_and_ps(cmp, max);
				iterations = _mm256_add_ps(increment, iterations);

				// if there are no pixels left to increment, then all pixles are in the set.  time to quit
				if (test == 0)
					break;

				// generate the new c: c = c^2 - d^2 + a
				c = _mm256_sub_ps(c2, d2);
				c = _mm256_add_ps(c, curr_a);

				// generate the new d: 2 * c * d + b
				d = _mm256_add_ps(cd2, curr_b);

			} // next iteration

			// convert the iteration count to a color index and save to memory
			iterations = _mm256_div_ps(iterations, max);
			_mm256_store_ps(iterations_avx, iterations);

			for (int j = 0; j < POINTS_CONCURRENT_AVX32; ++j)
			{
				if (iterations_avx[j] >= pApp->m_jiMaxIterations)
					iterations_avx[j] = 0;
				((unsigned int*)l_ppvBitsJulia)[y * pApp->m_iJuliaWidth + x + j] = RGB(iterations_avx[j], iterations_avx[j] / 2, iterations_avx[j] / 3);
			}

			// generate the next set of 'c' values by adding the c_increment to the current c
			c = _mm256_add_ps(start_c, delta_c);
			start_c = c;
			d = start_d;
		} // next x
		start_df = start_df + fdd;
	} // next y
	QueryPerformanceCounter(&iTicksEnd);
/*RAP need to implement this by initializeing the timer counts
if (pApp->m_ticksPerSecond.QuadPart == 0)
	{
		MessageBox(hWnd, "m_ticksPerSecond is not set.", "Error", MB_ICONERROR, MB_OK);
		exit(0);
	}
	pApp->m_iTicks[iThread].QuadPart = (iTicksEnd.QuadPart - iTicksStart.QuadPart) * 1000 / pApp->m_ticksPerSecond.QuadPart;
	*/
	PostMessage(get_hwnd(), WM_COMMAND, IDM_THREADCOMPLETEJULIA, 0);
	goto restart;
	return 0;
}
// #pragma optimize("", on )


//unsigned __stdcall CalculateFractalSSE( void* pArguments )
DWORD WINAPI CJuliasmApp::CalculateFractalSSE(void* pArguments)
{
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	pApp->m_CalcPlatformMand = CalcPlatform::SSE;


	int iThread = pThreadInfo->iThreadIndex;

	double fThreadHeight = (pApp->m_b2 - pApp->m_b1) / pApp->m_iMandelbrotThreads;

	__declspec(align(16)) float a_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float b_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float maximum_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float iterations_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float da_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float db_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float a1_sse[POINTS_CONCURRENT_SSE];

	double
		_a = pApp->m_a1,
		_b = pApp->m_b1 + fThreadHeight * iThread,
		da = (pApp->m_a2 - pApp->m_a1) / pApp->m_iMandWidth,
		db = (pApp->m_b2 - pApp->m_b1) / pApp->m_iMandHeight;

	pApp->m_da = da;
	pApp->m_db = db;

	for (int i = 0; i < POINTS_CONCURRENT_SSE; ++i)
	{
		maximum_sse[i] = MAXIMUM_MAND_SSE;
		b_sse[i] = (float)_b;
		db_sse[i] = (float)db;
		// load an array of the first a's
		a1_sse[i] = (float)_a;
		_a += da;
		da_sse[i] = (float)da * POINTS_CONCURRENT_SSE;
	}

	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);

	// load the imaginary component
	__asm movaps xmm4, oword ptr[b_sse]

	// load the maximum vector size
	__asm movaps xmm7, oword ptr[maximum_sse]

	// get the bitmap bits
	unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpMandelbrot.get_bmpBits();

	int pixelHeight = pApp->m_iMandHeight / pApp->m_iMandelbrotThreads;
	int starty = iThread * pixelHeight;
	int endy = starty + pixelHeight;

	unsigned int iIndex = 0 + starty * pApp->m_iMandWidth;

	for (int y = starty; y < endy; ++y)
	{
		// load the real component
		__asm movaps xmm3, a1_sse

		for (int x = 0; x < pApp->m_iMandWidth; x += POINTS_CONCURRENT_SSE)
		{

			//			memset(orbit_c_sse, 0, sizeof(orbit_c_sse));
			pApp->CalculatePointsSSE();

			__asm movaps oword ptr[iterations_sse], xmm6

			// normalize the iteration values between 0 and 255
			l_ppvBits[iIndex++] = (iterations_sse[0] == pApp->m_iMaxIterations) ? 0 : pApp->m_PaletteDefault.get_Color(iterations_sse[0]);
			l_ppvBits[iIndex++] = (iterations_sse[1] == pApp->m_iMaxIterations) ? 0 : pApp->m_PaletteDefault.get_Color(iterations_sse[1]);
			l_ppvBits[iIndex++] = (iterations_sse[2] == pApp->m_iMaxIterations) ? 0 : pApp->m_PaletteDefault.get_Color(iterations_sse[2]);
			l_ppvBits[iIndex++] = (iterations_sse[3] == pApp->m_iMaxIterations) ? 0 : pApp->m_PaletteDefault.get_Color(iterations_sse[3]);

			// update the real components
			__asm {
				addps xmm3, oword ptr[da_sse]
			}
		}
		// update the imaginary components
		__asm addps xmm4, oword ptr[db_sse]
	}
	QueryPerformanceCounter(&tStop);
	pApp->m_tTotal.QuadPart = tStop.QuadPart - tStart.QuadPart;

	HWND hWnd = get_hwnd();
	PostMessage(hWnd, WM_COMMAND, IDM_THREADCOMPLETE, 0);

	return 0;
}


// inputs
// a's in xmm3
// b's in xmm4
// maximums (e.g. 4) in xmm7
//
// outputs
// returns the iteration counts in XMM6
//

#define SSE_MAX xmm7
#define SSE_A xmm3
#define SSE_B xmm4
#define SSE_C xmm0 
#define SSE_C_SQU xmm0
#define SSE_D xmm1 
#define SSE_D_SQU xmm1
#define SSE_CD xmm2
#define SSE_2CD SSE_CD
#define SSE_ITERATIONS xmm6
#define SSE_RESULT xmm5
#define SSE_COUNT_MASK SSE_RESULT
void CJuliasmApp::CalculatePointsSSE(void)
{
	int tmp = this->m_iMaxIterations;
	__asm {
		// place the maximum number of iterations in AX
//		mov ax, WORD PTR m_iMaxIterations
		mov ax, WORD PTR tmp

		mov bl, BYTE PTR m_bSaveOrbit
		xor edx, edx	// reset the orbit save index

		// place 0 in c, d, and iterations
		xorps SSE_C, SSE_C						// xmm0
		xorps SSE_D, SSE_D						// xmm1
		xorps SSE_ITERATIONS, SSE_ITERATIONS	//xmm6

		iterate :
			// calculate c * d and place in xmm2
			movaps SSE_CD, SSE_C	// c  n(xmm2, xmm0)
			mulps SSE_CD, SSE_D		// * d (xmm2, xmm1)
			addps SSE_2CD, SSE_CD	// * 2 (by adding c * d to itself (xmm2, xmm2)

			// save the orbit point
			//		test bl,bl
			//		jnz save_orbit
			save_orbit_resume :
			mulps SSE_C, SSE_C		// calulate c^2 (xmm0, xmm0)
			mulps SSE_D, SSE_D	// calculate d^2    (xmm1, xmm1)

			// calculate the magnitude of the n^2 new values and store them in XMM5
			movaps SSE_RESULT, SSE_C_SQU	// add c^2 to the result (xmm5, xmm0)
			addps SSE_RESULT, SSE_D_SQU		// add d^2 to the result xmm5, xmm1)
			subps SSE_C_SQU, SSE_D_SQU		// calculate c^2-d^2 (xmm0, xmm1)

			movaps SSE_D, SSE_2CD // get the new d; d(n+1) = 2 * c(n) * d(n); (xmm1, xmm2) 
			addps SSE_C_SQU, SSE_A	// add 'a' to c^2; (xmm1, xmm3)
			addps SSE_D_SQU, SSE_B	// add 'b' to d^2; (xmm2, xmm4)

			// compare the previous iteration to the maximum value
			cmpltps SSE_RESULT, SSE_MAX		// campare to max range
			pmovmskb ecx, SSE_RESULT
			andps SSE_RESULT, SSE_MAX		// use result as bitmask for counter to generate increment
			addps SSE_ITERATIONS, SSE_COUNT_MASK		// add increment to counter

			test ecx, ecx
			jz done

				/*		cmp ax, 10
				jne tmp
				nop

				tmp:
				*/
			dec ax
			jnz iterate
			jmp done

		save_orbit :
		/*
		unsigned short bSaveOrbit = 1;

		unsigned short iOrbitIndex = 0;;
		unsigned short iOrbitPoints = 128;

		float orbit_c_sse[128];
		float orbit_d_sse[128];
		*/
		// save the first value of C
		//		mov cx, DWORD PTR [orbit_c_sse + bh]

		cmp edx, 1024  // RAP: use the maximum iteration count here instead of a constant
			jge save_orbit_resume
			movss DWORD PTR m_orbit_c_sse[4 * edx], xmm0
			movss DWORD PTR m_orbit_d_sse[4 * edx], xmm1
			inc edx;
		//		mov orbit_c_sse[ebx], 
		jmp save_orbit_resume

		done :
		// adjust the counts by 4
		divps xmm6, xmm7
	}
}

DWORD WINAPI CJuliasmApp::CalculateFractalSSE2(void* pArguments)
{
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	pApp->m_CalcPlatformMand = CalcPlatform::SSE2;


	int iThread = pThreadInfo->iThreadIndex;

	double fThreadHeight = (pApp->m_b2 - pApp->m_b1) / pApp->m_iMandelbrotThreads;

	__declspec(align(16)) double a_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double b_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double maximum_sse2[POINTS_CONCURRENT_SSE2] = { 4.0f, 4.0f };
	__declspec(align(16)) double iterations_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double da_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double db_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double a1_sse2[POINTS_CONCURRENT_SSE2];

	double
		_a = pApp->m_a1,
		_b = pApp->m_b1 + fThreadHeight * iThread,
		da = (pApp->m_a2 - pApp->m_a1) / pApp->m_iMandWidth,
		db = (pApp->m_b2 - pApp->m_b1) / pApp->m_iMandHeight;


	//	SetThreadAffinityMask(GetCurrentThread(), 1 << iThread);

	// RAP gotta implement this functionality
	//	::da = da;
	//::db = db;

	LARGE_INTEGER tStart, tStop;

	QueryPerformanceCounter(&tStart);

	b_sse2[0] = (double)_b;
	b_sse2[1] = (double)_b;

	db_sse2[0] = (double)db;
	db_sse2[1] = (double)db;

	// get the bitmap bits
	unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpMandelbrot.get_bmpBits();

	// load the imaginary component
	__asm movapd xmm4, oword ptr[b_sse2]

		// load the maximum vector size
		__asm movapd xmm7, oword ptr[maximum_sse2]

		// load an array of the first a's
		_a = pApp->m_a1;
	a1_sse2[0] = (double)_a; _a += da;
	a1_sse2[1] = (double)_a; _a += da;

	da_sse2[0] = (double)da * POINTS_CONCURRENT_SSE2;
	da_sse2[1] = (double)da * POINTS_CONCURRENT_SSE2;

	int pixelHeight = pApp->m_iMandHeight / pApp->m_iMandelbrotThreads;
	int starty = iThread * pixelHeight;
	int endy = starty + pixelHeight;

	unsigned int iIndex = 0 + starty * pApp->m_iMandWidth;

	for (int y = starty; y < endy; ++y)
	{
		// load the real component
		__asm movapd xmm3, a1_sse2

		for (int x = 0; x < pApp->m_iMandWidth; x += POINTS_CONCURRENT_SSE2)
		{

			//			memset(orbit_c_sse, 0, sizeof(orbit_c_sse));
			pApp->CalculatePointsSSE2();

			__asm movapd oword ptr[iterations_sse2], xmm6


			// normalize the iteration values between 0 and 255
			l_ppvBits[iIndex++] = (iterations_sse2[0] == pApp->m_iMaxIterations) ? 0 : pApp->m_PaletteDefault.get_Color(iterations_sse2[0]);
			l_ppvBits[iIndex++] = (iterations_sse2[1] == pApp->m_iMaxIterations) ? 0 : pApp->m_PaletteDefault.get_Color(iterations_sse2[1]);

			// update the real components
			__asm {
				addpd xmm3, oword ptr[da_sse2]
			}
		}
		// update the imaginary components
		__asm addpd xmm4, oword ptr[db_sse2]
	}
	QueryPerformanceCounter(&tStop);
	pApp->m_tTotal.QuadPart = tStop.QuadPart - tStart.QuadPart;

	PostMessage(pApp->get_hWnd(), WM_COMMAND, IDM_THREADCOMPLETE, 0);

	return 0;
}


void CJuliasmApp::CalculatePointsSSE2(void)
{
	int iMaxIterations = this->m_iMaxIterations;

	__asm {
		// place the maximum magnitude (4) in xmm7
		//		movapd xmm7, oword ptr [maximum_sse2]

		// place the maximum number of iterations in AX
		mov ax, WORD PTR iMaxIterations

			// place 0 in c and d
			xorpd xmm0, xmm0
			xorpd xmm1, xmm1
			xorpd xmm6, xmm6

			// load a and b
			//		movapd xmm3, oword ptr [a_sse2]
			//		movapd xmm4, oword ptr [b_sse2]

			align 16
		iterate:
		// calculate c * d and place in xmm2
		movapd xmm2, xmm0	// c
			mulpd xmm2, xmm1	// * d
			addpd xmm2, xmm2	// * 2

			// calulate c^2
			mulpd xmm0, xmm0

			// calculate d^2
			mulpd xmm1, xmm1

			//
			// calculate the magnitude of the n^2 new values and store them in XMM5
			//
			movapd xmm5, xmm0
			addpd xmm5, xmm1

			// calculate c^2-d^2
			subpd xmm0, xmm1

			// get the new d
			movapd xmm1, xmm2

			// add a to c^2
			addpd xmm0, xmm3

			// add b to d^2
			addpd xmm1, xmm4

			// compare the previous iteration to the maximum value
			cmpltpd xmm5, xmm7		// campare to max range
			pmovmskb ecx, xmm5
			andpd xmm5, xmm7		// use result as bitmask for counter to generate increment
			addpd xmm6, xmm5		// add increment to counter

			test ecx, ecx
			jz done
			dec ax
			jnz iterate
			jmp done

		done :
		// adjust the counts by 4
		divpd xmm6, xmm7
			//		movapd oword ptr[iterations_sse2], xmm6

	}
}


/*{
double _a = a1, _b = b1, da = (a2 - a1) / iMandWidth, db = (b2 - b1) / iMandHeight;
unsigned int iIndex = 0;

SetThreadAffinityMask(GetCurrentThread(), 2);

::da = da;
::db = db;

LARGE_INTEGER tStart, tStop;

QueryPerformanceCounter(&tStart);

for (int y = 0; y < iMandHeight; ++y)
{
b_sse2[0] = _b;
b_sse2[1] = _b;

_a = a1;

for (int x = 0; x < iMandWidth; x += 2)
{
a_sse2[0] = _a; _a += da;
a_sse2[1] = _a; _a += da;

CalculatePointsSSE2();

((unsigned int*)l_ppvBits)[iIndex++] = RGB(iterations_sse2[0], iterations_sse2[0]/2, iterations_sse2[0]/3);
((unsigned int*)l_ppvBits)[iIndex++] = RGB(iterations_sse2[1], iterations_sse2[1]/2, iterations_sse2[1]/3);
}

_b += db;
}
QueryPerformanceCounter(&tStop);
tTotal.QuadPart = tStop.QuadPart - tStart.QuadPart;

PostMessage(hWndMain, WM_COMMAND, IDM_THREADCOMPLETE, 0);
return 0;
}
*/

void CJuliasmApp::CalculateFractalX87(void)
{
	double _a = m_a1, _b = m_b1, da = (m_a2 - m_a1) / m_iMandWidth, db = (m_b2 - m_b1) / m_iMandHeight;
	unsigned int iIndex = 0;

	m_CalcPlatformMand = CalcPlatform::x87;


	LARGE_INTEGER tStart, tStop;

	QueryPerformanceCounter(&tStart);

	//get the bitmap bits
	unsigned int* l_ppvBits = (unsigned int*)m_bmpMandelbrot.get_bmpBits();

	m_da = da;
	m_db = db;

	_b = m_b1;
	for (int y = 0; y < m_iMandHeight; ++y)
	{
		_a = m_a1;
		for (int x = 0; x < m_iMandWidth; x += 1)
		{
			double c = 0.0f;
			double d = 0.0f;
			double c2, d2, cd2;
			double mag;
			int i;
			for (i = 0; i < m_iMaxIterations; ++i)
			{
				cd2 = 2 * c * d;
				c2 = c * c;
				d2 = d * d;
				mag = c2 + d2;
				c = c2 - d2 + _a;
				d = cd2 + _b;
				if (mag > 4.0)
					break;
			}
			_a += da;
			l_ppvBits[iIndex++] = (i == m_iMaxIterations) ? 0 : m_PaletteDefault.get_Color(i);
//			l_ppvBits[iIndex++] = RGB(log((double)i) / log((double)m_iMaxIterations) * 255, log((double)i) / log((double)m_iMaxIterations) * 512, log((double)i) / log((double)m_iMaxIterations) * 768);
		}
		_b += db;
	}
	QueryPerformanceCounter(&tStop);
	m_tTotal.QuadPart = tStop.QuadPart - tStart.QuadPart;
}
