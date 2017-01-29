// juliasm.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#define MAX_LOADSTRING 100


// ******************************
//
// bitmap variables
//

//
// window used to obtain DC characteristics
//
HWND l_hWnd;
HWND hWndMain;

//
// drawing surface for the mandelbrot set
//
HBITMAP l_hbmpCanvas;
BITMAPINFO l_bmi;
PBITMAPINFO l_pbmi;
VOID *l_ppvBits;

//
// drawing surface for the julia set
//
HBITMAP l_hbmpCanvasJulia;
BITMAPINFO l_bmiJulia;
PBITMAPINFO l_pbmiJulia;
VOID *l_ppvBitsJulia;

// *******************************
// Time / performance measurement
//
LARGE_INTEGER tTotal;
LARGE_INTEGER ticksPerSecond;
LARGE_INTEGER iTicks[MAX_THREADS];
LARGE_INTEGER iTotalTicks;

//
// Julia set parameters
//
float ja_sse, jb_sse;
float jc1_sse = -1.0, jc2_sse = 1.0;
float jd1_sse = -1.0, jd2_sse = 1.0;

int jiMaxIterations = 1024;
int iJMaxIter = 0;
int iJMaxThread = 0;

//
// Color Palette
//
CPalette PaletteDefault;

//
// Thread management
//
static LONG iMandelbrotThreads = 0;
static LONG iJuliaThreads = 0;
static volatile LONG iJuliaReady[MAX_THREADS] = { 0, 0, 0, 0 };

//
// Program initialization
//
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


//
// Bitmap functions for the mandelbrot and julia set drawing surfaces
//

// defaults the bitmap to a specified color
void DefaultColor(unsigned int *ppvBits, int iHeight, int iWidth, unsigned int rgbColor)
{
	for (int i = 0; i < iHeight * iWidth; ++i)
	{
		ppvBits[i] = rgbColor;
	}
}

// initializes the mandelbrot set's bitmap
void InitializeCanvas(int iHeight, int iWidth, HWND hWnd)
{
		// initialize the bitmapinfo structure
		HDC hdc = GetDC(hWnd);
		l_bmi.bmiHeader.biSize = sizeof(l_bmi.bmiHeader);
		l_bmi.bmiHeader.biWidth = iWidth;
		l_bmi.bmiHeader.biHeight = -iHeight;
		l_bmi.bmiHeader.biPlanes = 1;
		l_bmi.bmiHeader.biBitCount = 32;
		l_bmi.bmiHeader.biCompression = BI_RGB;
		l_bmi.bmiHeader.biSizeImage = 0;
		l_bmi.bmiHeader.biXPelsPerMeter = 0;
		l_bmi.bmiHeader.biYPelsPerMeter = 0;
		l_bmi.bmiHeader.biClrUsed = 0;
		l_bmi.bmiHeader.biClrImportant = 0;
		l_hbmpCanvas = CreateDIBSection(hdc, &l_bmi, DIB_RGB_COLORS, &l_ppvBits, NULL, 0);
		ReleaseDC(hWnd, hdc);
		DefaultColor((unsigned int*)l_ppvBits, FRACTAL_HEIGHT, FRACTAL_WIDTH, RGB(255, 255, 255));
}

// initializes the julia set's bitmap
void InitializeCanvasJulia(int iHeight, int iWidth, HWND hWnd)
{
		// initialize the bitmapinfo structure
		HDC hdc = GetDC(hWnd);
		l_bmiJulia.bmiHeader.biSize = sizeof(l_bmiJulia.bmiHeader);
		l_bmiJulia.bmiHeader.biWidth = iWidth;
		l_bmiJulia.bmiHeader.biHeight = -iHeight;
		l_bmiJulia.bmiHeader.biPlanes = 1;
		l_bmiJulia.bmiHeader.biBitCount = 32;
		l_bmiJulia.bmiHeader.biCompression = BI_RGB;
		l_bmiJulia.bmiHeader.biSizeImage = 0;
		l_bmiJulia.bmiHeader.biXPelsPerMeter = 0;
		l_bmiJulia.bmiHeader.biYPelsPerMeter = 0;
		l_bmiJulia.bmiHeader.biClrUsed = 0;
		l_bmiJulia.bmiHeader.biClrImportant = 0;
		l_hbmpCanvasJulia = CreateDIBSection(hdc, &l_bmiJulia, DIB_RGB_COLORS, &l_ppvBitsJulia, NULL, 0);
		ReleaseDC(hWnd, hdc);
		DefaultColor((unsigned int*)l_ppvBitsJulia, JULIA_HEIGHT, JULIA_WIDTH, RGB(255, 255, 255));
		int i = sizeof(int);
}

//
// SSE variables
//

// orbit saving
unsigned short bSaveOrbit = 0;
unsigned short iOrbitIndex = 0;;
unsigned short iOrbitPoints = 1024;
float orbit_c_sse[1024];
float orbit_d_sse[1024];

// bounding box
double a1=-2.0f, a2=2.0f, b1=-2.0f, b2=2.0f;
/*double 
	a1=+0.302565, 
	a2=+0.326177, 
	b1=-0.456631, 
	b2=-0.433020;
*/
// per-pixel offsets in the horizontal (da) and vertical (db) directions
double da, db;
unsigned int iMaxIterations = 1024;


