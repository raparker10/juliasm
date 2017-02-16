#include "stdafx.h"

#define DEFAULT_MAND_CACLINFO_INDEX 4
#define DEFAULT_JULIA_CACLINFO_INDEX 13

TCalcInfo CJuliasmApp::m_ci[] = {
	{ FractalType::Mand, FractalFormula::FormMand, CalcPlatform::None, 0, NULL, 0, 0, 0, 0, IMAGE_MAND },
	{ FractalType::Mand, FractalFormula::FormMand, CalcPlatform::x87, MAX_CPU_THREADS, CalculateMandX87, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_MAND },
	{ FractalType::Mand, FractalFormula::FormMand, CalcPlatform::SSE, MAX_CPU_THREADS, CalculateMandSSE, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_MAND },
	{ FractalType::Mand, FractalFormula::FormMand, CalcPlatform::SSE2, MAX_CPU_THREADS, CalculateMandSSE2, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_MAND },
	{ FractalType::Mand, FractalFormula::FormMand, CalcPlatform::AVX32, MAX_CPU_THREADS, CalculateMandAVX32, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_MAND },
	{ FractalType::Mand, FractalFormula::FormMand, CalcPlatform::AVX64, MAX_CPU_THREADS, CalculateMandAVX64, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_MAND },
	{ FractalType::Mand, FractalFormula::FormMand, CalcPlatform::FMA, MAX_CPU_THREADS, CalculateMandFMA, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_MAND },
	{ FractalType::Mand, FractalFormula::FormMand, CalcPlatform::OpenCL_CPU, MAX_OCL_THREADS, CalculateMandOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_CPU, 0, IMAGE_MAND },
	{ FractalType::Mand, FractalFormula::FormMand, CalcPlatform::OpenCL_GPU, MAX_OCL_THREADS, CalculateMandOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_GPU, 7, IMAGE_MAND },

	{ FractalType::Julia, FractalFormula::FormMand, CalcPlatform::None, 0, NULL, 0, 0, 0, 0 },
	{ FractalType::Julia, FractalFormula::FormMand, CalcPlatform::x87, MAX_CPU_THREADS, CalculateJuliaX87, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_JULIA },
	{ FractalType::Julia, FractalFormula::FormMand, CalcPlatform::SSE, MAX_CPU_THREADS, CalculateJuliaSSE, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_JULIA },
	{ FractalType::Julia, FractalFormula::FormMand, CalcPlatform::SSE2, MAX_CPU_THREADS, CalculateJuliaSSE2, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_JULIA },
	{ FractalType::Julia, FractalFormula::FormMand, CalcPlatform::AVX32, MAX_CPU_THREADS, CalculateJuliaAVX32, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_JULIA },
	{ FractalType::Julia, FractalFormula::FormMand, CalcPlatform::AVX64, MAX_CPU_THREADS, CalculateJuliaAVX64, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_JULIA },
	{ FractalType::Julia, FractalFormula::FormMand, CalcPlatform::FMA, MAX_CPU_THREADS, CalculateJuliaFMA, WM_COMMAND, IDM_THREADCOMPLETE, 0, 0, IMAGE_JULIA },

	{ FractalType::Julia, FractalFormula::FormMand, CalcPlatform::OpenCL_CPU, MAX_OCL_THREADS, CalculateJuliaOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_CPU, 1, IMAGE_JULIA },
	{ FractalType::Julia, FractalFormula::FormMand, CalcPlatform::OpenCL_GPU, MAX_OCL_THREADS, CalculateJuliaOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_GPU, 1, IMAGE_JULIA },

	{ FractalType::Julia, FractalFormula::FormSin, CalcPlatform::OpenCL_CPU, MAX_OCL_THREADS, CalculateJuliaOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_CPU, 4, IMAGE_JULIA },
	{ FractalType::Julia, FractalFormula::FormSin, CalcPlatform::OpenCL_GPU, MAX_OCL_THREADS, CalculateJuliaOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_GPU, 4, IMAGE_JULIA },

	{ FractalType::Julia, FractalFormula::FormCos, CalcPlatform::OpenCL_CPU, MAX_OCL_THREADS, CalculateJuliaOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_CPU, 5, IMAGE_JULIA },
	{ FractalType::Julia, FractalFormula::FormCos, CalcPlatform::OpenCL_GPU, MAX_OCL_THREADS, CalculateJuliaOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_GPU, 5, IMAGE_JULIA },

	{ FractalType::Julia, FractalFormula::FormExp, CalcPlatform::OpenCL_CPU, MAX_OCL_THREADS, CalculateJuliaOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_CPU, 6, IMAGE_JULIA },
	{ FractalType::Julia, FractalFormula::FormExp, CalcPlatform::OpenCL_GPU, MAX_OCL_THREADS, CalculateJuliaOpenCL, WM_COMMAND, IDM_THREADCOMPLETE, CL_DEVICE_TYPE_GPU, 6, IMAGE_JULIA },
};


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
	int i;

	// setup the default fractal information
	m_pCalcInfo[FractalType::Mand] = get_CalcInfo(FractalType::Mand, FractalFormula::FormMand, CalcPlatform::AVX32);
	m_pCalcInfo[FractalType::Julia] = get_CalcInfo(FractalType::Julia, FractalFormula::FormMand, CalcPlatform::AVX32);

	// clear the screen positioning information
	m_iMandScreenPct = 25;	// 25%
	memset(&m_rcMand, 0, sizeof(m_rcMand));
	_m_iMandHeight = 0;
	_m_iMandWidth = 0;

	for (int i = 0; i < sizeof(m_bShowFractal) / sizeof(m_bShowFractal[0]); ++i)
		m_bShowFractal[i] = true;

	memset(&m_rcJulia, 0, sizeof(m_rcJulia));
	_m_iJuliaHeight = 0;
	_m_iJuliaWidth = 0;

	memset(&m_rcInfoPanel, 0, sizeof(m_rcInfoPanel));
	_m_iInfoPanelHeight = 0;
	_m_iInfoPanelWidth = 0;
	m_bShowInfoPanel = true;

	// initialize mandelbrot calculation variables
	// This functionality is packaged into a function so that 
	// it is easily called separately from the program menu
	InitializeMand();

	// default the calculation "platform": None, x87, SSE, ... etc ...
	put_CalcPlatform(m_pCalcInfo[FractalType::Mand]->iFracType, m_pCalcInfo[FractalType::Mand]->iFracFormula, m_pCalcInfo[FractalType::Mand]->iCalcPlatform);
	put_CalcPlatform(m_pCalcInfo[FractalType::Julia]->iFracType, m_pCalcInfo[FractalType::Julia]->iFracFormula, m_pCalcInfo[FractalType::Julia]->iCalcPlatform);

	// initialize julia set calculation variables
	m_jc1 = -1.0;
	m_jc2 = 1.0;
	m_jd1 = -1.0;
	m_jd2 = 1.0;

	m_OCLFrac.put_Boundary(m_pCalcInfo[FractalType::Julia]->iOCLKernelNumber, (float)m_jc1, (float)m_jd1, (float)m_jc2, (float)m_jd2);

	put_MaxIterationsJulia(1024);

	// initialize thread-handling variables
	///	for (i = 0; i < FractalType::NUMBER_FRACTAL_TYPES; ++i)
	///		m_iCalcThreadCount[FractalType::Mand] = 0;

	for (i = 0; i < _countof(m_iCalculatingFractal); ++i)
	{
		InterlockedAnd(&m_iCalculatingFractal[i], 0);
	}

	//
	// setup the palette
	//
	// setup the default palette

	// setup the red color channel
	m_PaletteDefault.PushColorPoint(0, CPixelPoint(0, 0));
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
	//	m_PaletteDefault.put_BlackAndWhite(false);
	m_PaletteDefault.UpdateColors();


	// initialize the high-resolution timing functionality
	QueryPerformanceFrequency(&m_ticksPerSecond);


	//
	// initialize the font information
	ZeroMemory(&m_ncm, sizeof(m_ncm));
	ZeroMemory(&m_lfInfo, sizeof(m_lfInfo));
	m_hfInfo = (HFONT)INVALID_HANDLE_VALUE;


	m_bUpdateJulia = true;


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
	m_OCLFrac.put_Boundary(IMAGE_MAND, (float)m_a1, (float)m_b1, (float)m_a2, (float)m_b2);

	// per-pixel offsets in the horizontal (da) and vertical (db) directions
	put_MaxIterationsMand(1024);
}
// Initialize variables used for mandelbrot set calculation.
// These are broken out separately so that they can be easily
// reset from the main menu.
void CJuliasmApp::InitializeJulia(void)
{

	// bounding box
	m_jc1 = -2.0f;
	m_jc2 = 2.0f;
	m_jd1 = -2.0f;
	m_jd2 = 2.0f;

	// per-pixel offsets in the horizontal (da) and vertical (db) directions
	put_MaxIterationsJulia(1024);

	assert(m_OCLFrac.get_KernelCount() > 0); // this should always be called after compilation.  might need to reorganize this
	for (int i = 0; i < m_OCLFrac.get_KernelCount(); ++i)
	{
		m_OCLFrac.put_Boundary(i, (float)m_jc1, (float)m_jd1, (float)m_jc2, (float)m_jd2);
	}

}

