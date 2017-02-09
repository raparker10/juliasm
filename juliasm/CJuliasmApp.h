
//
// CJuliasmApp
//
// Applicaiton class for the Juliasm application
//

class CJuliaasmApp;

struct TThreadInfo {
	int iThreadIndex;
	class CJuliasmApp *pApp;
};

enum CalcPlatform {
	None,
	x87,
	SSE,
	SSE2,
	AVX32,
	AVX64,
	FMA,
	OpenCL_CPU,
	OpenCL_GPU,
	NUMBER_CALC_PLATFORMS,
};

#define THREAD_STACK_SIZE 20480

class CJuliasmApp : public CApplication {
	COpenCLMand m_OCLMand;
	COpenCLJulia m_OCLJulia;

	TThreadInfo m_ThreadInfoMand[MAX_MAND_THREADS];
	TThreadInfo m_ThreadInfoJulia[MAX_JULIA_THREADS];
	TThreadInfo m_ThreadInfoJuliaX87[MAX_JULIA_THREADS];
	TThreadInfo m_ThreadInfoJuliaAVX32[MAX_JULIA_THREADS];
	TThreadInfo m_ThreadInfoJuliaAVX64[MAX_JULIA_THREADS];

	HANDLE m_hThreadJulia[MAX_JULIA_THREADS];
	DWORD  m_dwThreadJuliaID[MAX_JULIA_THREADS];
	DWORD  m_dwThreadJuliaIDX87[MAX_JULIA_THREADS];
	DWORD  m_dwThreadJuliaIDAVX32[MAX_JULIA_THREADS];
	DWORD  m_dwThreadJuliaIDAVX64[MAX_JULIA_THREADS];
	CalcPlatform m_CalcPlatformMand;
	CalcPlatform m_CalcPlatformJulia;

	//
	// Font(s) for displaying text
	//
	NONCLIENTMETRICS m_ncm;
	LOGFONT m_lfInfo;
	HFONT m_hfInfo;
	TEXTMETRIC m_tmInfo;

	//
	// bitmaps to hold the mandelbrot and julia images
	//
	 CBitmap256Color m_bmpMandelbrot;
	 CBitmap256Color m_bmpJulia;
	 int m_iMandWidth;
	 int m_iMandHeight;
	 int m_iJuliaWidth;
	 int m_iJuliaHeight;
	 int m_iJuliaLeft;

	// *******************************
	// Time / performance measurement
	//
	 LARGE_INTEGER m_tTotal;
	 LARGE_INTEGER m_ticksPerSecond;
//	 LARGE_INTEGER m_iTicks[MAX_JULIA_THREADS];
//	 LARGE_INTEGER m_iTotalTicks;

	 // since there are (potentially) multiple threads, it is important to track both the 
	 // clock time (the amount of time that has passed for the operator), and the 
	 // total thread working duration.  The ratio of these two durations is how effective
	 // multi-threading is.
 	 LARGE_INTEGER 
			// worker thread duration tracking
			m_tMandelbrotThreadDuration[MAX_MAND_THREADS],		// duration of each thread
			m_tMandelbrotThreadDurationTotal,					// total duration of all threads

			// duration experienced by the operator - clock time
			m_tMandelbrotProcessStart,									// start time of all worker threads
			m_tMandelbrotProcessStop,									// stop time of all worker threads
			m_tMandelbrotProcessDurationTotal;							// total clock duration


 	 LARGE_INTEGER 
			// worker thread duration tracking
			m_tJuliaThreadDuration[MAX_MAND_THREADS],		// duration of each thread
			m_tJuliaThreadDurationTotal,					// total duration of all threads

			// duration experienced by the operator - clock time
			m_tJuliaProcessStart,									// start time of all worker threads
			m_tJuliaProcessStop,									// stop time of all worker threads
			m_tJuliaProcessDurationTotal;							// total clock duration

	//
	// Julia set parameters
	//
	 float m_ja_sse, m_jb_sse;
	 float m_jc1_sse, m_jc2_sse;
	 float m_jd1_sse, m_jd2_sse;

	 int m_iMaxIterationsJulia;
	 int m_iJMaxThread;

	//
	// Color Palette
	//
	 CPalette m_PaletteDefault;

	//
	// Thread management
	//
	 LONG m_iMandelbrotThreadCount;
	 LONG m_iJuliaThreadCount;
	 LONG m_iJuliaReady[MAX_JULIA_THREADS];

	//
	// SSE variables
	//

	// bounding box
	 double m_a1, m_a2, m_b1, m_b2;

	// per-pixel offsets in the horizontal (da) and vertical (db) directions
//	 double m_da, m_db;
	 int m_iMaxIterationsMand;