// inputs
// a's in xmm3
// b's in xmm3
// maximums (e.g. 4) in xmm7
void CalculatePointsSSE()
{
	__asm {
		// place the maximum number of iterations in AX
		mov ax, WORD PTR iMaxIterations

		mov bl, BYTE PTR bSaveOrbit
		xor edx, edx	// reset the orbit save index

		// place 0 in c, d, and iterations
		xorps xmm0, xmm0
		xorps xmm1, xmm1
		xorps xmm6, xmm6

iterate:
		// calculate c * d and place in xmm2
		movaps xmm2, xmm0	// c
		mulps xmm2, xmm1	// * d
		addps xmm2, xmm2	// * 2

		// save the orbit point
//		test bl,bl
//		jnz save_orbit
save_orbit_resume:

		// calulate c^2
		mulps xmm0, xmm0

		// calculate d^2
		mulps xmm1, xmm1

		//
		// calculate the magnitude of the n^2 new values and store them in XMM5
		//
		movaps xmm5, xmm0
		addps xmm5, xmm1

		// calculate c^2-d^2
		subps xmm0, xmm1

		// get the new d
		movaps xmm1, xmm2

		// add a to c^2
		addps xmm0, xmm3

		// add b to d^2
		addps xmm1, xmm4

		// compare the previous iteration to the maximum value
		cmpltps xmm5, xmm7		// campare to max range
		pmovmskb ecx, xmm5
		andps xmm5, xmm7		// use result as bitmask for counter to generate increment
		addps xmm6, xmm5		// add increment to counter

		test ecx,ecx
		jz done

/*		cmp ax, 10
		jne tmp
		nop

tmp:
*/
		dec ax
		jnz iterate
		jmp done

save_orbit:
	/*
unsigned short bSaveOrbit = 1;

unsigned short iOrbitIndex = 0;;
unsigned short iOrbitPoints = 128;

float orbit_c_sse[128];
float orbit_d_sse[128];
*/
		// save the first value of C
//		mov cx, DWORD PTR [orbit_c_sse + bh]

		cmp edx, 1024
		jge save_orbit_resume
		movss DWORD PTR orbit_c_sse[4 * edx], xmm0
		movss DWORD PTR orbit_d_sse[4 * edx], xmm1
		inc edx;
//		mov orbit_c_sse[ebx], 
		jmp save_orbit_resume

done:
		// adjust the counts by 4
		divps xmm6, xmm7
//		movaps oword ptr[iterations_sse], xmm6

	}
}
void CalculatePointsSSE2()
{
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

		test ecx,ecx
		jz done
		dec ax
		jnz iterate
		jmp done

done:
		// adjust the counts by 4
		divpd xmm6, xmm7
//		movapd oword ptr[iterations_sse2], xmm6

	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	//
	// setup the palette
	//
	// setup the default palette

	// setup the red color channel
	PaletteDefault.PushColorPoint(0, CPixelPoint(0,0));
	PaletteDefault.PushColorPoint(0, CPixelPoint(12, 255));
	PaletteDefault.PushColorPoint(0, CPixelPoint(200, 255));
	PaletteDefault.PushColorPoint(0, CPixelPoint(255, 0));

	// setup the green color channel
	PaletteDefault.PushColorPoint(1, CPixelPoint(0, 0));
	PaletteDefault.PushColorPoint(1, CPixelPoint(4, 255));
	PaletteDefault.PushColorPoint(1, CPixelPoint(128, 0));
	PaletteDefault.PushColorPoint(1, CPixelPoint(192, 255));
	PaletteDefault.PushColorPoint(1, CPixelPoint(255, 255));

	// setup the blue color channel
	PaletteDefault.PushColorPoint(2, CPixelPoint(0, 0));
	PaletteDefault.PushColorPoint(2, CPixelPoint(255 / 6, 255));
	PaletteDefault.PushColorPoint(2, CPixelPoint(255 / 3, 0));
	PaletteDefault.PushColorPoint(2, CPixelPoint(128, 255));
	PaletteDefault.PushColorPoint(2, CPixelPoint(255 * 2 / 3, 0));
	PaletteDefault.PushColorPoint(2, CPixelPoint(255 * 5 / 6, 255));
	PaletteDefault.PushColorPoint(2, CPixelPoint(255, 0));

	// create a palette from the color points
	PaletteDefault.UpdateColors();

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	QueryPerformanceFrequency(&ticksPerSecond);


 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_JULIASM, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JULIASM));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
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
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JULIASM));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_JULIASM);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   hWndMain = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

// pragma optimize("", off )
DWORD WINAPI CalculateJulia( void* pArguments )
{
	int iThread = (int)pArguments;

	//
	// wait for a message to calculate the picture
	//

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); // force the thread to create a message queue
	InterlockedIncrement(&iJuliaReady[iThread]);