// handle a mouse double-click.
// Used to zoom into an image
//
bool CJuliasmApp::handle_lbuttondoubleclick(HWND hWnd, WPARAM wvKeyDown, WORD x, WORD y)
{
	POINT p;
	p.x = x;
	p.y = y;


	if (PtInRect(&m_rcMand, p))
	{
		ZoomInMand(x, y, 0.8f);
	}
	else if (PtInRect(&m_rcJulia, p))
	{
		ZoomInJulia(x, y, 0.8f);
	}

	return true;
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
	if ((0 == m_iCalculatingFractal[FractalType::Mand]) && get_ShowFractal(FractalType::Mand))
	{
		iLines = m_bmpFractal[FractalType::Mand].Show(hdc, 0, 0);
	}

	// only draw the julia set if it is not being calculated
	if ((0 == m_iCalculatingFractal[FractalType::Julia]) && get_ShowFractal(FractalType::Julia))
	{
		iLines = m_bmpFractal[FractalType::Julia].Show(hdc, get_MandWidth(), 0);
	}

	//
	// update the statistics being displayed on the screen
	//

	// clear the text panel areas
	RECT rc;
	GetClientRect(hWnd, &rc);
	rc.bottom = 0;
	FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

	// clear the area below the mandelbrot display
	GetClientRect(hWnd, &rc);
	rc.top = get_MandHeight();
	rc.right = get_MandWidth();
	FillRect(hdc, &m_rcInfoPanel, (HBRUSH)GetStockObject(WHITE_BRUSH));

	//
	// display information
	//
	int iLen;
	HFONT hOldFont = (HFONT)SelectObject(hdc, m_hfInfo);
	int y = m_rcInfoPanel.top; // JULIA_TOP + get_MandHeight();
	char szPlatform[16];

	// mandelbrot section header
	iLen = sprintf_s(
		szBuf,
		_countof(szBuf),
		"Mandelbrot - %s - %d iter",
		get_CalcPlatformName(
		szPlatform,
		_countof(szPlatform),
		get_CalcPlatform(FractalType::Mand)),
		get_MaxIterationsMand());

	ExtTextOut(hdc, m_rcInfoPanel.left, y, ETO_CLIPPED, &m_rcInfoPanel, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// mandelbrot image bounding box
	iLen = sprintf_s(szBuf, _countof(szBuf), "Box (%02.2f, %02.2f)-(%02.2f, %02.2f)", m_a1, m_b1, m_a2, m_b2);
	ExtTextOut(hdc, m_rcInfoPanel.left + m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &m_rcInfoPanel, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// mouse pointer location
	iLen = sprintf_s(szBuf, _countof(szBuf), "Pointer (%02.4f, %02.4f)", m_ja, m_jb);
	ExtTextOut(hdc, m_rcInfoPanel.left + m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &m_rcInfoPanel, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// last recalc duration
	iLen = sprintf_s(szBuf, _countof(szBuf), "Clock Time %lld ms", 1000 * m_tProcessDurationTotal[FractalType::Mand].QuadPart / m_ticksPerSecond.QuadPart);
	ExtTextOut(hdc, m_rcInfoPanel.left + m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &m_rcInfoPanel, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// last recalc thread working duration
	iLen = sprintf_s(szBuf, _countof(szBuf), "Thread Time %lld ms", 1000 * m_tThreadDurationTotal[FractalType::Mand].QuadPart / m_ticksPerSecond.QuadPart);
	ExtTextOut(hdc, m_rcInfoPanel.left + m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &m_rcInfoPanel, szBuf, iLen, NULL);
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
		get_CalcPlatform(FractalType::Julia)),
		get_MaxIterationsJulia());

	ExtTextOut(hdc, m_rcInfoPanel.left, y, ETO_CLIPPED, &m_rcInfoPanel, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// Julia set image bounding box
	iLen = sprintf_s(szBuf, _countof(szBuf), "Box (%02.2f, %02.2f)-(%02.2f, %02.2f)", m_jc1, m_jd1, m_jc2, m_jd2);
	ExtTextOut(hdc, m_rcInfoPanel.left + m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &m_rcInfoPanel, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// last recalc duration
	iLen = sprintf_s(szBuf, _countof(szBuf), "Clock Time %lld ms", 1000 * m_tProcessDurationTotal[FractalType::Julia].QuadPart / m_ticksPerSecond.QuadPart);
	ExtTextOut(hdc, m_rcInfoPanel.left + m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &m_rcInfoPanel, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;

	// last recalc thread working duration
	iLen = sprintf_s(szBuf, _countof(szBuf), "Thread Time %lld ms", 1000 * m_tThreadDurationTotal[FractalType::Julia].QuadPart / m_ticksPerSecond.QuadPart);
	ExtTextOut(hdc, m_rcInfoPanel.left + m_tmInfo.tmAveCharWidth, y, ETO_CLIPPED, &m_rcInfoPanel, szBuf, iLen, NULL);
	y += m_tmInfo.tmHeight;



	SelectObject(hdc, hOldFont);
	return true;
}

TCalcInfo *CJuliasmApp::get_CalcInfo(FractalType ft, FractalFormula ff, CalcPlatform cp)
{
	TCalcInfo *pCI = NULL;
	for (int i = 0; i < sizeof(m_ci) / sizeof(m_ci[0]); ++i)
	{
		if (m_ci[i].iFracType == ft && m_ci[i].iFracFormula == ff && m_ci[i].iCalcPlatform == cp)
		{
			pCI = &m_ci[i];
			break;
		}
	}
	assert(pCI != NULL);
	return pCI;
}

//
// handle main window startup 
//
bool CJuliasmApp::handle_create(HWND hWnd, LPCREATESTRUCT *lpcs)
{
	m_hWnd = hWnd;

	// 
	// create the font for information display
	//
	m_ncm.cbSize = sizeof(m_ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(m_ncm), &m_ncm, 0);
	m_hfInfo = CreateFontIndirect(&m_ncm.lfMessageFont);
	HDC hdc = GetDC(hWnd);
	GetTextMetrics(hdc, &m_tmInfo);
	ReleaseDC(hWnd, hdc);

	// setup the application defaults
	//	put_CalcPlatform(FractalType::Mand, FractalFormula::FormMand, CalcPlatform::AVX32);
	put_MaxIterationsMand(1024);
	//	put_CalcPlatform(FractalType::Julia, FractalFormula::FormMand, CalcPlatform::AVX32);
	put_MaxIterationsJulia(1024);

	//
	// OpenCL preparation
	//
	m_OCLFrac.put_Palette(256, m_PaletteDefault.get_RedChannel(), m_PaletteDefault.get_GreenChannel(), m_PaletteDefault.get_BlueChannel());
	m_OCLFrac.UsePlatformDefault();
	m_OCLFrac.UseDeviceByType(m_pCalcInfo[FractalType::Mand]->iOCLKernelNumber, CL_DEVICE_TYPE_GPU);
	m_OCLFrac.UseDeviceByType(m_pCalcInfo[FractalType::Julia]->iOCLKernelNumber, CL_DEVICE_TYPE_GPU);
	if (false == m_OCLFrac.LoadProgram("fractals.cl", true))
	{
		MessageBox(hWnd, "Unable to load program 'fractals.cl' for the Mandelbrot GPU program.", "Error", MB_ICONSTOP | MB_OK);
		exit(0);
	}
	if (false == m_OCLFrac.PrepareProgram(get_hWnd(), IDM_OCL_BUILD_COMPLETE))
	{
		MessageBox(get_hWnd(), "Unable to initialize OpenCL. Other calculation platforms can still be used.", "OpenCL Error", MB_ICONINFORMATION | MB_OK);
	}

	UpdateCalcPlatformMenuJulia();
	UpdateCalcPlatformMenuMand();

	return true;
}

//
// Generic mandelbrot calculation function
//	- uses the current CalculationPlatform
// Return values:
//	true: calculation was initiated successfully
//	false: a calculation was not initiated, and there might have been an error
//
bool CJuliasmApp::StartCalc(FractalType ft)
{
	// don't calculate the fractal if it is not being shown on the screen
	if (false == get_ShowFractal(ft))
		return false;

	// don't perform a calculation if there is no current calculation function
	if (NULL == m_pCalcInfo[ft]->pCalculateFunc)
		return false;

	// only recalculate if there is not an ongoing calculation
	if (m_iCalculatingFractal[ft] != 0)
	{
		return false;
	}

	//
	// Setup performance measurement
	//

	// record the proess start time
	QueryPerformanceCounter(&m_tProcessStart[ft]);
	m_tProcessStop[ft].QuadPart = 0;
	m_tProcessDurationTotal[ft].QuadPart = 0;
	m_tThreadDurationTotal[ft].QuadPart = 0;
	SecureZeroMemory(&m_tThreadDuration[ft], sizeof(m_tThreadDuration[ft]));

	//
	// create threads to perform the calculation
	//
	SecureZeroMemory(m_hThread[ft], sizeof(m_hThread[ft]));
	for (int i = 0; i < m_pCalcInfo[ft]->iThreadCount; ++i)
	{
		InterlockedIncrement(&m_iCalculatingFractal[ft]);
		m_ThreadInfo[ft][i].pCalcInfo = m_pCalcInfo[ft];
		m_ThreadInfo[ft][i].iThreadIndex = i;
		m_ThreadInfo[ft][i].pApp = this;
		m_ThreadInfo[ft][i].iThreadCompleteLParam = MAKELPARAM((WORD)ft, (WORD)m_pCalcInfo[ft]->iCalcPlatform);

		if (m_bmpFractal[ft].get_bmpBits() == NULL)
		{
			MessageBox(NULL, "bitmap is NULL", "Error", MB_ICONEXCLAMATION | MB_OK);
		}

		m_hThread[ft][i] = CreateThread(NULL, THREAD_STACK_SIZE, m_pCalcInfo[ft]->pCalculateFunc, (LPVOID)&m_ThreadInfo[ft][i], 0, NULL);
	}
	return true;
}
bool CJuliasmApp::handle_rbuttondown(HWND hWnd, WORD wvKeys, int x, int y)
{
	m_bUpdateJulia = !m_bUpdateJulia;
	return true;
}
bool CJuliasmApp::handle_rbuttonup(HWND hWnd, WORD	wvKeys, int x, int y)
{
	return false;
}

void CJuliasmApp::SetupOCLOptions(bool bOCLAvailable)
{
	//	m_OCLFrac.UseDeviceByType(m_pCalcInfo[FractalType::Julia]->iOCLKernelNumber, CL_DEVICE_TYPE_GPU);
	if (m_OCLFrac.get_BuffersReady())
	{
		if (false == m_OCLFrac.put_ImageSize(IMAGE_MAND, m_bmpFractal[FractalType::Mand].get_Width(), m_bmpFractal[FractalType::Mand].get_Height()))
		{
			MessageBox(get_hWnd(), "Unable to resize Mandelbrot image.", "Error", MB_ICONSTOP | MB_OK);
			exit(0);
		}
		if (false == m_OCLFrac.put_ImageSize(IMAGE_JULIA, m_bmpFractal[FractalType::Julia].get_Width(), m_bmpFractal[FractalType::Julia].get_Height()))
		{
			MessageBox(get_hWnd(), "Unable to resize Julia image.", "Error", MB_ICONSTOP | MB_OK);
			exit(0);
		}
	}

	// enable OpenCL menu options
	static int iOCLMenuOptions[] = {
		IDM_CALCULATE_MAND_OPENCL_CPU,
		IDM_CALCULATE_MAND_OPENCL_GPU,
		IDM_CALCULATE_JULIA_OPENCL_CPU,
		IDM_CALCULATE_JULIA_OPENCL_GPU,
		IDM_CALCULATE_SIN_JULIA_OPENCL_CPU,
		IDM_CALCULATE_SIN_JULIA_OPENCL_GPU,
		IDM_CALCULATE_COS_JULIA_OPENCL_CPU,
		IDM_CALCULATE_COS_JULIA_OPENCL_GPU,
		IDM_CALCULATE_EXP_JULIA_OPENCL_CPU,
		IDM_CALCULATE_EXP_JULIA_OPENCL_GPU,
	};

	HWND hWnd = get_hWnd();
	if (hWnd != NULL)
	{
		HMENU hMenu = GetMenu(hWnd);
		if (hMenu != NULL)
		{
			for (int i = 0; i < _countof(iOCLMenuOptions); ++i)
			{
				EnableMenuItem(hMenu, iOCLMenuOptions[i], MF_ENABLED | MF_BYCOMMAND);
			}
		}
	}


}

//
// handle command messages 
//
bool CJuliasmApp::handle_command(HWND hWnd, int wmID, int wmEvent, LPARAM lParam)
{
	int i;
	FractalType ft;

	switch (wmID)
	{
	case IDM_OCL_BUILD_COMPLETE:
		SetupOCLOptions(wmID != 0);
		return true;

	case IDM_CALCULATE_MAND_X87:
		put_CalcPlatform(FractalType::Mand, FractalFormula::FormMand, CalcPlatform::x87);
		PostCalc(FractalType::Mand);
		break;

	case IDM_CALCULATE_MAND_SSE:
		put_CalcPlatform(FractalType::Mand, FractalFormula::FormMand, CalcPlatform::SSE);
		PostCalc(FractalType::Mand);
		break;

	case IDM_CALCULATE_MAND_SSE2:
		put_CalcPlatform(FractalType::Mand, FractalFormula::FormMand, CalcPlatform::SSE2);
		PostCalc(FractalType::Mand);
		break;

	case IDM_CALCULATE_MAND_AVX32:
		put_CalcPlatform(FractalType::Mand, FractalFormula::FormMand, CalcPlatform::AVX32);
		PostCalc(FractalType::Mand);
		break;

	case IDM_CALCULATE_MAND_AVX64:
		put_CalcPlatform(FractalType::Mand, FractalFormula::FormMand, CalcPlatform::AVX64);
		PostCalc(FractalType::Mand);
		break;

	case IDM_CALCULATE_MAND_OPENCL_CPU:
		put_CalcPlatform(FractalType::Mand, FractalFormula::FormMand, CalcPlatform::OpenCL_CPU);
		PostCalc(FractalType::Mand);
		break;

	case IDM_CALCULATE_MAND_OPENCL_GPU:
		put_CalcPlatform(FractalType::Mand, FractalFormula::FormMand, CalcPlatform::OpenCL_GPU);
		PostCalc(FractalType::Mand);
		break;

	case IDM_CLEAR_MAND:
		m_bmpFractal[FractalType::Julia].Erase(RGB(255, 255, 255));
		m_bmpFractal[FractalType::Mand].Erase(RGB(255, 255, 255));
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case IDM_RESET_MAND:
		if (m_iCalculatingFractal[FractalType::Mand] == 0)
		{
			InitializeMand();
			PostCalc(FractalType::Mand);
		}
		break;

	case IDM_RESET_JULIA:
		if (m_iCalculatingFractal[FractalType::Julia] == 0)
		{
			InitializeJulia();
			PostCalc(FractalType::Julia);
		}
		break;


	case IDM_RECALCULATE_FRACTAL:
		StartCalc((FractalType)LOWORD(lParam));
		break;

	case IDM_CALCULATE_JULIA_X87:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormMand, CalcPlatform::x87);
		PostCalc(FractalType::Julia);
		return 0;

	case IDM_CALCULATE_JULIA_SSE:	// fall through until SSE Julia is implemented
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormMand, CalcPlatform::SSE);
		PostCalc(FractalType::Julia);
		return 0;

	case IDM_CALCULATE_JULIA_SSE2:	// fall through until SSE2 Julia implemented
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormMand, CalcPlatform::SSE2);
		PostCalc(FractalType::Julia);
		return 0;

	case IDM_CALCULATE_JULIA_AVX32:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormMand, CalcPlatform::AVX32);
		PostCalc(FractalType::Julia);
		return 0;

	case IDM_CALCULATE_JULIA_AVX64:	// fall through
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormMand, CalcPlatform::AVX64);
		PostCalc(FractalType::Julia);
		return 0;

	case IDM_CALCULATE_JULIA_OPENCL_CPU:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormMand, CalcPlatform::OpenCL_CPU);
		PostCalc(FractalType::Julia);
		break;

	case IDM_CALCULATE_JULIA_OPENCL_GPU:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormMand, CalcPlatform::OpenCL_GPU);
		PostCalc(FractalType::Julia);
		break;

	case IDM_CALCULATE_SIN_JULIA_OPENCL_CPU:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormSin, CalcPlatform::OpenCL_CPU);
		PostCalc(FractalType::Julia);
		break;

	case IDM_CALCULATE_SIN_JULIA_OPENCL_GPU:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormSin, CalcPlatform::OpenCL_GPU);
		PostCalc(FractalType::Julia);
		break;

	case IDM_CALCULATE_COS_JULIA_OPENCL_CPU:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormCos, CalcPlatform::OpenCL_CPU);
		PostCalc(FractalType::Julia);
		break;

	case IDM_CALCULATE_COS_JULIA_OPENCL_GPU:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormCos, CalcPlatform::OpenCL_GPU);
		PostCalc(FractalType::Julia);
		break;

	case IDM_CALCULATE_EXP_JULIA_OPENCL_CPU:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormExp, CalcPlatform::OpenCL_CPU);
		PostCalc(FractalType::Julia);
		break;

	case IDM_CALCULATE_EXP_JULIA_OPENCL_GPU:
		put_CalcPlatform(FractalType::Julia, FractalFormula::FormExp, CalcPlatform::OpenCL_GPU);
		PostCalc(FractalType::Julia);
		break;

	case IDM_THREADCOMPLETE:
		ft = (FractalType)LOWORD(lParam);
		if (0 == InterlockedDecrement(&m_iCalculatingFractal[ft]))
		{
			m_tProcessDurationTotal[ft].QuadPart = 0;

			// kill the threads and update the total thread working duration
			for (i = 0; i < m_pCalcInfo[ft]->iThreadCount; ++i)
			{
				CloseHandle(m_hThread[ft][i]);
				m_tThreadDurationTotal[ft].QuadPart += m_tThreadDuration[ft][i].QuadPart;
			}
			// update the total process duration
			QueryPerformanceCounter(&m_tProcessStop[ft]);
			m_tProcessDurationTotal[ft].QuadPart = m_tProcessStop[ft].QuadPart - m_tProcessStart[ft].QuadPart;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case IDM_VIEW_MAND_ONLY:
		put_MandScreenPct(100);
		PostCalc(FractalType::Mand);
		break;

	case IDM_VIEW_SMMAND_LGJULIA:
		put_MandScreenPct(25);
		PostCalcAll();
		break;

	case IDM_VIEW_LGMAND_SMJULIA:
		put_MandScreenPct(75);
		PostCalcAll();
		break;

	case IDM_VIEW_JULIA_ONLY:
		put_MandScreenPct(0);
		PostCalcAll();
		break;

	case IDM_HELP_CPU:
		DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_CPUBOX), get_hWnd(), HelpCPU, (LPARAM)this);
		break;

	case IDM_HELP_GPU:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), get_hWnd(), About);
		break;

	case IDM_ABOUT:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUTBOX), get_hWnd(), About);
		break;

	case IDM_EXIT:
		DestroyWindow(get_hWnd());
		break;

	case IDM_ITERATIONS_MAND_64:
		put_MaxIterationsMand(64);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_MAND_128:
		put_MaxIterationsMand(128);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_MAND_256:
		put_MaxIterationsMand(256);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_MAND_512:
		put_MaxIterationsMand(512);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_MAND_1024:
		put_MaxIterationsMand(1024);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_MAND_2048:
		put_MaxIterationsMand(2048);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_MAND_4096:
		put_MaxIterationsMand(4096);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_MAND_8192:
		put_MaxIterationsMand(8192);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_MAND_16384:
		put_MaxIterationsMand(16384);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_MAND_32767:
		put_MaxIterationsMand(32768);
		PostCalc(FractalType::Mand);
		break;

	case IDM_ITERATIONS_JULIA_64:
		put_MaxIterationsJulia(64);
		PostCalc(FractalType::Julia);
		break;

	case IDM_ITERATIONS_JULIA_128:
		put_MaxIterationsJulia(128);
		PostCalc(FractalType::Julia);
		break;

	case IDM_ITERATIONS_JULIA_256:
		put_MaxIterationsJulia(256);
		PostCalc(FractalType::Julia);
		break;

	case IDM_ITERATIONS_JULIA_512:
		put_MaxIterationsJulia(512);
		PostCalc(FractalType::Julia);
		break;

	case IDM_ITERATIONS_JULIA_1024:
		put_MaxIterationsJulia(1024);
		PostCalc(FractalType::Julia);
		break;

	case IDM_ITERATIONS_JULIA_2048:
		put_MaxIterationsJulia(2048);
		PostCalc(FractalType::Julia);
		break;

	case IDM_ITERATIONS_JULIA_4096:
		put_MaxIterationsJulia(4096);
		PostCalc(FractalType::Julia);
		break;

	case IDM_ITERATIONS_JULIA_8192:
		put_MaxIterationsJulia(8192);
		PostCalc(FractalType::Julia);
		break;

	case IDM_ITERATIONS_JULIA_16384:
		put_MaxIterationsJulia(16384);
		PostCalc(FractalType::Julia);
		break;

	case IDM_ITERATIONS_JULIA_32767:
		put_MaxIterationsJulia(32768);
		PostCalc(FractalType::Julia);
		break;


	default:
		return false;
	}
	return true;
}

