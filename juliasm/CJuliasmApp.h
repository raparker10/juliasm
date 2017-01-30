
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
	AVX,
	AVX2,
	NUMBER_CALC_PLATFORMS,
};

class CJuliasmApp : public CApplication {

	TThreadInfo m_ThreadInfoMand[MAX_MAND_THREADS];
	TThreadInfo m_ThreadInfoJulia[MAX_JULIA_THREADS];
	HANDLE m_hThreadJulia[MAX_JULIA_THREADS];
	DWORD  m_dwThreadJuliaID[MAX_JULIA_THREADS];
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
	 LARGE_INTEGER m_iTicks[MAX_JULIA_THREADS];
	 LARGE_INTEGER m_iTotalTicks;

	//
	// Julia set parameters
	//
	 float m_ja_sse, m_jb_sse;
	 float m_jc1_sse, m_jc2_sse;
	 float m_jd1_sse, m_jd2_sse;

	 int m_jiMaxIterations;
	 int m_iJMaxIter;
	 int m_iJMaxThread;

	//
	// Color Palette
	//
	 CPalette m_PaletteDefault;

	//
	// Thread management
	//
	 LONG m_iMandelbrotThreads;
	 LONG m_iJuliaThreads;
	 LONG m_iJuliaReady[MAX_JULIA_THREADS];

	//
	// SSE variables
	//

	// orbit saving
	 unsigned short m_bSaveOrbit;
	 unsigned short m_iOrbitIndex;
	 unsigned short m_iOrbitPoints;
	 float m_orbit_c_sse[1024];
	 float m_orbit_d_sse[1024];

	// bounding box
	 double m_a1, m_a2, m_b1, m_b2;

	// per-pixel offsets in the horizontal (da) and vertical (db) directions
	 double m_da, m_db;
	 int m_iMaxIterations;

	 volatile LONG m_iCalculatingJulia;
	 volatile LONG m_iCalculatingMandelbrot;
	 LARGE_INTEGER m_tMandelbrotStart, m_tMandelbrotStop, m_tMandelbrotTotal;
	 char *m_szMethod;
	 HANDLE m_hThreadMandelbrotSSE[MAX_MAND_THREADS];

	void Initialize(void);
	void InitializeMand(void);

public:
	CJuliasmApp();
	~CJuliasmApp();

	static HWND get_hwnd(void) { return m_hWnd; }

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

	void PostRecalcMand(void);
	void PostRecalcJulia(void);
	void PostRecalcAll(void);

	bool RecalculateJulia(void);

	inline CalcPlatform get_CalcPlatformMand(void) const {
		return m_CalcPlatformMand;
	}
	void put_CalcPlatofrmMand(CalcPlatform cp) {
		m_CalcPlatformMand = cp;
	}
	inline CalcPlatform get_CalcPlatformJulia(void) const {
		return m_CalcPlatformJulia;
	}
	void put_CalcPlatoformJulia(CalcPlatform cp) {
		m_CalcPlatformJulia = cp;
	}
	static DWORD WINAPI CJuliasmApp::CalculateJuliaAVX(void* pArguments);

	void CalculateMandelbrot(void);
	void StartMandelbrotx87(HWND hWnd);
	void StartMandelbrotSSE(HWND hWnd);
	void StartMandelbrotSSE2(HWND hWnd);
	void StartMandelbrotAVX(HWND hWnd);
	void StartMandelbrotAVX2(HWND hWnd);

	static DWORD WINAPI CJuliasmApp::CalculateFractalSSE(void* pArguments);
	void CJuliasmApp::CalculatePointsSSE(void);

	void CalculateFractalX87(void); // Note: single-threaded

	static DWORD WINAPI CalculateFractalSSE2(void* pArguments);
	void CalculatePointsSSE2(void);





};