restart:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		if (msg.message == WM_COMMAND && msg.wParam == 1)
			break;
	}

	if (iThread == 0)
		iJMaxIter = 0;

	//	SetThreadAffinityMask(GetCurrentThread(), 4);
	volatile int heightPixels = JULIA_HEIGHT / iJuliaThreads;
	volatile int x, y;
	volatile float a2, b2, ab2;
	volatile float c2, d2, cd2;
	volatile float a, b, c, d;
	volatile float mag;
	volatile float dc = (jc2_sse - jc1_sse) / JULIA_WIDTH;
	volatile float dd = (jd2_sse - jd1_sse) / JULIA_HEIGHT;
	volatile float heightNumeric = (jd2_sse - jd1_sse) / iJuliaThreads;

	volatile 	int startY = iThread * heightPixels;
	volatile int stopY = startY + heightPixels;


	volatile __declspec(align(16)) float a_sse[4] = { ja_sse, ja_sse, ja_sse, ja_sse };
	volatile __declspec(align(16)) float b_sse[4] = { jb_sse, jb_sse, jb_sse, jb_sse };
	volatile __declspec(align(16)) float c_sse[4];
	volatile __declspec(align(16)) float d_sse[4];

	volatile __declspec(align(16)) float maximum_sse[4] = {4.0f, 4.0f, 4.0f, 4.0f};
	volatile __declspec(align(16)) float iterations_sse[4];
	volatile __declspec(align(16)) float da_sse[4];


	a = ja_sse;
	b = jb_sse;


	d = jd1_sse + heightNumeric * iThread;
	
	// load the maximum magnitude into xmm7
	__asm movaps xmm7, oword ptr [maximum_sse]
	__asm movaps xmm3, oword ptr [a_sse]
	__asm movaps xmm4, oword ptr [b_sse]

	for (y = startY; y < stopY; ++y)
	{
			d_sse[0] = d;
			d_sse[1] = d;
			d_sse[2] = d;
			d_sse[3] = d;

		c = jc1_sse;
		for (x = 0; x < JULIA_WIDTH; x += POINTS_CONCURRENT_SSE)
		{
			c_sse[0] = c; c += dc;
			c_sse[1] = c; c += dc;
			c_sse[2] = c; c += dc;
			c_sse[3] = c; c += dc;

			__asm {
				movaps xmm0, oword ptr [c_sse]
				movaps xmm1, oword ptr [d_sse]
				xorps xmm6, xmm6
				movaps oword ptr [iterations_sse], xmm6
			}


				__asm {
					mov ax, WORD PTR jiMaxIterations
iterate:
					// 2 * c * d
					movaps xmm2, xmm0	// c
					mulps xmm2, xmm1	// * d
					addps xmm2, xmm2	// * 2

					// c ^ 2
					mulps xmm0, xmm0

					// d ^ 2
					mulps xmm1, xmm1

					// magnitude
					movaps xmm5, xmm0
					addps xmm5, xmm1

					// compare the previous iteration to the maximum value
					xor ecx, ecx
					cmpltps xmm5, xmm7		// campare to max range
					pmovmskb ecx, xmm5
					andps xmm5, xmm7		// use result as bitmask for counter to generate increment
					addps xmm6, xmm5		// add increment to counter

					test ecx,ecx
					jz done

					// generate the new c
					subps xmm0, xmm1	// subtract d^2
					addps xmm0, xmm3	// add a

					// generate the new d
					movaps xmm1, xmm2	// get 2*c*d
					addps xmm1, xmm4	// add b

					dec ax
					jnz iterate
done:	
					divps xmm6, xmm7
					movaps oword ptr [iterations_sse], xmm6
				}
					

			if (iterations_sse[0] >= jiMaxIterations)
				iterations_sse[0] = 0;
			if (iterations_sse[1] >= jiMaxIterations)
				iterations_sse[1] = 0;
			if (iterations_sse[2] >= jiMaxIterations)
				iterations_sse[2] = 0;
			if (iterations_sse[3] >= jiMaxIterations)
				iterations_sse[3] = 0;

			((unsigned int*)l_ppvBitsJulia)[y * JULIA_WIDTH + x+0] = RGB(iterations_sse[0], iterations_sse[0] / 2, iterations_sse[0] / 3);
			((unsigned int*)l_ppvBitsJulia)[y * JULIA_WIDTH + x+1] = RGB(iterations_sse[1], iterations_sse[1] / 2, iterations_sse[1] / 3);
			((unsigned int*)l_ppvBitsJulia)[y * JULIA_WIDTH + x+2] = RGB(iterations_sse[2], iterations_sse[2] / 2, iterations_sse[2] / 3);
			((unsigned int*)l_ppvBitsJulia)[y * JULIA_WIDTH + x+3] = RGB(iterations_sse[3], iterations_sse[3] / 2, iterations_sse[3] / 3);

			if (iThread == 0)
			{
				if (iterations_sse[0] > iJMaxIter)
					iJMaxIter = iterations_sse[0];
				if (iterations_sse[1] > iJMaxIter)
					iJMaxIter = iterations_sse[1];
				if (iterations_sse[2] > iJMaxIter)
					iJMaxIter = iterations_sse[2];
				if (iterations_sse[3] > iJMaxIter)
					iJMaxIter = iterations_sse[3];
			}

		}
		d = d + dd;
	}
	PostMessage(hWndMain, WM_COMMAND, IDM_THREADCOMPLETEJULIA, 0);
	goto restart;
	return 0;
}
// pragma optimize("", on )

// pragma optimize("", off )
DWORD WINAPI CalculateJuliaAVXOld(void* pArguments)
{
	int iThread = (int)pArguments;

	//
	// wait for a message to calculate the picture
	//

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); // force the thread to create a message queue
	InterlockedIncrement(&iJuliaReady[iThread]);

