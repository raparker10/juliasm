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
// calculates the Julia set image using AVX32
//
//NOTE: This thread is started upon program initialization and waits until
// it receives a message to begin working.  In practice, this saves
// about 1ms per execution and is really not worth the overhead
// of having a bunch of waiting threads to manage... so, this needs to be
// converted to an approach that creates the threads in response
// to a drawing request.
//
DWORD WINAPI CJuliasmApp::CalculateJuliaAVX32(void* pArguments)
{
	// 
	// Setup the function by getting the thread index and 
	//	pointer to the parent Application object.
	//	These values will be used to determine which portion of the
	//	Julia set to compute and avoid a lot of thread synchronization.
	//
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	int iThreadIndex = pThreadInfo->iThreadIndex;

	// Indicate that another working thread has started
	InterlockedIncrement(&pApp->m_iJuliaReady[iThreadIndex]);

	//
	// start the calculation
	//


	// Record the start time to later determine thread working duratioj
	LARGE_INTEGER iTicksStart;
	LARGE_INTEGER iTicksEnd;
	QueryPerformanceCounter(&iTicksStart);


	// get the bitmap for drawing the image
	unsigned int* l_ppvBitsJulia = (unsigned int*)pApp->m_bmpJulia.get_bmpBits();

	//
	// make a local copy of variables from the parent (Application) object
	// to simplify code
	volatile __declspec(align(32)) int 
			x, y,										// current x, and y locations
			num_threadsi = pApp->m_iJuliaThreadCount,	// the number of worker threads (needed to chop up the problem per thread)
			width_pixi = pApp->m_iJuliaWidth,			// the screen width in pixels
			height_pixi = pApp->m_iJuliaHeight;			// the screen height in pixels


	volatile __declspec(align(32)) float 
		c1f = pApp->m_jc1_sse,		// the REAL numerical value assigned to the LEFT-most pixel on the screen
		c2f = pApp->m_jc2_sse,		//  the REAL numerical value assigned to the RIGHT-most pixel on the screen
		d1f = pApp->m_jd1_sse,		//  the IMAGINARY numerical value assigned to the TOP-most pixel on the screen
		d2f = pApp->m_jd2_sse,		//  the IMAGINARY numerical value assigned to the BOTTOM-most pixel on the screen
		
		af = pApp->m_ja_sse,		// the REAL portion of the constant parameter that defines this julia set (a pixel from the Mandelbrot set)
		bf = pApp->m_jb_sse,		// the IMAGINARY portion of the constant parameter that defines this julia set (a pixel from the Mandelbrot set)

		height_num_threadf = (d2f - d1f) / num_threadsi,	// the numerical heigt of the pportion of the calculation handled by this thread

		dcf = (c2f - c1f) / width_pixi,						// the REAL-valued difference between each pixel
		ddf = (d2f - d1f) / height_pixi,					// the IMAGINARY-valued difference between each pixel
		start_df = d1f + iThreadIndex * height_num_threadf;	// the starting numerical location of the portion of the image calculated by this thread

	volatile __declspec(align(32)) int
		max_i = pApp->get_MaxIterationsJulia(),				// get the number of iterations from the parent Application
		height_pix_thread = height_pixi / num_threadsi,		// the height in pixels of the portion calculated by this thread
		start_yi = iThreadIndex * height_pix_thread,		// the top pixel calculated by this thread
		stop_yi = start_yi + height_pix_thread;				// the bottom pixel calculated by this thread

	//
	// initialize per-pixel variables
	//
	__declspec(align(32)) float iterations_avx[POINTS_CONCURRENT_AVX32];

	// initialize constants
	__declspec(align(32)) const float MAX_JULIA_MAGNITUDE = 4.0f;
	__declspec(align(32)) const float PIXEL_STRIDE_AVX32 = dcf * POINTS_CONCURRENT_AVX32;
	__declspec(align(32)) const float PIXEL_INCREMENT_AVX32[POINTS_CONCURRENT_AVX32] = {
		c1f + 0 * dcf,
		c1f + 1 * dcf,
		c1f + 2 * dcf,
		c1f + 3 * dcf,
		c1f + 4 * dcf,
		c1f + 5 * dcf,
		c1f + 6 * dcf,
		c1f + 7 * dcf,
	};


	__m256 max = _mm256_broadcast_ss(&MAX_JULIA_MAGNITUDE);					// get the maximum allowable magnitude (4.0)
	__m256 delta_c = _mm256_broadcast_ss(&PIXEL_STRIDE_AVX32);				// save the c increment
	__m256 initial_c = _mm256_load_ps((const float*)PIXEL_INCREMENT_AVX32);	// load the initial C values
	__m256 curr_a = _mm256_broadcast_ss((const float*)&af);					// broadcast the Julia set real component to an AVX variable
	__m256 curr_b = _mm256_broadcast_ss((const float*)&bf);					// broadcast the Julia set imaginary component into an AVX variable


	for (y = start_yi; y < stop_yi; ++y)
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

			for (int i = 0; i < max_i; ++i)
			{
				__m256 cd2 = _mm256_mul_ps(c, d);			// calculate 2 * c * d
				cd2 = _mm256_add_ps(cd2, cd2);
				__m256 c2 = _mm256_mul_ps(c, c);			// calculate c^2 + d^2
				__m256 d2 = _mm256_mul_ps(d, d);
				__m256 mag = _mm256_add_ps(c2, d2);
				__m256 cmp = _mm256_cmp_ps(mag, max, _CMP_LT_OQ);	// compare the magnitude to the max allowable magnitude (2.0f)
				unsigned int test = _mm256_movemask_ps(cmp);
				__m256 increment = _mm256_and_ps(cmp, max);			// increment the iterations
				iterations = _mm256_add_ps(increment, iterations);

				if (test == 0)	// quit if there are no pixels li=eft to increment
					break;

				c = _mm256_sub_ps(c2, d2);					// generate the new c: c = c^2 - d^2 + a
				c = _mm256_add_ps(c, curr_a);
				d = _mm256_add_ps(cd2, curr_b);				// generate the new d: 2 * c * d + b
			} // next iteration

			// convert the iteration count to a color index and save to memory
			iterations = _mm256_div_ps(iterations, max);
			_mm256_store_ps(iterations_avx, iterations);

			for (int j = 0; j < POINTS_CONCURRENT_AVX32; ++j)
			{
				((unsigned int*)l_ppvBitsJulia)[y * pApp->m_iJuliaWidth + x + j] = (iterations_avx[j] >= max_i) ? 0 : pApp->m_PaletteDefault.get_Color(iterations_avx[j]);
			}

			// generate the next set of 'c' values by adding the c_increment to the current c
			c = _mm256_add_ps(start_c, delta_c);
			start_c = c;
			d = start_d;
		} // next x
		start_df = start_df + ddf;

	} // next y
	
	// stop the performance counter
	QueryPerformanceCounter(&iTicksEnd);
	pApp->m_tJuliaThreadDuration[iThreadIndex].QuadPart = iTicksEnd.QuadPart - iTicksStart.QuadPart;

	// tell the application that the calculation is complete
	PostMessage(pApp->get_hWnd(), WM_COMMAND, IDM_THREADCOMPLETEJULIAAVX32, 0);

	// end the function.  we should never get here
	return 0;
}
// #pragma optimize("", on )