void CJuliasmApp::PostCalc(FractalType ft)
{
	PostMessage(get_hWnd(), WM_COMMAND, IDM_RECALCULATE_FRACTAL, ft);
}
void CJuliasmApp::PostCalcAll(void)
{
	PostCalc(FractalType::Mand);
	PostCalc(FractalType::Julia);
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

bool CJuliasmApp::put_MandRect(int iImageIndex, int x1, int y1, int x2, int y2, bool bShow)
{
	char szBuf[128];

	m_rcMand.left = x1;
	m_rcMand.top = y1;
	m_rcMand.right = x2;
	m_rcMand.bottom = y2;
	_m_iMandWidth = x2 - x1;
	_m_iMandHeight = y2 - y1;
	put_ShowFractal(FractalType::Mand, bShow && (_m_iMandWidth > 0) && (_m_iMandHeight > 0));

	if (false == m_bmpFractal[FractalType::Mand].Resize(get_hWnd(), _m_iMandWidth, _m_iMandHeight))
	{
		sprintf_s(szBuf, _countof(szBuf), "Unable to create Mandelbrot bitmap. (w=%d, h=%d)", _m_iMandWidth, _m_iMandHeight);
		MessageBox(get_hWnd(), szBuf, "Error", MB_ICONSTOP | MB_OK);
		exit(0);
	}
	//
	// update the OpenCL object if it is ready, because buffers require the existance of an OpenCL context
	//
	if (m_OCLFrac.get_BuffersReady())
	{
		if (false == m_OCLFrac.put_ImageSize(IMAGE_MAND, _m_iMandWidth, _m_iMandHeight))
		{
			MessageBox(get_hWnd(), "Unable to resize Mandelbrot image.", "Error", MB_ICONSTOP | MB_OK);
			exit(0);
		}
	}
	//
	// it's okay to set the bitmap even if the buffers arent ready because they bitmaps don't require the existance of an OpenCL context
	if (false == m_OCLFrac.put_Bitmap(IMAGE_MAND, bShow ? m_bmpFractal[FractalType::Mand].get_bmpBits() : NULL))
	{
		MessageBox(get_hWnd(), "Unable to put bitmap in Mandelbrot image.", "Error", MB_ICONSTOP | MB_OK);
		exit(0);
	}
	return true;
}
bool CJuliasmApp::put_JuliaRect(int iImageIndex, int x1, int y1, int x2, int y2, bool bShow)
{
	char szBuf[128];

	m_rcJulia.left = x1;
	m_rcJulia.top = y1;
	m_rcJulia.right = x2;
	m_rcJulia.bottom = y2;
	_m_iJuliaWidth = x2 - x1;
	_m_iJuliaHeight = y2 - y1;
	put_ShowFractal(FractalType::Julia, bShow && (_m_iJuliaWidth > 0) && (_m_iJuliaHeight > 0));

	if (false == m_bmpFractal[FractalType::Julia].Resize(get_hWnd(), _m_iJuliaWidth, _m_iJuliaHeight))
	{
		sprintf_s(szBuf, _countof(szBuf), "Unable to create Julia bitmap. (w=%d, h=%d)", _m_iJuliaWidth, _m_iJuliaHeight);
		MessageBox(get_hWnd(), "Unable to create Julia set bitmap.", "Error", MB_ICONSTOP | MB_OK);
		exit(0);
	}
	if (m_OCLFrac.get_BuffersReady())
	{
		if (false == m_OCLFrac.put_ImageSize(IMAGE_JULIA, _m_iJuliaWidth, _m_iJuliaHeight))
		{
			MessageBox(get_hWnd(), "Unable to create resize Julia set image.", "Error", MB_ICONSTOP | MB_OK);
			exit(0);
		}
	}
	if (get_ShowFractal(FractalType::Julia))
	{
		if (false == m_OCLFrac.put_Bitmap(IMAGE_JULIA, m_bmpFractal[FractalType::Julia].get_bmpBits()))
		{
			MessageBox(get_hWnd(), "Unable to create put bitmap in Julia set image.", "Error", MB_ICONSTOP | MB_OK);
			exit(0);
		}
	}

	return true;
}
bool CJuliasmApp::put_InfoRect(int x1, int y1, int x2, int y2, bool bShow)
{
	m_rcInfoPanel.left = x1;
	m_rcInfoPanel.top = y1;
	m_rcInfoPanel.right = x2;
	m_rcInfoPanel.bottom = y2;
	_m_iInfoPanelWidth = x2 - x1;
	_m_iInfoPanelHeight = y2 - y1;
	m_bShowInfoPanel = bShow && (_m_iInfoPanelWidth > 0) && (_m_iInfoPanelHeight > 0);
	return true;
}

bool CJuliasmApp::put_MandScreenPct(int iPct)
{
	RECT rcClient;
	GetClientRect(get_hWnd(), &rcClient);

	m_iMandScreenPct = iPct;

	//
	// Option 1: Show only the Mandelbrot Set
	//
	if (iPct == 100)
	{
		rcClient.right = rcClient.right - rcClient.right % POINTS_CONCURRENT_MAX;
		// calculate a new numerical bounding box to keep pixles square inthe event of an aspect ratio change
		double am = (m_a2 + m_a1) / 2.0;
		double bm = (m_b2 + m_b1) / 2.0;
		double da = (m_a2 - m_a1) / rcClient.right;
		double db = (m_b2 - m_a1) / rcClient.bottom;
		double d = max(da, db);
		m_a1 = am - d * rcClient.right / 2.0;
		m_a2 = am + d * rcClient.right / 2.0;
		m_b1 = bm - d * rcClient.bottom / 2.0;
		m_b2 = bm + d * rcClient.bottom / 2.0;
		m_OCLFrac.put_Boundary(IMAGE_MAND, (float)m_a1, (float)m_b1, (float)m_a2, (float)m_b2);

		put_MandRect(IMAGE_MAND, 0, 0, rcClient.right, rcClient.bottom, true);
		put_JuliaRect(IMAGE_JULIA, 0, 0, 0, 0, false);
		put_InfoRect(0, 0, 0, 0, false);
	}
	//
	// Option 2 allow a rectangular Mandelbrot and a square Julia
	else if (iPct >= 50)
	{
		RECT rcMand;

		// align to top-left of screen
		rcMand.left = 0;
		rcMand.top = 0;

		// make sure the width is a multiple of POINTS_CONCURRENT_MAX
		int iWidth = rcClient.right * iPct / 100;
		iWidth = iWidth - iWidth % POINTS_CONCURRENT_MAX;
		rcMand.right = iWidth;

		// make sure the image is at least 1 pixel high
		rcMand.bottom = rcClient.bottom;

		// calculate a new numerical bounding box to keep pixles square inthe event of an aspect ratio change
		double am = (m_a2 + m_a1) / 2.0;
		double bm = (m_b2 + m_b1) / 2.0;
		double da = (m_a2 - m_a1) / rcMand.right;
		double db = (m_b2 - m_a1) / rcMand.bottom;
		double d = max(da, db);
		m_a1 = am - d * rcMand.right / 2.0;
		m_a2 = am + d * rcMand.right / 2.0;
		m_b1 = bm - d * rcMand.bottom / 2.0;
		m_b2 = bm + d * rcMand.bottom / 2.0;
		m_OCLFrac.put_Boundary(IMAGE_MAND, (float)m_a1, (float)m_b1, (float)m_a2, (float)m_b2);

		put_MandRect(IMAGE_MAND, 0, 0, rcMand.right, rcMand.bottom, true);

		RECT rcJulia;
		rcJulia.left = rcMand.right;
		rcJulia.top = 0;
		rcJulia.right = rcClient.right;
		rcJulia.bottom = rcClient.right - rcMand.right;

		put_JuliaRect(IMAGE_JULIA, rcJulia.left, rcJulia.top, rcJulia.right, rcJulia.bottom, true);

		put_InfoRect(rcJulia.left, rcJulia.bottom, rcClient.right, rcClient.bottom, true);
	}

	//
	// Option 2: Show the mandelbrot at less than 100%.  Ensure square pixels.
	else if (iPct > 0 && iPct < 100)
	{
		RECT rcMand;

		// align to top-left of screen
		rcMand.left = 0;
		rcMand.top = 0;

		// make sure the width is a multiple of POINTS_CONCURRENT_MAX
		int iWidth = min(rcClient.right * iPct / 100, rcClient.bottom);
		iWidth = iWidth - iWidth % POINTS_CONCURRENT_MAX;
		rcMand.right = iWidth;

		// make sure the image is at least 1 pixel high
		rcMand.bottom = max(1, rcMand.right);

		// calculate a new numerical bounding box to keep pixles square inthe event of an aspect ratio change
		double am = (m_a2 + m_a1) / 2.0;
		double bm = (m_b2 + m_b1) / 2.0;
		double da = (m_a2 - m_a1) / rcMand.right;
		double db = (m_b2 - m_a1) / rcMand.bottom;
		double d = max(da, db);
		m_a1 = am - d * rcMand.right / 2.0;
		m_a2 = am + d * rcMand.right / 2.0;
		m_b1 = bm - d * rcMand.bottom / 2.0;
		m_b2 = bm + d * rcMand.bottom / 2.0;
		m_OCLFrac.put_Boundary(IMAGE_MAND, (float)m_a1, (float)m_b1, (float)m_a2, (float)m_b2);


		put_MandRect(IMAGE_MAND, 0, 0, rcMand.right, rcMand.bottom, true);
		put_JuliaRect(IMAGE_JULIA, rcMand.right, 0, rcClient.right, rcClient.bottom, true);
		put_InfoRect(0, rcMand.bottom, rcMand.right, rcClient.bottom, true);
	}
	// Option 3: Show only the julia set
	else if (iPct == 0)
	{
		put_MandRect(IMAGE_MAND, 0, 0, 0, 0, false);
		put_JuliaRect(IMAGE_JULIA, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, true);
		put_InfoRect(0, 0, 0, 0, false);
	}
	return true;
}


bool CJuliasmApp::handle_size(HWND hWnd, HDC hdc, int iSizeType, int iWidth, int iHeight)
{
	// only respond to messages if the right type of resize messages are sent
	if ((iSizeType == SIZE_MAXIMIZED) || (iSizeType == SIZE_MAXSHOW) || (iSizeType == SIZE_RESTORED))
	{
		int i = 0;
		while ((m_iCalculatingFractal[FractalType::Mand] || m_iCalculatingFractal[FractalType::Julia]) && (i < 20))
		{
			Sleep(1);
			++i;
		}
		if (m_iCalculatingFractal[FractalType::Mand] || m_iCalculatingFractal[FractalType::Julia])
			return false;

		put_MandScreenPct(m_iMandScreenPct);

		// redraw the images
		PostCalc(FractalType::Mand);
		return true;
	}
	return false;
}

//
// handlemounse moves
//

bool CJuliasmApp::handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y)
{
	// don't respond to mouse moves if the image is currently recalculating
	if (m_iCalculatingFractal[FractalType::Julia] || m_iCalculatingFractal[FractalType::Mand])
	{
		return false;
	}

	//
	// handle a mouse capture
	//
	if (wParam & MK_LBUTTON)
	{
		//
		// if the mouse is being click-dragged over a fractal, 
		//	then initiate a drag operation
		//
		if (m_bDragging == false)
		{
			// determine which fractal is being dragged
			POINT p;
			p.x = x;
			p.y = y;
			if (PtInRect(&m_rcMand, p))
			{
				m_DragFractal = FractalType::Mand;
				m_bDragging = true;
			}
			else if (PtInRect(&m_rcJulia, p))
			{
				m_DragFractal = FractalType::Julia;
				m_bDragging = true;
			}

			//
			// if the click-drag was over a fractal, then save the beginning state
			//	for the mouse click, as well as for the Mandelbrot and Julia fractals
			//
			if (true == m_bDragging)
			{
				SetCapture(hWnd);		// keep the mouse

				m_iDragStartX = x;		// keep the initial mouse-click location
				m_iDragStartY = y;

				m_start_a1 = m_a1;		// keep the Mandelbrot set numerical boundaries
				m_start_b1 = m_b1;
				m_start_a2 = m_a2;
				m_start_b2 = m_b2;

				m_start_jc1 = m_jc1;	// keep the Julia ser numerical boundaries
				m_start_jd1 = m_jd1;
				m_start_jc2 = m_jc2;
				m_start_jd2 = m_jd2;
			}
		}

		// save the current drag location
		m_iDragCurrentX = x;
		m_iDragCurrentY = y;
		m_iDragEndX = x;
		m_iDragEndY = y;

		//
		// redraw the selected fractal based on the new drag location
		//

		// determine the number of pixels moved
		int dx = (m_iDragCurrentX - m_iDragStartX);
		int dy = (m_iDragCurrentY - m_iDragStartY);

		// convert that into a numeric offset
		if (m_DragFractal == FractalType::Mand)
		{
			double offset_a = (m_start_a2 - m_start_a1) / get_MandWidth() * dx;
			double offset_b = (m_start_b2 - m_start_b1) / get_MandHeight() * dy;

			// update the drawing variables
			m_a1 = m_start_a1 - offset_a;
			m_a2 = m_start_a2 - offset_a;
			m_b1 = m_start_b1 - offset_b;
			m_b2 = m_start_b2 - offset_b;

			m_OCLFrac.put_Boundary(m_pCalcInfo[FractalType::Mand]->iOCLKernelNumber, (float)m_a1, (float)m_b1, (float)m_a2, (float)m_b2);

			StartCalc(FractalType::Mand);
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
			return true;
		}
		else if (m_DragFractal == FractalType::Julia)
		{
			double offset_c = (m_start_jc2 - m_start_jc1) / get_JuliaWidth() * dx;
			double offset_d = (m_start_jd2 - m_start_jd1) / get_JuliaHeight() * dy;

			// update the drawing variables
			m_jc1 = m_start_jc1 - offset_c;
			m_jc2 = m_start_jc2 - offset_c;
			m_jd1 = m_start_jd1 - offset_d;
			m_jd2 = m_start_jd2 - offset_d;


			m_OCLFrac.put_Boundary(m_pCalcInfo[FractalType::Julia]->iOCLKernelNumber, (float)m_jc1, (float)m_jd1, (float)m_jc2, (float)m_jd2);

			StartCalc(FractalType::Julia);
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
			return true;
		}
	}

	// update the Julia set location with the numerical (not pixel) location of the mouse pointer
	put_JuliaPoint(
		m_pCalcInfo[FractalType::Julia]->iOCLKernelNumber,
		(m_a1 + (m_a2 - m_a1) / get_MandWidth() * x),
		(m_b1 + (m_b2 - m_b1) / get_MandHeight() * y)
		);
	PostCalc(FractalType::Julia);

	return true;
}