restart:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		if (msg.message == WM_COMMAND && msg.wParam == 1)
			break;
	}

	if (iThread == 0)
		iJMaxIter = 0;

	LARGE_INTEGER iTicksStart;
	LARGE_INTEGER iTicksEnd;

	QueryPerformanceCounter(&iTicksStart);

	//	SetThreadAffinityMask(GetCurrentThread(), 4);
	volatile int heightPixels = JULIA_HEIGHT / iJuliaThreads;
	volatile int x, y;
	__declspec(align(32)) float a, b, c, d;
	volatile float dc = (jc2_sse - jc1_sse) / JULIA_WIDTH;
	volatile float dd = (jd2_sse - jd1_sse) / JULIA_HEIGHT;
	volatile float heightNumeric = (jd2_sse - jd1_sse) / iJuliaThreads;

	volatile 	int startY = iThread * heightPixels;
	volatile int stopY = startY + heightPixels;
	int i;
	 

	volatile __declspec(align(32)) float a_avx[POINTS_CONCURRENT_AVX32] = { ja_sse, ja_sse, ja_sse, ja_sse, ja_sse, ja_sse, ja_sse, ja_sse };
	volatile __declspec(align(32)) float b_avx[POINTS_CONCURRENT_AVX32] = { jb_sse, jb_sse, jb_sse, jb_sse, jb_sse, jb_sse, jb_sse, jb_sse };
	volatile __declspec(align(32)) float c_avx[POINTS_CONCURRENT_AVX32];
	volatile __declspec(align(32)) float d_avx[POINTS_CONCURRENT_AVX32];
	volatile __declspec(align(32)) float dc_avx[POINTS_CONCURRENT_AVX32] = { 8.0f * dc, 8.0f * dc, 8.0f * dc, 8.0f * dc, 8.0f * dc, 8.0f * dc, 8.0f * dc, 8.0f * dc };
	volatile __declspec(align(32)) float ic_avx[POINTS_CONCURRENT_AVX32] = { jc1_sse, jc1_sse + 1.0f * dc, jc1_sse + 2.0f * dc, jc1_sse + 3.0f * dc, jc1_sse + 4.0f * dc, jc1_sse + 5.0f * dc, jc1_sse + 6.0f * dc, jc1_sse + 7.0f * dc };


	const __declspec(align(32)) float maximum_avx[POINTS_CONCURRENT_AVX32] = { 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f };
	volatile __declspec(align(32)) float iterations_avx[POINTS_CONCURRENT_AVX32];


	a = ja_sse;
	b = jb_sse;


	d = jd1_sse + heightNumeric * iThread;

	// load the maximum magnitude into xmm7
	__asm vmovaps ymm7, ymmword ptr[maximum_avx]
	__asm vmovaps ymm3, ymmword ptr[a_avx]
	__asm vmovaps ymm4, ymmword ptr[b_avx]
//	__asm vmovaps ymm10, ymmword ptr[a_avx]


		for (y = startY; y < stopY; ++y)
		{
			__asm vbroadcastss ymm0, [d]
			__asm vmovaps ymmword ptr[d_avx], ymm0

			__asm vmovaps ymm0, ymmword ptr[ic_avx]
			__asm vmovaps ymmword ptr[c_avx], ymm0
				for (x = 0; x < JULIA_WIDTH; x += POINTS_CONCURRENT_AVX32)
			{

				__asm {
					vmovaps ymm0, ymmword ptr[c_avx]
					vmovaps ymm1, ymmword ptr[d_avx]
					vxorps ymm6, ymm6, ymm6
					vmovaps ymmword ptr[iterations_avx], ymm6
				}

				__asm {
					mov ax, WORD PTR jiMaxIterations
					iterate :
					// 2 * c * d
						vmulps ymm2, ymm0, ymm1	// c * d
						vaddps ymm2, ymm2, ymm2	// + c * d

						vmulps ymm0, ymm0, ymm0	// c^2
						vmulps ymm1, ymm1, ymm1	// d ^ 2

						vaddps ymm5, ymm0, ymm1	// magnitude

						// compare the previous iteration to the maximum value
						xor ecx, ecx
						vcmpltps ymm5, ymm5, ymm7		// campare to max range
						vpmovmskb ecx, ymm5
						vandps ymm5, ymm5, ymm7		// use result as bitmask for counter to generate increment
						vaddps ymm6, ymm6, ymm5		// add increment to counter

						test ecx, ecx
						jz done

						// generate the new c
						vsubps ymm0, ymm0, ymm1	// subtract d^2
						vaddps ymm0, ymm0, ymm3	// add a

						// generate the new d
						vaddps ymm1, ymm2, ymm4	// add b

						dec ax
						jnz iterate
					done :
					vdivps ymm6, ymm6, ymm7
					vmovaps ymmword ptr[iterations_avx], ymm6
				}

				for (i = 0; i < POINTS_CONCURRENT_AVX32; ++i)
				{
					if (iterations_avx[i] >= jiMaxIterations)
						iterations_avx[i] = 0;
					((unsigned int*)l_ppvBitsJulia)[y * JULIA_WIDTH + x + i] = RGB(iterations_avx[i], iterations_avx[i] / 2, iterations_avx[i] / 3);
				}
				
				__asm vmovaps ymm0, ymmword ptr[c_avx]
				__asm vaddps ymm0, ymm0, ymmword ptr[dc_avx]
				__asm vmovaps ymmword ptr[c_avx], ymm0
//					for (i = 0; i < POINTS_CONCURRENT_AVX32; ++i, c += dc)
//					c_avx[i] = c;

			}
			d = d + dd;
		}
		QueryPerformanceCounter(&iTicksEnd);
		iTicks[iThread].QuadPart = (iTicksEnd.QuadPart - iTicksStart.QuadPart) * 1000 / ticksPerSecond.QuadPart;
		PostMessage(hWndMain, WM_COMMAND, IDM_THREADCOMPLETEJULIA, 0);
		goto restart;
		return 0;
}

// pragma optimize("", on )

DWORD WINAPI CalculateJuliaAVX(void* pArguments)
{
	int iThread = (int)pArguments;

	//
	// wait for a message to calculate the picture
	//

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); // force the thread to create a message queue
	InterlockedIncrement(&iJuliaReady[iThread]);

