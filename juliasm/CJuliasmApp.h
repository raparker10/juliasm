
//
// CJuliasmApp
//
// Applicaiton class for the Juliasm application
//

class CJuliasmApp;


//
//  Stores information to be passed to and from each individual calculation thread
//
struct TThreadInfo {
	// inputs
	int iThreadIndex;			// unique thread id.  starts at zero and increases by one
	class CJuliasmApp *pApp;		// pointer to the application object. enables reaching back into the application variables

	// Thread complete message
	UINT iThreadCompleteMessage;	// message that will be sent to the message queue when the thread is complere
	WPARAM iThreadCompleteWParam;	// WPARAM of message sent to host when thread is complete
	LPARAM iThreadCompleteLParam;	// LPARAM of message sent to host when thread is complete
	int iKernelNumber;				// OpenCL kerel number to execute within program

	// outputs
	cl_int oclError;				// Status code returned by the OpenCL kernel exeution
};

//
// The types of computational resources used to perform calculations
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

//
// The different types of fractals that can be calculated.
//	As a practical metter, these serve as indexes into arrays.
//	Since this behavior is not really type-safe it might be necessary to 
//	find another approach for this.
//
enum FractalType {
	Mand = 0,
	Julia,
	NUMBER_FRACTAL_TYPES,
};

//
// Structure used to define the various possible fractal calculations.
//	These are essentially hardware/fractal type combinations, along with the
//	parameters needed to perform and communicate the calculation.
//
struct TCalcInfo {
	FractalType iFracType;			// type of fractal (e.g. Mand / Julia)
	CalcPlatform iCalcPlatform;		// calculation platform (e.g. x87, AVX...)
	int iThreadCount;				// number of threads to start
	DWORD (WINAPI *pCalculateMandFunc)(void* pArguments);	// calculation function
	UINT iThreadCompleteMessage;	// message to pass when complete
	WPARAM iThreadCompleteWParam;	// wParam to pass when complere
	cl_device_type	oclDeviceType;	// CPU, GPU, etc...
	int iOCLKernelNumber;			// which OCL kernel to use 
};

class CJuliasmApp : public CApplication {
	//
	// object initialization functions
	//
	void Initialize(void);
	void InitializeMand(void);
	void InitializeJulia(void);


	//
	// Font(s) for displaying text
	//
	NONCLIENTMETRICS m_ncm;
	LOGFONT m_lfInfo;
	HFONT m_hfInfo;
	TEXTMETRIC m_tmInfo;

	//
	// Color Palette
	//
	 CPalette m_PaletteDefault;

	 //
	 // CPU feature identification
	 //
	CCPU cpu;

	//
	// dialog box callbacks
	//

