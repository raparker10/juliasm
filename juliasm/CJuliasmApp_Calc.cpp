#include "stdafx.h"

//
// Calculate a Mandelbrot set using single-threaded naive x87 compilation
//
// Uses:
//	m_tMandelbrotStart, m_tMandelbrotStop, m_tMandelbrotTotal for timing
//	m_iMandHeight, m_iMandWidth
//	m_bmpMandelbrot
//	get_MaxIterationsMand()
//	m_paletteDefault
//	get_CalcPlatformMand()
void CJuliasmApp::CalculateMandX87(void)
{
	// initialize variables
	double _a = m_a1, 
		_b = m_b1, 
		da = (m_a2 - m_a1) / m_iMandWidth, 
		db = (m_b2 - m_b1) / m_iMandHeight;

	unsigned int iIndex = 0;
	int iThreadIndex = 0; // default to thread 0 until multithreading is implemented

	// set the x87 as the current calculation platform
	if (get_CalcPlatformMand() != CalcPlatform::x87)
		put_CalcPlatformMand(CalcPlatform::x87);


	// setup the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);

	//get the bitmap bits
	unsigned int* l_ppvBits = (unsigned int*)m_bmpMandelbrot.get_bmpBits();

	m_da = da;
	m_db = db;

	// calculate each row...
	_b = m_b1;
	for (int y = 0; y < m_iMandHeight; ++y)
	{
		// calculate each pixel...
		_a = m_a1;
		for (int x = 0; x < m_iMandWidth; x += 1)
		{
			// calculate each iteration...
			double c = 0.0f;
			double d = 0.0f;
			double c2, d2, cd2;
			double mag;
			int i;
			for (i = 0; i < get_MaxIterationsMand(); ++i)
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

			// convert the iteration count into a color and store it in the bitmap bits
			l_ppvBits[iIndex++] = (i == get_MaxIterationsMand()) ? 0 : m_PaletteDefault.get_Color(i);
		}
		_b += db;
	}
	// stop the performance counter
	QueryPerformanceCounter(&tStop);
	m_tMandelbrotDuration[iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;
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
void CJuliasmApp::CalculateMandPointsSSE(void)
{
	int tmp = get_MaxIterationsMand();
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

			dec ax
			jnz iterate
			jmp done

		jmp save_orbit_resume

		done :
		// adjust the counts by 4
		divps xmm6, xmm7
	}
}
//
// Calculates a mandelbrot set using SSE instructions.
// 
// This functions handles the iteration over pixels,
//	while the calculation is actually performed bu the
//	CalculateMandPointsSSE function.
//
// Uses:
//	put_CalcPlatformMand();
//	m_iMandelbrotThreadCount;

//unsigned __stdcall CalculateMandSSE( void* pArguments )
DWORD WINAPI CJuliasmApp::CalculateMandSSE(void* pArguments)
{
	// get Application and thread inde information
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	pApp->put_CalcPlatformMand(CalcPlatform::SSE);
	int iThreadIndex = pThreadInfo->iThreadIndex;

	// determine how much of the image this thread will calculate
	double fThreadHeight = (pApp->m_b2 - pApp->m_b1) / pApp->m_iMandelbrotThreadCount;

	// declare calculation variables used by SSE
	__declspec(align(16)) float b_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float maximum_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float iterations_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float da_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float db_sse[POINTS_CONCURRENT_SSE];
	__declspec(align(16)) float a1_sse[POINTS_CONCURRENT_SSE];

	double
		_a = pApp->m_a1,
		_b = pApp->m_b1 + fThreadHeight * iThreadIndex,
		da = (pApp->m_a2 - pApp->m_a1) / pApp->m_iMandWidth,
		db = (pApp->m_b2 - pApp->m_b1) / pApp->m_iMandHeight;

	pApp->m_da = da;
	pApp->m_db = db;

	// initialize SSE variables
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

	// initialzie the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);

	// load the imaginary component
	__asm movaps xmm4, oword ptr[b_sse]

	// load the maximum vector size
	__asm movaps xmm7, oword ptr[maximum_sse]

	// get the bitmap bits
	unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpMandelbrot.get_bmpBits();

	int pixelHeight = pApp->m_iMandHeight / pApp->m_iMandelbrotThreadCount;
	int starty = iThreadIndex * pixelHeight;
	int endy = starty + pixelHeight;

	unsigned int iIndex = 0 + starty * pApp->m_iMandWidth;

	for (int y = starty; y < endy; ++y)
	{
		// load the real component
		__asm movaps xmm3, a1_sse

		for (int x = 0; x < pApp->m_iMandWidth; x += POINTS_CONCURRENT_SSE)
		{

			//			memset(orbit_c_sse, 0, sizeof(orbit_c_sse));
			pApp->CalculateMandPointsSSE();

			__asm movaps oword ptr[iterations_sse], xmm6

			// normalize the iteration values between 0 and 255
			l_ppvBits[iIndex++] = (((int)iterations_sse[0]) == pApp->get_MaxIterationsMand()) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[0]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[1]) == pApp->get_MaxIterationsMand()) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[1]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[2]) == pApp->get_MaxIterationsMand()) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[2]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[3]) == pApp->get_MaxIterationsMand()) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[3]);

			// update the real components
			__asm {
				addps xmm3, oword ptr[da_sse]
			}
		}
		// update the imaginary components
		__asm addps xmm4, oword ptr[db_sse]
	}
	QueryPerformanceCounter(&tStop);
	pApp->m_tMandelbrotDuration[iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	HWND hWnd = get_hwnd();
	PostMessage(hWnd, WM_COMMAND, IDM_THREADCOMPLETE, 0);

	return 0;
}
DWORD WINAPI CJuliasmApp::CalculateMandSSE2(void* pArguments)
{
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	pApp->put_CalcPlatformMand(CalcPlatform::SSE2);


	int iThreadIndex = pThreadInfo->iThreadIndex;

	double fThreadHeight = (pApp->m_b2 - pApp->m_b1) / pApp->m_iMandelbrotThreadCount;

	__declspec(align(16)) double b_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double maximum_sse2[POINTS_CONCURRENT_SSE2] = { 4.0f, 4.0f };
	__declspec(align(16)) double iterations_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double da_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double db_sse2[POINTS_CONCURRENT_SSE2];
	__declspec(align(16)) double a1_sse2[POINTS_CONCURRENT_SSE2];

	double
		_a = pApp->m_a1,
		_b = pApp->m_b1 + fThreadHeight * iThreadIndex,
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

	int pixelHeight = pApp->m_iMandHeight / pApp->m_iMandelbrotThreadCount;
	int starty = iThreadIndex * pixelHeight;
	int endy = starty + pixelHeight;

	unsigned int iIndex = 0 + starty * pApp->m_iMandWidth;

	for (int y = starty; y < endy; ++y)
	{
		// load the real component
		__asm movapd xmm3, a1_sse2

		for (int x = 0; x < pApp->m_iMandWidth; x += POINTS_CONCURRENT_SSE2)
		{

			//			memset(orbit_c_sse, 0, sizeof(orbit_c_sse));
			pApp->CalculateMandPointsSSE2();

			__asm movapd oword ptr[iterations_sse2], xmm6


			// normalize the iteration values between 0 and 255
			l_ppvBits[iIndex++] = (((int)iterations_sse2[0]) == pApp->get_MaxIterationsMand()) ? 0 : pApp->m_PaletteDefault.get_Color(((int)iterations_sse2[0]));
			l_ppvBits[iIndex++] = (((int)iterations_sse2[1]) == pApp->get_MaxIterationsMand()) ? 0 : pApp->m_PaletteDefault.get_Color(((int)iterations_sse2[1]));

			// update the real components
			__asm {
				addpd xmm3, oword ptr[da_sse2]
			}
		}
		// update the imaginary components
		__asm addpd xmm4, oword ptr[db_sse2]
	}
	QueryPerformanceCounter(&tStop);
	pApp->m_tMandelbrotDuration[iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	PostMessage(pApp->get_hWnd(), WM_COMMAND, IDM_THREADCOMPLETE, 0);

	return 0;
}


void CJuliasmApp::CalculateMandPointsSSE2(void)
{
	int iMaxIterations = this->get_MaxIterationsMand();

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



//
// calculates the Julia set image using AVX
//
DWORD WINAPI CJuliasmApp::CalculateJuliaAVX(void* pArguments)
{
	// get the Application pointer and the thread number
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	int iThread = pThreadInfo->iThreadIndex;

	// This thread continues to run for the life of the program.
	// It will return to the 'restart:' location after calculation
	// is complete and await the next calculation initiation.
	//

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

	//
	// start the calculation
	//

	// RAP why is this being done?
	if (iThread == 0)
	{
		pApp->m_iJMaxIter = 0;
	}

	// setup the performance counter
	LARGE_INTEGER iTicksStart;
	LARGE_INTEGER iTicksEnd;

	QueryPerformanceCounter(&iTicksStart);


	// get the bitmap for drawing the image
	unsigned int* l_ppvBitsJulia = (unsigned int*)pApp->m_bmpJulia.get_bmpBits();


	volatile int heightPixels = pApp->m_iJuliaHeight / pApp->m_iJuliaThreadCount;
	volatile int x, y;
	volatile __declspec(align(32)) float a, b, start_df;
	volatile float fdc = (pApp->m_jc2_sse - pApp->m_jc1_sse) / pApp->m_iJuliaWidth;
	volatile float fdd = (pApp->m_jd2_sse - pApp->m_jd1_sse) / pApp->m_iJuliaHeight;
	volatile float heightNumeric = (pApp->m_jd2_sse - pApp->m_jd1_sse) / pApp->m_iJuliaThreadCount;

	volatile 	int startY = iThread * heightPixels;
	volatile int stopY = startY + heightPixels;
	int i;


	//
	// initialize per-pixel variables
	//
	__declspec(align(32)) float a_avx[POINTS_CONCURRENT_AVX32];
	__declspec(align(32)) float b_avx[POINTS_CONCURRENT_AVX32];
	__declspec(align(32)) float dc_avx[POINTS_CONCURRENT_AVX32];
	__declspec(align(32)) float ic_avx[POINTS_CONCURRENT_AVX32];

	__declspec(align(32)) float maximum_avx[POINTS_CONCURRENT_AVX32];
	__declspec(align(32)) float iterations_avx[POINTS_CONCURRENT_AVX32];
	__declspec(align(32)) float zero_avx[POINTS_CONCURRENT_AVX32];
	__declspec(align(32)) float increment_avx[POINTS_CONCURRENT_AVX32];

	for (i = 0; i < POINTS_CONCURRENT_AVX32; ++i)
	{
		a_avx[i] = pApp->m_ja_sse;
		b_avx[i] = pApp->m_jb_sse;
		dc_avx[i] = 8.0f * fdc;
		ic_avx[i] = pApp->m_jc1_sse + (i * 1.0f) * fdc;
		maximum_avx[i] = 4.0f;
		zero_avx[i] = 0.0f;
	}
	
	a = pApp->m_ja_sse;
	b = pApp->m_jb_sse;


	start_df = pApp->m_jd1_sse + heightNumeric * iThread;

	// load the maximum magnitude into xmm7
	__m256 max = _mm256_load_ps(maximum_avx);

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
				increment_avx[ii] = 4.0f;
			}

			for (i = 0; i < pApp->get_MaxIterationsJulia(); ++i)
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
				if (iterations_avx[j] >= pApp->get_MaxIterationsJulia())
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
	
	// stop the performance counter
	QueryPerformanceCounter(&iTicksEnd);

	// tell the application that the calculation is complete
	PostMessage(get_hwnd(), WM_COMMAND, IDM_THREADCOMPLETEJULIA, 0);

	// prepare for the next calculation request
	goto restart;

	// end the function.  we should never get here
	return 0;
}
// #pragma optimize("", on )