restart:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		if (msg.message == WM_COMMAND && msg.wParam == 1)
			break;
	}

	if (iThread == 0)
		iJMaxIter = 0;

	LARGE_INTEGER iTicksStart;
	LARGE_INTEGER iTicksEnd;

	QueryPerformanceCounter(&iTicksStart);

	//	SetThreadAffinityMask(GetCurrentThread(), 4);
	volatile int heightPixels = JULIA_HEIGHT / iJuliaThreads;
	volatile int x, y;
	volatile __declspec(align(32)) float a, b, start_df;
	volatile float fdc = (jc2_sse - jc1_sse) / JULIA_WIDTH;
	volatile float fdd = (jd2_sse - jd1_sse) / JULIA_HEIGHT;
	volatile float heightNumeric = (jd2_sse - jd1_sse) / iJuliaThreads;

	volatile 	int startY = iThread * heightPixels;
	volatile int stopY = startY + heightPixels;
	int i;


	const __declspec(align(32)) float a_avx[POINTS_CONCURRENT_AVX32] = { ja_sse, ja_sse, ja_sse, ja_sse, ja_sse, ja_sse, ja_sse, ja_sse };
	const __declspec(align(32)) float b_avx[POINTS_CONCURRENT_AVX32] = { jb_sse, jb_sse, jb_sse, jb_sse, jb_sse, jb_sse, jb_sse, jb_sse };
	__declspec(align(32)) float dc_avx[POINTS_CONCURRENT_AVX32] = { 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc, 8.0f * fdc };
	__declspec(align(32)) float ic_avx[POINTS_CONCURRENT_AVX32] = { jc1_sse, jc1_sse + 1.0f * fdc, jc1_sse + 2.0f * fdc, jc1_sse + 3.0f * fdc, jc1_sse + 4.0f * fdc, jc1_sse + 5.0f * fdc, jc1_sse + 6.0f * fdc, jc1_sse + 7.0f * fdc };
	__declspec(align(32)) float c_avx[POINTS_CONCURRENT_AVX32];
	__declspec(align(32)) float d_avx[POINTS_CONCURRENT_AVX32];

	const __declspec(align(32)) float maximum_avx[POINTS_CONCURRENT_AVX32] = { 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f };
	__declspec(align(32)) float iterations_avx[POINTS_CONCURRENT_AVX32];
	const __declspec(align(32)) float zero_avx[POINTS_CONCURRENT_AVX32] = { 0, 0, 0, 0, 0, 0, 0, 0 };
//	const __declspec(align(32)) float increment_avx[POINTS_CONCURRENT_AVX32] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	__declspec(align(32)) float increment_avx[POINTS_CONCURRENT_AVX32];

	a = ja_sse;
	b = jb_sse;


	start_df = jd1_sse + heightNumeric * iThread;

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
			
			for (x = 0; x < JULIA_WIDTH; x += POINTS_CONCURRENT_AVX32)
			{
				// initilize c, d, and the iteration count
				__m256 iterations = _mm256_setzero_ps();
				for (int ii = 0; ii < POINTS_CONCURRENT_AVX32; ++ii)
				{
					increment_avx[ii] = 4.0;
				}

				for (i = 0; i < jiMaxIterations; ++i)
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
					if (iterations_avx[j] >= jiMaxIterations)
						iterations_avx[j] = 0;
					((unsigned int*)l_ppvBitsJulia)[y * JULIA_WIDTH + x + j] = RGB(iterations_avx[j], iterations_avx[j] / 2, iterations_avx[j] / 3);
				}

				// generate the next set of 'c' values by adding the c_increment to the current c
				c = _mm256_add_ps(start_c, delta_c);
				start_c = c;
				d = start_d;
			} // next x
			start_df = start_df + fdd;
		} // next y
		QueryPerformanceCounter(&iTicksEnd);
		iTicks[iThread].QuadPart = (iTicksEnd.QuadPart - iTicksStart.QuadPart) * 1000 / ticksPerSecond.QuadPart;
		PostMessage(hWndMain, WM_COMMAND, IDM_THREADCOMPLETEJULIA, 0);
		goto restart;
		return 0;
}
// #pragma optimize("", on )