void CJuliasmApp::ZoomInMand(int x, int y, float fAmount)
{
	// default x and y, if needed
	if (x < 0)
	{
		x = get_MandWidth() / 2;
	}
	if (y < 0)
	{
		y = get_MandHeight() / 2;
	}

	double height, width, da, db, anew, bnew;

	// only proceed if there are no mandelbrot calculations in the works
	if (m_iCalculatingFractal[FractalType::Mand] == 0)
	{
		da = (m_a2 - m_a1) / get_MandWidth();
		db = (m_b2 - m_b1) / get_MandHeight();
		width = m_a2 - m_a1;
		height = m_b2 - m_b1;
		anew = m_a1 + da * x;
		bnew = m_b1 + db * y;

		width = width * fAmount;
		height = height * fAmount;
		m_a1 = anew - (width / 2.0);
		m_a2 = m_a1 + width;

		m_b1 = bnew - (height / 2.0);
		m_b2 = m_b1 + height;

		// share the new bounding box with the OpenCL object
		m_OCLFrac.put_Boundary(m_pCalcInfo[FractalType::Mand]->iOCLKernelNumber, (float)m_a1, (float)m_b1, (float)m_a2, (float)m_b2);

		// recalculate the image
		PostCalc(FractalType::Mand);
	}
}
void CJuliasmApp::ZoomInJulia(int x, int y, float fAmount)
{
	// default x and y, if needed
	if (x < 0)
	{
		x = get_JuliaWidth() / 2;
	}
	if (y < 0)
	{
		y = get_JuliaHeight() / 2;
	}

	double height, width, dc, dd, anew, bnew;

	// only proceed if there are no mandelbrot calculations in the works
	if (m_iCalculatingFractal[FractalType::Julia] == 0)
	{
		dc = (m_jc2 - m_jc1) / get_JuliaWidth();
		dd = (m_jd2 - m_jd1) / get_JuliaHeight();
		width = m_jc2 - m_jc1;
		height = m_jd2 - m_jd1;
		anew = m_jc1 + dc * x;
		bnew = m_jd1 + dd * y;

		width = width * fAmount;
		height = height * fAmount;
		m_jc1 = anew - (width / 2.0);
		m_jc2 = m_jc1 + width;

		m_jd1 = bnew - (height / 2.0);
		m_jd2 = m_jd1 + height;

		// share the new bounding box with the OpenCL object
		m_OCLFrac.put_Boundary(m_pCalcInfo[FractalType::Julia]->iOCLKernelNumber, (float)m_jc1, (float)m_jd1, (float)m_jc2, (float)m_jd2);

		// recalculate the image
		PostCalc(FractalType::Julia);
	}
}