	// displays information about the system's CPU
	static INT_PTR CALLBACK HelpCPU(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	//
	// Time / performance measurement
	//
	 LARGE_INTEGER m_tTotal;
	 LARGE_INTEGER m_ticksPerSecond;
 	 LARGE_INTEGER 
			// worker thread duration tracking
			m_tThreadDuration[FractalType::NUMBER_FRACTAL_TYPES][MAX_CPU_THREADS],		// duration of each thread
			m_tThreadDurationTotal[FractalType::NUMBER_FRACTAL_TYPES],					// total duration of all threads

			// duration experienced by the operator - clock time
			m_tProcessStart[FractalType::NUMBER_FRACTAL_TYPES],				// start time of all worker threads
			m_tProcessStop[FractalType::NUMBER_FRACTAL_TYPES],				// stop time of all worker threads
			m_tProcessDurationTotal[FractalType::NUMBER_FRACTAL_TYPES];		// total clock duration


	// 
	// Calculation & threading
	//
	const int			THREAD_STACK_SIZE = 1024;
	static TCalcInfo	m_ci[];

	DWORD (WINAPI		*m_pCalculateFunc[FractalType::NUMBER_FRACTAL_TYPES])(void* pArguments);

	UINT	m_CalcCompleteMessage[FractalType::NUMBER_FRACTAL_TYPES];
	WPARAM	m_CalcCompleteWParam[FractalType::NUMBER_FRACTAL_TYPES];
	LPARAM	m_CalcCompleteLParam[FractalType::NUMBER_FRACTAL_TYPES];

	int				m_iCalcKernel[FractalType::NUMBER_FRACTAL_TYPES];

	volatile LONG	m_iCalculatingFractal[FractalType::NUMBER_FRACTAL_TYPES];
	HANDLE			m_hThread[FractalType::NUMBER_FRACTAL_TYPES][MAX_CPU_THREADS];
	LONG			m_iCalcThreadCount[FractalType::NUMBER_FRACTAL_TYPES];
	TThreadInfo		m_ThreadInfo[FractalType::NUMBER_FRACTAL_TYPES][MAX_CPU_THREADS];

	CalcPlatform	m_CalcPlatform[NUMBER_FRACTAL_TYPES];

	bool StartCalc(FractalType ft);
	void PostCalc(FractalType ft);
	void PostCalcAll(void);

	bool put_OCLDeviceType(COpenCLImage *ocl, cl_device_type oclDeviceType, int iImageWidth, int iImageHeight);
	cl_device_type get_OCLDeviceType(COpenCLImage *ocl) const { return ocl->get_CurrentDeviceType(); }
	char *get_CalcPlatformName(char *szBuf, size_t iLen, CalcPlatform cp);

	inline CalcPlatform get_CalcPlatform(FractalType fracType) const { return m_CalcPlatform[fracType];	}
	bool put_CalcPlatform(FractalType ft, CalcPlatform cp);

	void put_MaxIterationsMand(int iMaxIterationsMand);
	inline int get_MaxIterationsMand(void) const { return this->m_iMaxIterationsMand; }

	void put_MaxIterationsJulia(int iMaxIterationsJulia);
	inline int get_MaxIterationsJulia(void) const { return this->m_iMaxIterationsJulia; }


	//
	// bounding box
	// RAP: this is work in progress
	 //
//	 double m_ac, m_bc, m_da, m_db;
/*	 void put_Boundary(double a1, double b1, double a2, double b2)
	 {
		 m_a1 = a1;
		 m_b1 = b1;
		 m_a2 = a2;
		 m_b2 = b2;

		 m_ac = (a2 - a1) / 2;
		 m_bc = (b2 - b1) / 2;

	 }
*/
	// per-pixel offsets in the horizontal (da) and vertical (db) directions
//	 double m_da, m_db;


	//
	// mouse dragging
	//

	// true if a drag operation is underway
	bool m_bDragging;
	FractalType m_DragFractal;	// which fractal is being dragged

	// location where the drag operation began
	int m_iDragStartX, 
		m_iDragStartY;

	// location where the drag operation is currently centered
	int m_iDragCurrentX, 
		m_iDragCurrentY;

	// location where the drag operation ended
	int m_iDragEndX,	
		m_iDragEndY;

	double m_start_a1, 
		m_start_b1, 
		m_start_a2, 
		m_start_b2;

	double m_start_jc1, 
		m_start_jc2,	// the Julia Set numerical bounding box
		m_start_jd1, 
		m_start_jd2;



	// These are broken out separately because they drive the 
	// size and positioning of the display of all information.
	// The percentage of the screen used by the Mandelbrot set layout
	//	determines where and how the other items will be positioned.
	//
	CBitmap256Color m_bmpFractal[FractalType::NUMBER_FRACTAL_TYPES]; // Mandelbrot;
	int				m_iMandScreenPct;
	bool			m_bShowFractal[FractalType::NUMBER_FRACTAL_TYPES];

	bool put_MandScreenPct(int iPct);
	int get_MandScreenPct(void)				const		{ return m_iMandScreenPct; }
	bool get_ShowFractal(FractalType ft)	const		{ return m_bShowFractal[ft]; }
	void put_ShowFractal(FractalType ft, bool bShow)	 {m_bShowFractal[ft] = bShow;}
	
	//
	// screen layout functions for the Mandelbrot Set
	//
	COpenCLMand		m_OCLMand;
	RECT			m_rcMand;
	int				_m_iMandHeight, _m_iMandWidth;
	double			m_a1, m_a2, m_b1, m_b2; // numerical bounding box
	int				m_iMaxIterationsMand;

	inline int get_MandHeight(void) const { return _m_iMandHeight; }
	inline int get_MandWidth(void) const { return _m_iMandWidth; }
	bool put_MandRect(int x1, int y1, int x2, int y2, bool bShow);

	void ZoomInMand(int x, int y, float fAmount);
	void ZoomInJulia(int x, int y, float fAmount);
	//
	// screen layout for the Julia Set
	//
	COpenCLJulia	m_OCLJulia;
	RECT			m_rcJulia;
	int				_m_iJuliaHeight, _m_iJuliaWidth;
	inline int get_JuliaHeight(void) const { return _m_iJuliaHeight; }
	inline int get_JuliaWidth(void) const { return _m_iJuliaWidth; }
	bool put_JuliaRect(int x1, int y1, int x2, int y2, bool bShow);
	bool put_JuliaPoint(double a, double b) {
		if (m_bUpdateJulia) {
			m_ja = a;
			m_jb = b;
			// update the OpenCL object as well
			m_OCLJulia.put_ConstPoint((float)m_ja, (float)m_jb);
			return true;
		}
		return false;
	}

	//
	// Julia set parameters
	//
	 double m_ja, m_jb;		// the Mandelbrot set point for which the Julia set is being calculated
	 double m_jc1, m_jc2;	// the Julia Set numerical bounding box
	 double m_jd1, m_jd2;
	 int m_iMaxIterationsJulia;

	//
	// Screen Layout for the Information Panel
	//
	RECT	m_rcInfoPanel;
	int		_m_iInfoPanelHeight, _m_iInfoPanelWidth;
	bool	m_bShowInfoPanel;
	inline int get_InfoPanelHeight(void) const { return _m_iInfoPanelHeight; }
	inline int get_InfoPanelWidth(void) const { return _m_iInfoPanelWidth; }
	inline bool get_ShowInfoPanel(void) const { return m_bShowInfoPanel; }
	bool put_InfoRect(int x1, int y1, int x2, int y2, bool bShow);

	//
	// message handlers overridden from parent CApplication object
	//
	virtual bool handle_paint(HWND hWnd, HDC hdc, LPPAINTSTRUCT ps);
	virtual bool handle_create(HWND hWnd, LPCREATESTRUCT *lpcs);
	virtual bool handle_command(HWND hWnd, int wmID, int wmEvent, LPARAM lParam);
	virtual bool handle_keydown(HWND hWnd, int iVKey);
	virtual bool handle_char(HWND hWnd, int iChar);
	virtual bool handle_vscroll(HWND hWnd, int iScrollRequest, int iScrollPosition);
	virtual bool handle_size(HWND hWnd, HDC hdc, int iSizeType, int iWidth, int iHeight);
	virtual bool handle_mousemove(HWND hWnd, WPARAM wParam, WORD x, WORD y);
	virtual bool handle_lbuttondoubleclick(HWND hWnd, WPARAM wvKeyDown, WORD x, WORD y);
	virtual bool handle_mousewheel(HWND hWnd, WORD wvKeys, int iRotationAmount, int x, int y);
	virtual bool handle_lbuttondown(HWND hWnd, WORD wvKeys, int x, int y);
	virtual bool handle_lbuttonup(HWND hWnd, WORD	wvKeys, int x, int y);	
	virtual bool handle_rbuttondown(HWND hWnd, WORD wvKeys, int x, int y);
	virtual bool handle_rbuttonup(HWND hWnd, WORD	wvKeys, int x, int y);

	//
	// Menu Update functions
	//
	void UpdateCalcPlatformMenuMand();
	void UpdateCalcPlatformMenuJulia();

	// 
	// Julia set static calculation functions used for multi-threading
	//
	static DWORD WINAPI CalculateJuliaX87(void* pArguments);
	static DWORD WINAPI CalculateJuliaSSE(void* pArguments);
	static DWORD WINAPI CalculateJuliaSSE2(void* pArguments);
	static DWORD WINAPI CalculateJuliaAVX32(void* pArguments);
	static DWORD WINAPI CalculateJuliaAVX64(void* pArguments);
	static DWORD WINAPI CalculateJuliaFMA(void* pArguments);
	static DWORD WINAPI CalculateJuliaOpenCL(void* pArguments);

	//
	// Mandelbrot set static calculation functions used for multi-threading
	//
	static DWORD WINAPI CalculateMandX87(void* pArguments);
	static DWORD WINAPI CalculateMandSSE(void* pArguments);
	static DWORD WINAPI CalculateMandSSE2(void* pArguments);
	static DWORD WINAPI CalculateMandAVX32(void* pArguments);
	static DWORD WINAPI CalculateMandAVX64(void* pArguments);
	static DWORD WINAPI CalculateMandFMA(void* pArguments);
	static DWORD WINAPI CalculateMandOpenCL(void* pArguments);

	//
	// Julia set management
	//
	bool m_bUpdateJulia;

public:
	// constructor / destructor
	CJuliasmApp();
	~CJuliasmApp();
};