//unsigned __stdcall CalculateFractalSSE( void* pArguments )
DWORD WINAPI CalculateFractalSSE( void* pArguments )
{
	int iThread = (int)pArguments;

	double fThreadHeight = (b2 - b1) / iMandelbrotThreads;

	__declspec(align(16)) float a_sse[4];
	__declspec(align(16)) float b_sse[4];
	__declspec(align(16)) float maximum_sse[4] = {4.0f, 4.0f, 4.0f, 4.0f};
	__declspec(align(16)) float iterations_sse[4];
	__declspec(align(16)) float da_sse[4];
	__declspec(align(16)) float db_sse[4];
	__declspec(align(16)) float a1_sse[4];

	double 
		_a = a1, 
		_b = b1 + fThreadHeight * iThread, 
		da = (a2 - a1) / FRACTAL_WIDTH, 
		db = (b2 - b1) / FRACTAL_HEIGHT;

	
//	SetThreadAffinityMask(GetCurrentThread(), 1 << iThread);

	::da = da;
	::db = db;

	LARGE_INTEGER tStart, tStop;

	QueryPerformanceCounter(&tStart);

	b_sse[0] = (float)_b;
	b_sse[1] = (float)_b;
	b_sse[2] = (float)_b;
	b_sse[3] = (float)_b;

	db_sse[0] = (float)db;
	db_sse[1] = (float)db;
	db_sse[2] = (float)db;
	db_sse[3] = (float)db;

	// load the imaginary component
	__asm movaps xmm4, oword ptr [b_sse]

	// load the maximum vector size
	__asm movaps xmm7, oword ptr [maximum_sse]

	// load an array of the first a's
	_a = a1;
	a1_sse[0] = (float)_a; _a += da;
	a1_sse[1] = (float)_a; _a += da;
	a1_sse[2] = (float)_a; _a += da;
	a1_sse[3] = (float)_a; _a += da;

	da_sse[0] = (float)da * 4;
	da_sse[1] = (float)da * 4;
	da_sse[2] = (float)da * 4;
	da_sse[3] = (float)da * 4;

	int pixelHeight = FRACTAL_HEIGHT / iMandelbrotThreads;
	int starty = iThread * pixelHeight;
	int endy = starty + pixelHeight;

	unsigned int iIndex = 0 + starty * FRACTAL_WIDTH;

	for (int y = starty; y < endy; ++y)
	{
		// load the real component
		__asm movaps xmm3, a1_sse

		for (int x = 0; x < FRACTAL_WIDTH; x += 4)
		{

//			memset(orbit_c_sse, 0, sizeof(orbit_c_sse));
			CalculatePointsSSE();

			__asm movaps oword ptr[iterations_sse], xmm6

			// normalize the iteration values between 0 and 255
			((unsigned int*)l_ppvBits)[iIndex++] = (iterations_sse[0] == iMaxIterations) ? 0 : PaletteDefault.get_Color(iterations_sse[0]);
			((unsigned int*)l_ppvBits)[iIndex++] = (iterations_sse[1] == iMaxIterations) ? 0 : PaletteDefault.get_Color(iterations_sse[1]);
			((unsigned int*)l_ppvBits)[iIndex++] = (iterations_sse[2] == iMaxIterations) ? 0 : PaletteDefault.get_Color(iterations_sse[2]);
			((unsigned int*)l_ppvBits)[iIndex++] = (iterations_sse[3] == iMaxIterations) ? 0 : PaletteDefault.get_Color(iterations_sse[3]);

			// update the real components
			__asm {
				addps xmm3, oword ptr [da_sse]
			}
		}
		// update the imaginary components
		__asm addps xmm4, oword ptr [db_sse]
	}
	QueryPerformanceCounter(&tStop);
	tTotal.QuadPart = tStop.QuadPart - tStart.QuadPart;

	PostMessage(hWndMain, WM_COMMAND, IDM_THREADCOMPLETE, 0);

	return 0;
}
DWORD WINAPI CalculateFractalSSE2( void* pArguments )
{
	int iThread = (int)pArguments;

	double fThreadHeight = (b2 - b1) / iMandelbrotThreads;

	__declspec(align(16)) double a_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double b_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double maximum_sse2[POINTS_CONCURRENT_SSE2] = {4.0f, 4.0f};
	__declspec(align(16)) double iterations_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double da_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double db_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double a1_sse2[POINTS_CONCURRENT_SSE2];

	double 
		_a = a1, 
		_b = b1 + fThreadHeight * iThread, 
		da = (a2 - a1) / FRACTAL_WIDTH, 
		db = (b2 - b1) / FRACTAL_HEIGHT;

	
//	SetThreadAffinityMask(GetCurrentThread(), 1 << iThread);

	::da = da;
	::db = db;

	LARGE_INTEGER tStart, tStop;

	QueryPerformanceCounter(&tStart);

	b_sse2[0] = (double)_b;
	b_sse2[1] = (double)_b;

	db_sse2[0] = (double)db;
	db_sse2[1] = (double)db;

	// load the imaginary component
	__asm movapd xmm4, oword ptr [b_sse2]

	// load the maximum vector size
	__asm movapd xmm7, oword ptr [maximum_sse2]

	// load an array of the first a's
	_a = a1;
	a1_sse2[0] = (double)_a; _a += da;
	a1_sse2[1] = (double)_a; _a += da;

	da_sse2[0] = (double)da * POINTS_CONCURRENT_SSE2;
	da_sse2[1] = (double)da * POINTS_CONCURRENT_SSE2;

	int pixelHeight = FRACTAL_HEIGHT / iMandelbrotThreads;
	int starty = iThread * pixelHeight;
	int endy = starty + pixelHeight;

	unsigned int iIndex = 0 + starty * FRACTAL_WIDTH;

	for (int y = starty; y < endy; ++y)
	{
		// load the real component
		__asm movapd xmm3, a1_sse2

		for (int x = 0; x < FRACTAL_WIDTH; x += POINTS_CONCURRENT_SSE2)
		{

//			memset(orbit_c_sse, 0, sizeof(orbit_c_sse));
			CalculatePointsSSE2();

			__asm movapd oword ptr[iterations_sse2], xmm6


			// normalize the iteration values between 0 and 255
			((unsigned int*)l_ppvBits)[iIndex++] = (iterations_sse2[0] == iMaxIterations) ? 0 : PaletteDefault.get_Color(iterations_sse2[0]);
			((unsigned int*)l_ppvBits)[iIndex++] = (iterations_sse2[1] == iMaxIterations) ? 0 : PaletteDefault.get_Color(iterations_sse2[1]);

			// update the real components
			__asm {
				addpd xmm3, oword ptr [da_sse2]
			}
		}
		// update the imaginary components
		__asm addpd xmm4, oword ptr [db_sse2]
	}
	QueryPerformanceCounter(&tStop);
	tTotal.QuadPart = tStop.QuadPart - tStart.QuadPart;

	PostMessage(hWndMain, WM_COMMAND, IDM_THREADCOMPLETE, 0);

	return 0;
}
/*{
	double _a = a1, _b = b1, da = (a2 - a1) / FRACTAL_WIDTH, db = (b2 - b1) / FRACTAL_HEIGHT;
	unsigned int iIndex = 0;

	SetThreadAffinityMask(GetCurrentThread(), 2);

	::da = da;
	::db = db;

	LARGE_INTEGER tStart, tStop;

	QueryPerformanceCounter(&tStart);

	for (int y = 0; y < FRACTAL_HEIGHT; ++y)
	{
		b_sse2[0] = _b;
		b_sse2[1] = _b;

		_a = a1;

		for (int x = 0; x < FRACTAL_WIDTH; x += 2)
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
void CalculateFractalX87(void)
{
	double _a = a1, _b = b1, da = (a2 - a1) / FRACTAL_WIDTH, db = (b2 - b1) / FRACTAL_HEIGHT;
	unsigned int iIndex = 0;

	LARGE_INTEGER tStart, tStop;

	QueryPerformanceCounter(&tStart);

	::da = da;
	::db = db;

	_b = b1;
	for (int y = 0; y < FRACTAL_HEIGHT; ++y)
	{
		_a = a1;
		for (int x = 0; x < FRACTAL_WIDTH; x += 1)
		{
			double c = 0.0f;
			double d = 0.0f;
			double c2, d2, cd2;
			double mag;
			int i;
			for (i = 0; i < iMaxIterations; ++i)
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
			((unsigned int*)l_ppvBits)[iIndex++] = RGB(log((double)i) / log((double)iMaxIterations) * 255, log((double)i) / log((double)iMaxIterations) * 512, log((double)i) / log((double)iMaxIterations) * 768);
		}
		_b += db;
	}
	QueryPerformanceCounter(&tStop);
	tTotal.QuadPart = tStop.QuadPart - tStart.QuadPart;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int lines;
	char buf[200];
	static volatile LONG iCalculatingJulia = 0;
	static volatile LONG iCalculatingMandelbrot = 0;
	static LARGE_INTEGER tMandelbrotStart, tMandelbrotStop, tMandelbrotTotal;
	static char *szMethod = "Undefined";
	static HANDLE hThreadMandelbrotSSE[MAX_THREADS];
	static HANDLE hThreadJulia[MAX_THREADS];
	static DWORD  dwThreadJuliaID[MAX_THREADS];
	double height, width, da, db, anew, bnew;
	int x, y;
	int i;

	switch (message)
	{
	case WM_CREATE:
		InitializeCanvas(FRACTAL_HEIGHT, FRACTAL_WIDTH, hWnd);
		InitializeCanvasJulia(JULIA_HEIGHT, JULIA_WIDTH, hWnd);
//		CalculateFractalSSE();

		// create the julia thread pool
		for (i = 0; i < 4; ++i)
		{
			hThreadJulia[i] = CreateThread(NULL, 1024, CalculateJuliaAVX, (LPVOID)i, 0, &dwThreadJuliaID[i]);
		}

		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_CALCULATE_SSE:
			if (iCalculatingMandelbrot != 0)
				break;
			iMandelbrotThreads=4;
			SecureZeroMemory(hThreadMandelbrotSSE, sizeof(hThreadMandelbrotSSE));
			SecureZeroMemory(&tMandelbrotStart, sizeof(tMandelbrotStart));
			SecureZeroMemory(&tMandelbrotStop, sizeof(tMandelbrotStop));
			SecureZeroMemory(&tMandelbrotTotal, sizeof(tMandelbrotTotal));
			QueryPerformanceCounter(&tMandelbrotStart);
			szMethod = "SSE";

			for (i = 0; i < iMandelbrotThreads ; ++i)
			{
				InterlockedIncrement(&iCalculatingMandelbrot);
				hThreadMandelbrotSSE[i] = CreateThread(NULL, 1024, CalculateFractalSSE, (LPVOID)i, 0, NULL);
			}
			break;

		case IDM_CALCULATE_SSE2:
			if (iCalculatingMandelbrot != 0)
				break;
			szMethod = "SSE2";
			iMandelbrotThreads=4;
			QueryPerformanceCounter(&tMandelbrotStart);
			for (i = 0; i < iMandelbrotThreads; ++i)
			{
				InterlockedIncrement(&iCalculatingMandelbrot);
				hThreadMandelbrotSSE[i] = CreateThread(NULL, 1024, CalculateFractalSSE2, (LPVOID)i, 0, NULL);
			}
			break;

		case IDM_CALCULATE_X87:
			szMethod = "x87";
			CalculateFractalX87();
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case IDM_CLEAR:
			DefaultColor((unsigned int*)l_ppvBits, FRACTAL_HEIGHT, FRACTAL_WIDTH, RGB(255, 255, 255));
			DefaultColor((unsigned int*)l_ppvBitsJulia, JULIA_HEIGHT, JULIA_WIDTH, RGB(255, 255, 255));
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case IDM_THREADCOMPLETE:
			InterlockedDecrement(&iCalculatingMandelbrot);
			if (iCalculatingMandelbrot == 0)
			{
				// kill the threads
				for (i = 0; i < iMandelbrotThreads ; ++i)
				{
					CloseHandle(hThreadMandelbrotSSE[i]);
				}
				QueryPerformanceCounter(&tMandelbrotStop);
				tMandelbrotTotal.QuadPart = tMandelbrotStop.QuadPart - tMandelbrotStart.QuadPart;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break; 

		case IDM_THREADCOMPLETEJULIA:
			InterlockedDecrement(&iCalculatingJulia);
			for (i = 0, iTotalTicks.QuadPart = 0; i < MAX_THREADS; ++i)
				iTotalTicks.QuadPart += iTicks[i].QuadPart;

/*			if (0 == InterlockedDecrement(&iCalculatingJulia))
			{
				for (i = 0; i < iJuliaThreads; ++i)
					CloseHandle(hThreadJulia[i]);
			}
*/
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		case IDM_ITERATIONS_64: jiMaxIterations = iMaxIterations = 64; break;
		case IDM_ITERATIONS_128: jiMaxIterations = iMaxIterations = 128; break;
		case IDM_ITERATIONS_256: jiMaxIterations = iMaxIterations = 256; break;
		case IDM_ITERATIONS_512: jiMaxIterations = iMaxIterations = 512; break;
		case IDM_ITERATIONS_1024: jiMaxIterations = iMaxIterations = 1024; break;
		case IDM_ITERATIONS_2048: jiMaxIterations = iMaxIterations = 2048; break;
		case IDM_ITERATIONS_4096: jiMaxIterations = iMaxIterations = 4096; break;
		case IDM_ITERATIONS_8192: jiMaxIterations = iMaxIterations = 8192; break;
		case IDM_ITERATIONS_16384: jiMaxIterations = iMaxIterations = 16384; break;
		case IDM_ITERATIONS_32767: jiMaxIterations = iMaxIterations = 32768; break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...

		if (iCalculatingMandelbrot == 0)
		{
			lines = StretchDIBits(
					hdc, 
					0, 50, FRACTAL_WIDTH, FRACTAL_HEIGHT, 
					0, 0, FRACTAL_WIDTH, FRACTAL_HEIGHT,
					::l_ppvBits,
					&l_bmi,
					DIB_RGB_COLORS, 
					SRCCOPY);
		}

		if (!iCalculatingJulia)
		{
			lines = StretchDIBits(
					hdc, 
					FRACTAL_WIDTH, 50, JULIA_WIDTH, JULIA_HEIGHT, 
					0, 0, JULIA_WIDTH, JULIA_HEIGHT, 
					::l_ppvBitsJulia,
					&l_bmiJulia,
					DIB_RGB_COLORS, 
					SRCCOPY);
		}