//
// handle mouse wheel events
// The mouse wheel is used to zoom into the mandelbrot set image.
// RAP: consider using the mouse wheel to also zoom into the Julia set image
//
bool CJuliasmApp::handle_mousewheel(HWND hWnd, WORD wvKeys, int iRotationAmount, int x, int y)
{
	POINT p;
	float fZoomPct = 0.20f;
	
	p.x = x;
	p.y = y;
	ScreenToClient(hWnd, &p);

	iRotationAmount = iRotationAmount / WHEEL_DELTA;

	if (wvKeys & MK_CONTROL)
	{
		fZoomPct *= 0.10f;
	}
	if (wvKeys & MK_SHIFT)
	{
		fZoomPct *= 2.0f;
	}

	if (PtInRect(&m_rcMand, p))
	{
		float fAmt = (iRotationAmount < 0) ? (1.0f + fZoomPct) : (1.0f - fZoomPct);
		ZoomInMand(-1, -1, fAmt);
		return true;
	}
	else if (PtInRect(&m_rcJulia, p))
	{
		float fAmt = (iRotationAmount < 0) ? (1.0f + fZoomPct) : (1.0f - fZoomPct);
		ZoomInJulia(-1, -1, fAmt);
		return true;
	}
	return true;
}

bool CJuliasmApp::handle_lbuttondown(HWND hWnd, WORD wvKeys, int x, int y)
{
	m_iDragStartX = x;
	m_iDragStartY = y;
	return true;
}
bool CJuliasmApp::handle_lbuttonup(HWND hWnd, WORD	wvKeys, int x, int y)
{
	if (m_bDragging)
	{
		m_iDragEndX = x;
		m_iDragEndY = y;
		m_bDragging = false;
		ReleaseCapture();
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
	}
	return true;
}
//
// Update the platofmr menu to indicate which calculation platform is being used
// for the calculation of Mandelbrot sets
//
void CJuliasmApp::UpdateCalcPlatformMenuMand(void)
{
	// convert the current platform into a menu item
	int iMenuItem = -1;
	switch (get_CalcPlatform(FractalType::Mand))
	{
	case x87:	iMenuItem = IDM_CALCULATE_MAND_X87; break;
	case SSE:	iMenuItem = IDM_CALCULATE_MAND_SSE; break;
	case SSE2:	iMenuItem = IDM_CALCULATE_MAND_SSE2; break;
	case AVX32:	iMenuItem = IDM_CALCULATE_MAND_AVX32; break;
	case AVX64:	iMenuItem = IDM_CALCULATE_MAND_AVX64; break;
	case FMA:	iMenuItem = IDM_CALCULATE_MAND_FMA; break;
	case OpenCL_CPU:	iMenuItem = IDM_CALCULATE_MAND_OPENCL_CPU; break;
	case OpenCL_GPU:	iMenuItem = IDM_CALCULATE_MAND_OPENCL_GPU; break;
	default:	iMenuItem = 0; break;
	}

	// uncheck all of the platform menu items
	HWND hWnd = get_hWnd();
	if (hWnd != NULL)
	{
		assert(hWnd != NULL);
		HMENU hMenu = GetMenu(hWnd);
		if (hMenu != NULL)
		{
			assert(hMenu != NULL);
			CheckMenuRadioItem(GetMenu(get_hWnd()), IDM_CALCULATE_MAND_MIN, IDM_CALCULATE_MAND_MAX, iMenuItem, MF_BYCOMMAND);
		}
	}

}
//
// Update the platform menu to indicate which calculation platform is being used
// for the calculation of Julia sets
void CJuliasmApp::UpdateCalcPlatformMenuJulia(void)
{
	// convert the current platform into a menu item
	int iMenuItem = -1;
	switch (get_CalcPlatform(FractalType::Julia))
	{
	case x87:	iMenuItem = IDM_CALCULATE_JULIA_X87; break;
	case SSE:	iMenuItem = IDM_CALCULATE_JULIA_SSE; break;
	case SSE2:	iMenuItem = IDM_CALCULATE_JULIA_SSE2; break;
	case AVX32:	iMenuItem = IDM_CALCULATE_JULIA_AVX32; break;
	case AVX64:	iMenuItem = IDM_CALCULATE_JULIA_AVX64; break;
	case FMA:	iMenuItem = IDM_CALCULATE_JULIA_FMA; break;
	case OpenCL_CPU:
		switch (m_pCalcInfo[FractalType::Julia]->iFracFormula)
		{
		case FormMand: iMenuItem = IDM_CALCULATE_JULIA_OPENCL_CPU; break;
		case FormSin: iMenuItem = IDM_CALCULATE_SIN_JULIA_OPENCL_CPU; break;
		case FormCos: iMenuItem = IDM_CALCULATE_COS_JULIA_OPENCL_CPU; break;
		case FormExp: iMenuItem = IDM_CALCULATE_EXP_JULIA_OPENCL_CPU; break;
		}
		break;

	case OpenCL_GPU:
		switch (m_pCalcInfo[FractalType::Julia]->iFracFormula)
		{
		case FormMand: iMenuItem = IDM_CALCULATE_JULIA_OPENCL_GPU; break;
		case FormSin: iMenuItem = IDM_CALCULATE_SIN_JULIA_OPENCL_GPU; break;
		case FormCos: iMenuItem = IDM_CALCULATE_COS_JULIA_OPENCL_GPU; break;
		case FormExp: iMenuItem = IDM_CALCULATE_EXP_JULIA_OPENCL_GPU; break;
		}
		break;

	default:	iMenuItem = 0; break;
	}

	// uncheck all of the platform menu items
	int iError;
	HMENU hMenu = GetMenu(get_hWnd());
	if (hMenu != NULL)
	{
		if (0 == CheckMenuRadioItem(hMenu, IDM_CALCULATE_JULIA_MIN, IDM_CALCULATE_JULIA_MAX, iMenuItem, MF_BYCOMMAND))
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
	m_OCLFrac.put_MaxIterations(m_iMaxIterationsMand, m_pCalcInfo[FractalType::Mand]->iOCLKernelNumber);

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
	m_OCLFrac.put_MaxIterations(m_iMaxIterationsJulia, m_pCalcInfo[FractalType::Julia]->iOCLKernelNumber);

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
	case FMA: ptr = "FMA"; break;
	case OpenCL_CPU: ptr = "OpenCL CPU"; break;
	case OpenCL_GPU: ptr = "OpenCL GPU"; break;
	default: ptr = "Undefined"; break;
	}
	strcpy_s(szBuf, iLen, ptr);
	return szBuf;
}

