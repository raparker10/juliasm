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
// Application object
//
CJuliasmApp App;


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


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

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
	wcex.lpfnWndProc = CApplication::MessageProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra = sizeof(CJuliasmApp*);
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
	   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, (LPVOID)&App);

   if (!hWnd)
   {
      return FALSE;
   }

   hWndMain = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
/*
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

	unsigned int* l_ppvBitsJulia = (unsigned int*)bmpJulia.get_bmpBits();


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
	volatile int heightPixels = iJuliaHeight / iJuliaThreads;
	volatile int x, y;
	volatile float a2, b2, ab2;
	volatile float c2, d2, cd2;
	volatile float a, b, c, d;
	volatile float mag;
	volatile float dc = (jc2_sse - jc1_sse) / iJuliaWidth;
	volatile float dd = (jd2_sse - jd1_sse) / iJuliaHeight;
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
		for (x = 0; x < iJuliaWidth; x += POINTS_CONCURRENT_SSE)
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

			((unsigned int*)l_ppvBitsJulia)[y * iJuliaWidth + x+0] = RGB(iterations_sse[0], iterations_sse[0] / 2, iterations_sse[0] / 3);
			((unsigned int*)l_ppvBitsJulia)[y * iJuliaWidth + x+1] = RGB(iterations_sse[1], iterations_sse[1] / 2, iterations_sse[1] / 3);
			((unsigned int*)l_ppvBitsJulia)[y * iJuliaWidth + x+2] = RGB(iterations_sse[2], iterations_sse[2] / 2, iterations_sse[2] / 3);
			((unsigned int*)l_ppvBitsJulia)[y * iJuliaWidth + x+3] = RGB(iterations_sse[3], iterations_sse[3] / 2, iterations_sse[3] / 3);

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
*/

/*
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

	unsigned int* l_ppvBitsJulia = (unsigned int*)bmpJulia.get_bmpBits();


	//	SetThreadAffinityMask(GetCurrentThread(), 4);
	volatile int heightPixels = iJuliaHeight / iJuliaThreads;
	volatile int x, y;
	__declspec(align(32)) float a, b, c, d;
	volatile float dc = (jc2_sse - jc1_sse) / iJuliaWidth;
	volatile float dd = (jd2_sse - jd1_sse) / iJuliaHeight;
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
				for (x = 0; x < iJuliaWidth; x += POINTS_CONCURRENT_AVX32)
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
					((unsigned int*)l_ppvBitsJulia)[y * iJuliaWidth + x + i] = RGB(iterations_avx[i], iterations_avx[i] / 2, iterations_avx[i] / 3);
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
*/

/*

*/

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
/*	static volatile LONG iCalculatingJulia = 0;
	static volatile LONG iCalculatingMandelbrot = 0;
	static LARGE_INTEGER tMandelbrotStart, tMandelbrotStop, tMandelbrotTotal;
	static char *szMethod = "Undefined";
	static HANDLE hThreadMandelbrotSSE[MAX_THREADS];
	*/
	double height, width, da, db, anew, bnew;
	int x, y;
	int i;
	static bool bCreated = false;

	switch (message)
	{
	case WM_CREATE:
		break;

	case WM_SIZE:
		break;

	case WM_COMMAND:
/* RAP
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
/*
		if (iCalculatingMandelbrot == 0)
		{
			lines = bmpMandelbrot.Show(hdc, 0, 50);
		}

		if (!iCalculatingJulia)
		{
			lines = bmpJulia.Show(hdc, iMandWidth, 50);
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
		*/
		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDBLCLK:
/*
		if (iCalculatingMandelbrot == 0)
		{
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			y = y - 50;
	
			da = (a2 - a1) / iMandWidth;
			db = (b2 - b1) / iMandHeight;
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
		*/
		break;

	case WM_MOUSEMOVE:
		/*
		if (iCalculatingJulia || iCalculatingMandelbrot)
			break;
	
		iJuliaThreads = JULIA_THREADS;

		x = LOWORD(lParam);
		y = HIWORD(lParam) - 50;

		ja_sse = (float)a1 + (a2 - a1) / iMandWidth * x;
		jb_sse = (float)b1 + (b2 - b1) / iMandHeight * y;
		for (i = 0; i < iJuliaThreads; ++i)
		{
			InterlockedIncrement(&iCalculatingJulia);
			while (iJuliaReady[i] == 0)
				Sleep(100);
			if (0 == PostThreadMessage(dwThreadJuliaID[i], WM_COMMAND, 1, 0))
				MessageBox(hWnd, "PostThreadMessage failed.", "Calculate Julia", MB_OK);
		}
		break;
		*/

/*		if (iCalculatingJulia || iCalculatingMandelbrot)
			break;

		x = LOWORD(lParam);
		y = HIWORD(lParam) - 50;

		ja_sse = (float)a1 + (a2 - a1) / iMandWidth * x;
		jb_sse = (float)b1 + (b2 - b1) / iMandHeight * y;
		iJuliaThreads = JULIA_THREADS;
		for (i = 0; i < iJuliaThreads; ++i)
		{
			InterlockedIncrement(&iCalculatingJulia);
			hThreadJulia[i] = CreateThread(NULL, 1024, CalculateJulia, (LPVOID)i, 0, NULL);
		}
		break;
*/
	case WM_MOUSEWHEEL:
		/*
		if (iCalculatingMandelbrot == 0)
		{
			WORD wTicks = HIWORD(wParam); 
			int iTicks = (short int)wTicks;

			iTicks = iTicks / WHEEL_DELTA;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			y = y - 50;
	
			da = (a2 - a1) / iMandWidth;
			db = (b2 - b1) / iMandHeight;
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
		*/
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