//		sprintf(buf, "Mandelbrot: Duration=%u:%u, Ticks Per Clock=%u:%u, da=%e, db=%e", tMandelbrotTotal.HighPart, tMandelbrotTotal.LowPart, ticksPerSecond.HighPart, ticksPerSecond.LowPart, ::da, ::db);
		sprintf_s(buf, "Mandelbrot: Duration=%u, Ticks Per Clock=%u:%u, da=%e, db=%e", tMandelbrotTotal.QuadPart, ticksPerSecond.HighPart, ticksPerSecond.LowPart, ::da, ::db);
		TextOut(hdc, 0, 0, buf, lstrlen(buf));

		sprintf_s(buf, "Method=%s, p(1)=(%f, %f), p(2)=(%f, %f)", szMethod, a1, b1, a2, b2);
		TextOut(hdc, 0, 20, buf, lstrlen(buf));

		// display the julia set parameters
		sprintf_s(buf, "Julia: a=%f, b=%f, %s, maxiter=%d, maxthread=%d                  ", ja_sse, jb_sse, (iCalculatingJulia == 0) ? "Not Calculating" : "Calculating", iJMaxIter, iJMaxThread);
		TextOut(hdc, 500, 20, buf, lstrlen(buf));

		// display the julia set parameters
		sprintf_s(buf, "Julia: Duration=%u ms  ", iTotalTicks.QuadPart);
		TextOut(hdc, 0, 35, buf, lstrlen(buf));

		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDBLCLK:
		if (iCalculatingMandelbrot == 0)
		{
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			y = y - 50;
	
			da = (a2 - a1) / FRACTAL_WIDTH;
			db = (b2 - b1) / FRACTAL_HEIGHT;
			width = a2 - a1;
			height = b2 - b1;
			anew = a1 + da * x;
			bnew = b1 + db * y;
	
			width = width * 0.8;
			height = height * 0.8;
			a1 = anew - (width / 2.0);
			a2 = a1 + width;
	
			b1 = bnew - (height / 2.0);
			b2 = b1 + height;
	
			if (da < 0.00001)
				PostMessage(hWnd, WM_COMMAND, IDM_CALCULATE_SSE2, 0);
			else
				PostMessage(hWnd, WM_COMMAND, IDM_CALCULATE_SSE, 0);
		}
		break;

	case WM_MOUSEMOVE:
		if (iCalculatingJulia || iCalculatingMandelbrot)
			break;
	
		iJuliaThreads = JULIA_THREADS;

		x = LOWORD(lParam);
		y = HIWORD(lParam) - 50;

		ja_sse = (float)a1 + (a2 - a1) / FRACTAL_WIDTH * x;
		jb_sse = (float)b1 + (b2 - b1) / FRACTAL_HEIGHT * y;
		for (i = 0; i < iJuliaThreads; ++i)
		{
			InterlockedIncrement(&iCalculatingJulia);
			while (iJuliaReady[i] == 0)
				Sleep(100);
			if (0 == PostThreadMessage(dwThreadJuliaID[i], WM_COMMAND, 1, 0))
				MessageBox(hWnd, "PostThreadMessage failed.", "Calculate Julia", MB_OK);
		}
		break;