bool CJuliasmApp::put_OCLDeviceType(COpenCLImage *ocl, cl_device_type oclDeviceType, int iKernelIndex, int iImageIndex, int iImageWidth, int iImageHeight)
{
	//
	// reset the OpenCL environment
	//
	if (ocl->get_CurrentDeviceTypeByKernel(iKernelIndex) != oclDeviceType)
	{
		HCURSOR hCursor = LoadCursor(NULL, IDC_WAIT);
		HCURSOR hOldCursor = SetCursor(hCursor);
		ocl->UseDeviceByType(iKernelIndex, oclDeviceType);

		if (false == ocl->put_ImageSize(iImageIndex, iImageWidth, iImageHeight))
		{
			MessageBox(m_hWnd, "Unable to resize OpenCL image.", "Error", MB_ICONSTOP | MB_OK);
			exit(0);	//RAP: using exit for now, as this is currently an unrecoverable error
			return false;
		}
		hCursor = SetCursor(hOldCursor);
	}
	return true;
}

bool CJuliasmApp::put_CalcPlatform(FractalType ft, FractalFormula ff, CalcPlatform cp)
{
	// find the specified combination of FtactalType and CalcPlatform
	m_pCalcInfo[ft] = NULL;
	TCalcInfo *found_ci = get_CalcInfo(ft, ff, cp);

	// bail out if the requested combination of Fractal Type and Calc Platform was not found
	if (found_ci == NULL)
		return false;

	m_pCalcInfo[ft] = found_ci;

	// update the OpenCL objects with the desired device type
	if (cp == CalcPlatform::OpenCL_CPU || cp == CalcPlatform::OpenCL_GPU)
	{
		if (ft == FractalType::Julia)
		{
			int k = m_pCalcInfo[FractalType::Julia]->iOCLKernelNumber;
			put_OCLDeviceType(&m_OCLFrac, found_ci->iOCLDeviceType, k, IMAGE_JULIA, get_JuliaWidth(), get_JuliaHeight());
			m_OCLFrac.put_MaxIterations(m_iMaxIterationsJulia, k);
			m_OCLFrac.put_Boundary(k, m_jc1, m_jd1, m_jc2, m_jd2);
			m_OCLFrac.put_ConstPoint(k, m_ja, m_jb);
		}
		else
		{
			int k = m_pCalcInfo[FractalType::Mand]->iOCLKernelNumber;
			put_OCLDeviceType(&m_OCLFrac, found_ci->iOCLDeviceType, found_ci->iOCLKernelNumber, IMAGE_MAND, get_MandWidth(), get_MandHeight());
			m_OCLFrac.put_MaxIterations(m_iMaxIterationsMand, k);
			m_OCLFrac.put_Boundary(k, m_a1, m_b1, m_a2, m_b2);
		}
	}

	UpdateCalcPlatformMenuJulia();
	UpdateCalcPlatformMenuMand();


	return true;
}
