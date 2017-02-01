#include "stdafx.h"

//
// Calculate a Julia set set using compiled C++, assuming x87 compilation
//
// Uses:
//	m_iJuliaHeight, m_iJuliaWidth
//	m_bmpJulia
//	get_MaxIterationsJulia()
//	m_paletteDefault
//	get_CalcPlatformJulia()
//
// implements Z(n+1)=Z(n)^2+C where...
//		Z is the current pixel location, represented by the variables 'c', and 'd'
//		C is the constant that represents a specific Julia set and does not vary and is contained in 'a', and 'b'  (m_ja_sse, and m_jb_sse in CJuliasmApp)
//
DWORD WINAPI CJuliasmApp::CalculateJuliaX87(void* pArguments)
{
	// get Application and thread inde information
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	pApp->put_CalcPlatformJulia(CalcPlatform::x87);
	int iThreadIndex = pThreadInfo->iThreadIndex;


	// initialzie the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);


	// determine how much of the image this thread will calculate
	double fThreadHeight = (pApp->m_jc2_sse - pApp->m_jc1_sse) / pApp->m_iJuliaThreadCount;

	double
		a = pApp->m_ja_sse,
		b = pApp->m_jb_sse,
		start_c = pApp->m_jc1_sse,
		start_d = pApp->m_jd1_sse + fThreadHeight * iThreadIndex,
		dc = (pApp->m_jc2_sse - pApp->m_jc1_sse) /pApp->m_iJuliaWidth,
		dd = (pApp->m_jd2_sse - pApp->m_jd1_sse) /pApp->m_iJuliaHeight;

	// get the bitmap bits
	unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpJulia.get_bmpBits();

	int pixelHeight = pApp->m_iJuliaHeight / pApp->m_iJuliaThreadCount;
	int starty = iThreadIndex * pixelHeight;
	int endy = starty + pixelHeight;

	unsigned int iIndex = 0 + starty * pApp->m_iJuliaWidth;

	// calculate each row...
	double _d = start_d;
	for (int y = starty; y < endy; ++y)
	{
		// calculate each pixel...
		double _c = start_c;

		for (int x = 0; x < pApp->m_iJuliaWidth; x += 1)
		{
			// calculate each iteration...
			double c = _c;
			double d = _d;
			double c2, d2, cd2;
			double mag;
			int i;
			for (i = 0; i < pApp->get_MaxIterationsJulia(); ++i)
			{
				cd2 = 2 * c * d;
				c2 = c * c;
				d2 = d * d;
				mag = c2 + d2;
				c = c2 - d2 + a;
				d = cd2 + b;
				if (mag > 4.0)
					break;
			}
			_c += dc;

			// convert the iteration count into a color and store it in the bitmap bits
			l_ppvBits[iIndex++] = (i == pApp->get_MaxIterationsJulia()) ? 0 : pApp->m_PaletteDefault.get_Color(i);
		}
		_d += dd;
	}
	// stop the performance counter
	QueryPerformanceCounter(&tStop);
	pApp->m_tJuliaThreadDuration[iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	// tell the application that the thread is complete
	PostMessage(pApp->get_hWnd(), WM_COMMAND, IDM_THREADCOMPLETEJULIA87, 0);

	return 0;
}



//
// calculates the Julia set image using AVX
//
DWORD WINAPI CJuliasmApp::CalculateJuliaAVX(void* pArguments)
{
	// get the Application pointer and the thread number
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	int iThreadIndex = pThreadInfo->iThreadIndex;

	// increase the number of working threads
	InterlockedIncrement(&pApp->m_iJuliaReady[iThreadIndex]);

	// This thread continues to run for the life of the program.
	// It will return to the 'restart:' location after calculation
	// is complete and await the next calculation initiation.
	//

	//
	// wait for a message to calculate the picture
	//
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE); // force the thread to create a message queue

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
	if (iThreadIndex == 0)
	{
		pApp->m_iJMaxIter = 0;
	}

	// Record the start time to later determine thread working duratioj
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

	volatile 	int startY = iThreadIndex * heightPixels;
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


	start_df = pApp->m_jd1_sse + heightNumeric * iThreadIndex;

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
				((unsigned int*)l_ppvBitsJulia)[y * pApp->m_iJuliaWidth + x + j] = (iterations_avx[j] == pApp->get_MaxIterationsJulia()) ? 0 : pApp->m_PaletteDefault.get_Color(iterations_avx[j]);
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
	pApp->m_tJuliaThreadDuration[iThreadIndex].QuadPart = iTicksEnd.QuadPart - iTicksStart.QuadPart;

	// tell the application that the calculation is complete
	PostMessage(pApp->get_hWnd(), WM_COMMAND, IDM_THREADCOMPLETEJULIA, 0);

	// prepare for the next calculation request
	goto restart;

	// end the function.  we should never get here
	return 0;
}
// #pragma optimize("", on )