//
// calculates the Julia set image using AVX64
//
DWORD WINAPI CJuliasmApp::CalculateJuliaAVX64(void* pArguments)
{
	// 
	// Setup the function by getting the thread index and 
	//	pointer to the parent Application object.
	//	These values will be used to determine which portion of the
	//	Julia set to compute and avoid a lot of thread synchronization.
	//
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	int iThreadIndex = pThreadInfo->iThreadIndex;

	// Indicate that another working thread has started
	InterlockedIncrement(&pApp->m_iJuliaReady[iThreadIndex]);


	//
	// start the calculation
	//


	// Record the start time to later determine thread working duratioj
	LARGE_INTEGER iTicksStart;
	LARGE_INTEGER iTicksEnd;
	QueryPerformanceCounter(&iTicksStart);


	// get the bitmap for drawing the image
	unsigned int* l_ppvBitsJulia = (unsigned int*)pApp->m_bmpJulia.get_bmpBits();

	//
	// make a local copy of variables from the parent (Application) object
	// to simplify code
	volatile __declspec(align(32)) int 
			x, y,										// current x, and y locations
			num_threadsi = pApp->m_iJuliaThreadCount,	// the number of worker threads (needed to chop up the problem per thread)
			width_pixi = pApp->m_iJuliaWidth,			// the screen width in pixels
			height_pixi = pApp->m_iJuliaHeight;			// the screen height in pixels


	volatile __declspec(align(32)) double 
		c1f = pApp->m_jc1_sse,		// the REAL numerical value assigned to the LEFT-most pixel on the screen
		c2f = pApp->m_jc2_sse,		//  the REAL numerical value assigned to the RIGHT-most pixel on the screen
		d1f = pApp->m_jd1_sse,		//  the IMAGINARY numerical value assigned to the TOP-most pixel on the screen
		d2f = pApp->m_jd2_sse,		//  the IMAGINARY numerical value assigned to the BOTTOM-most pixel on the screen
		
		af = pApp->m_ja_sse,		// the REAL portion of the constant parameter that defines this julia set (a pixel from the Mandelbrot set)
		bf = pApp->m_jb_sse,		// the IMAGINARY portion of the constant parameter that defines this julia set (a pixel from the Mandelbrot set)

		height_num_threadf = (d2f - d1f) / num_threadsi,	// the numerical heigt of the pportion of the calculation handled by this thread

		dcf = (c2f - c1f) / width_pixi,						// the REAL-valued difference between each pixel
		ddf = (d2f - d1f) / height_pixi,					// the IMAGINARY-valued difference between each pixel
		start_df = d1f + iThreadIndex * height_num_threadf;	// the starting numerical location of the portion of the image calculated by this thread

	volatile __declspec(align(32)) int
		max_i = pApp->get_MaxIterationsJulia(),				// get the number of iterations from the parent Application
		height_pix_thread = height_pixi / num_threadsi,		// the height in pixels of the portion calculated by this thread
		start_yi = iThreadIndex * height_pix_thread,		// the top pixel calculated by this thread
		stop_yi = start_yi + height_pix_thread;				// the bottom pixel calculated by this thread

	//
	// initialize per-pixel variables
	//
	__declspec(align(32)) double iterations_avx[POINTS_CONCURRENT_AVX64];

	// initialize constants
	__declspec(align(32)) const double MAX_JULIA_MAGNITUDE = 4.0f;
	__declspec(align(32)) const double PIXEL_STRIDE_AVX64 = dcf * POINTS_CONCURRENT_AVX64;
	__declspec(align(32)) const double PIXEL_INCREMENT_AVX64[POINTS_CONCURRENT_AVX64] = {
		c1f + 0 * dcf,
		c1f + 1 * dcf,
		c1f + 2 * dcf,
		c1f + 3 * dcf,
	};


	__m256d max = _mm256_broadcast_sd(&MAX_JULIA_MAGNITUDE);					// get the maximum allowable magnitude (4.0)
	__m256d delta_c = _mm256_broadcast_sd(&PIXEL_STRIDE_AVX64);				// save the c increment
	__m256d initial_c = _mm256_load_pd((const double*)PIXEL_INCREMENT_AVX64);	// load the initial C values
	__m256d curr_a = _mm256_broadcast_sd((const double*)&af);					// broadcast the Julia set real component to an AVX variable
	__m256d curr_b = _mm256_broadcast_sd((const double*)&bf);					// broadcast the Julia set imaginary component into an AVX variable


	for (y = start_yi; y < stop_yi; ++y)
	{
		// load the current d across all _avx values
		__m256d d = _mm256_broadcast_sd((const double*)&start_df);
		__m256d start_d = d;

		// load the initial value of c, and then store it in the current c array
		__m256d start_c = initial_c; // _mm256_load_pd((const double*)ic_avx);
		__m256d c = initial_c;

		for (x = 0; x < pApp->m_iJuliaWidth; x += POINTS_CONCURRENT_AVX64)
		{
			// initilize c, d, and the iteration count
			__m256d iterations = _mm256_setzero_pd();

			for (int i = 0; i < max_i; ++i)
			{
				__m256d cd2 = _mm256_mul_pd(c, d);			// calculate 2 * c * d
				cd2 = _mm256_add_pd(cd2, cd2);
				__m256d c2 = _mm256_mul_pd(c, c);			// calculate c^2 + d^2
				__m256d d2 = _mm256_mul_pd(d, d);
				__m256d mag = _mm256_add_pd(c2, d2);
				__m256d cmp = _mm256_cmp_pd(mag, max, _CMP_LT_OQ);	// compare the magnitude to the max allowable magnitude (2.0f)
				unsigned int test = _mm256_movemask_pd(cmp);
				__m256d increment = _mm256_and_pd(cmp, max);			// increment the iterations
				iterations = _mm256_add_pd(increment, iterations);

				if (test == 0)	// quit if there are no pixels li=eft to increment
					break;

				c = _mm256_sub_pd(c2, d2);					// generate the new c: c = c^2 - d^2 + a
				c = _mm256_add_pd(c, curr_a);
				d = _mm256_add_pd(cd2, curr_b);				// generate the new d: 2 * c * d + b
			} // next iteration

			// convert the iteration count to a color index and save to memory
			iterations = _mm256_div_pd(iterations, max);
			_mm256_store_pd(iterations_avx, iterations);

			for (int j = 0; j < POINTS_CONCURRENT_AVX64; ++j)
			{
				((unsigned int*)l_ppvBitsJulia)[y * pApp->m_iJuliaWidth + x + j] = (iterations_avx[j] >= max_i) ? 0 : pApp->m_PaletteDefault.get_Color(iterations_avx[j]);
			}

			// generate the next set of 'c' values by adding the c_increment to the current c
			c = _mm256_add_pd(start_c, delta_c);
			start_c = c;
			d = start_d;
		} // next x
		start_df = start_df + ddf;

	} // next y
	
	// stop the performance counter
	QueryPerformanceCounter(&iTicksEnd);
	pApp->m_tJuliaThreadDuration[iThreadIndex].QuadPart = iTicksEnd.QuadPart - iTicksStart.QuadPart;

	// tell the application that the calculation is complete
	PostMessage(pApp->get_hWnd(), WM_COMMAND, IDM_THREADCOMPLETEJULIAAVX64, 0);

	// end the function.  we should never get here
	return 0;
}
// #pragma optimize("", on )