	 volatile LONG m_iCalculatingJulia;
	 volatile LONG m_iCalculatingMandelbrot;

	 char *m_szMethod;
	 HANDLE m_hThreadMandelbrot[MAX_MAND_THREADS];

	 //
	 // CPU feature identification
	 //
	CCPU cpu;


	void Initialize(void);
	void InitializeMand(void);

	//
	// mouse dragging
	//
	bool m_bDragging;
	int m_iDragStartX, m_iDragStartY,
		m_iDragCurrentX, m_iDragCurrentY,
		m_iDragEndX, m_iDragEndY;
	double m_start_a1, 
		m_start_b1, 
		m_start_a2, 
		m_start_b2;


public:
	CJuliasmApp();
	~CJuliasmApp();

	virtual bool handle_paint(HWND hWnd, HDC hdc, LPPAINTSTRUCT ps);
	virtual bool handle_create(HWND hWnd, LPCREATESTRUCT *lpcs);
	virtual bool handle_command(HWND hWnd, int wmID, int wmEvent);
	virtual bool handle_keydown(HWND hWnd, int iVKey);
	virtual bool handle_char(HWND hWnd, int iChar);
	virtual bool handle_vscroll(HWND hWnd, int iScrollRequest, int iScrollPosition);
	virtual bool handle_size(HWND hWnd, HDC hdc, int iSizeType, int iWidth, int iHeight);
	virtual bool handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y);
	virtual bool handle_lbuttondoubleclick(HWND hWnd, WPARAM wvKeyDown, WORD x, WORD y);
	virtual bool handle_mousewheel(HWND hWnd, WORD wvKeys, int iRotationAmount, int x, int y);
	virtual bool handle_lbuttondown(HWND hWnd, WORD wvKeys, int x, int y);
	virtual bool handle_lbuttonup(HWND hWnd, WORD	wvKeys, int x, int y);	


	void ZoomInMand(int x=-1, int y = -1);
	void ZoomOutMand(void);

	void PostRecalcMand(void);
	void PostRecalcJulia(void);
	void PostRecalcAll(void);

	bool RecalculateJulia(void);

	void UpdateCalcPlatformMenuMand();
	void UpdateCalcPlatformMenuJulia();

	char *get_CalcPlatformName(char *szBuf, size_t iLen, CalcPlatform cp);

	inline CalcPlatform get_CalcPlatformMand(void) const {
		return m_CalcPlatformMand;
	}

	void put_CalcPlatformMand(CalcPlatform cp) {
		m_CalcPlatformMand = cp;
		UpdateCalcPlatformMenuMand();
	}
	inline CalcPlatform get_CalcPlatformJulia(void) const {
		return m_CalcPlatformJulia;
	}
	void put_CalcPlatformJulia(CalcPlatform cp) {
		m_CalcPlatformJulia = cp;
		UpdateCalcPlatformMenuJulia();
	}

	void put_MaxIterationsMand(int iMaxIterationsMand);
	inline int get_MaxIterationsMand(void) const { return this->m_iMaxIterationsMand; }

	void put_MaxIterationsJulia(int iMaxIterationsJulia);
	inline int get_MaxIterationsJulia(void) const { return this->m_iMaxIterationsJulia; }

	static DWORD WINAPI CalculateJuliaX87(void* pArguments);
	static DWORD WINAPI CalculateJuliaSSE(void* pArguments);
	static DWORD WINAPI CalculateJuliaSSE2(void* pArguments);
	static DWORD WINAPI CalculateJuliaAVX32(void* pArguments);
	static DWORD WINAPI CalculateJuliaAVX64(void* pArguments);

	void CalculateMandelbrot(void);
	void StartMandelbrotx87(HWND hWnd);
	void StartMandelbrotSSE(HWND hWnd);
	void StartMandelbrotSSE2(HWND hWnd);
	void StartMandelbrotAVX32(HWND hWnd);
	void StartMandelbrotAVX64(HWND hWnd);
	void StartMandelbrotAVX2(HWND hWnd);
	void StartMandelbrotOpenCL_CPU(HWND hWnd);
	void StartMandelbrotOpenCL_GPU(HWND hWnd);


	static DWORD WINAPI CJuliasmApp::CalculateMandSSE(void* pArguments);
	void CJuliasmApp::CalculateMandPointsSSE(void);

	static DWORD WINAPI CalculateMandX87(void* pArguments);
	static DWORD WINAPI CalculateMandSSE2(void* pArguments);
	static DWORD WINAPI CalculateMandAVX32(void* pArguments);
	static DWORD WINAPI CalculateMandAVX64(void* pArguments);


};