/*		if (iCalculatingJulia || iCalculatingMandelbrot)
			break;

		x = LOWORD(lParam);
		y = HIWORD(lParam) - 50;

		ja_sse = (float)a1 + (a2 - a1) / FRACTAL_WIDTH * x;
		jb_sse = (float)b1 + (b2 - b1) / FRACTAL_HEIGHT * y;
		iJuliaThreads = JULIA_THREADS;
		for (i = 0; i < iJuliaThreads; ++i)
		{
			InterlockedIncrement(&iCalculatingJulia);
			hThreadJulia[i] = CreateThread(NULL, 1024, CalculateJulia, (LPVOID)i, 0, NULL);
		}
		break;
*/
	case WM_MOUSEWHEEL:
		if (iCalculatingMandelbrot == 0)
		{
			WORD wTicks = HIWORD(wParam); 
			int iTicks = (short int)wTicks;

			iTicks = iTicks / WHEEL_DELTA;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			y = y - 50;
	
			da = (a2 - a1) / FRACTAL_WIDTH;
			db = (b2 - b1) / FRACTAL_HEIGHT;
			width = a2 - a1;
			height = b2 - b1;
			anew = a1 + da * x;
			bnew = b1 + db * y;
	
			if (iTicks < 0)
			{
				width = width * 1.1;
				height = height * 1.1;
			}
			else
			{
				width = width * 0.95;
				height = height * 0.95;
			}
			a1 = anew - (width / 2.0);
			a2 = a1 + width;
	
			b1 = bnew - (height / 2.0);
			b2 = b1 + height;
	
			if (da < 0.00001)
				PostMessage(hWnd, WM_COMMAND, IDM_CALCULATE_SSE2, 0);
			else
				PostMessage(hWnd, WM_COMMAND, IDM_CALCULATE_SSE, 0);
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
