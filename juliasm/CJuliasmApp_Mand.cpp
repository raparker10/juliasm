#include "stdafx.h"

//
// Calculate a Mandelbrot set using compiled C++, assuming x87 compilation
//
// Uses:
//	m_tMandelbrotStart, m_tMandelbrotStop, m_tMandelbrotTotal for timing
//	m_iMandHeight, m_iMandWidth
//	m_bmpMandelbrot
//	get_MaxIterationsMand()
//	m_paletteDefault
//	
DWORD WINAPI CJuliasmApp::CalculateMandX87(void* pArguments)
{
	// get Application and thread inde information
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	int iThreadIndex = pThreadInfo->iThreadIndex;


	// initialzie the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);


	// determine how much of the image this thread will calculate
	double fThreadHeight = (pApp->m_b2 - pApp->m_b1) / pApp->m_iCalcThreadCount[FractalType::Mand];

	double
		_a = pApp->m_a1,
		_b = pApp->m_b1 + fThreadHeight * iThreadIndex,
		da = (pApp->m_a2 - pApp->m_a1) / pApp->get_MandWidth(),
		db = (pApp->m_b2 - pApp->m_b1) / pApp->get_MandHeight();

	// get the bitmap bits
	unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpFractal[FractalType::Mand].get_bmpBits();

	int pixelHeight = pApp->get_MandHeight() / pApp->m_iCalcThreadCount[FractalType::Mand];
	int starty = iThreadIndex * pixelHeight;
	int endy = starty + pixelHeight;

	unsigned int iIndex = 0 + starty * pApp->get_MandWidth();

	// calculate each row...
	for (int y = starty; y < endy; ++y)
	{
		// calculate each pixel...
		_a = pApp->m_a1;
		for (int x = 0; x < pApp->get_MandWidth(); x += 1)
		{
			// calculate each iteration...
			double c = 0.0f;
			double d = 0.0f;
			double c2, d2, cd2;
			double mag;
			int i;
			float fColor = 0;
			for (i = 0; i < pApp->get_MaxIterationsMand(); ++i)
			{
				c2 = c * c;
				d2 = d * d;
				mag = c2 + d2;
				if (mag > 40.0)
				{
					double _mc = log(sqrt(mag));
					double _cc;
					if (_mc > 0)
						_cc = log(_mc);
					else
						_cc = log(-_mc);
					fColor = i + 1.0 - _cc / log(2.0);
					break;	
				}
				cd2 = 2 * c * d;
				c = c2 - d2 + _a;
				d = cd2 + _b;
			}
			_a += da;

			// convert the iteration count into a color and store it in the bitmap bits
//			l_ppvBits[iIndex++] = (i == pApp->get_MaxIterationsMand()) ? 0 : pApp->m_PaletteDefault.get_Color(i);
			l_ppvBits[iIndex++] = (i == pApp->get_MaxIterationsMand()) ? 0 : pApp->m_PaletteDefault.get_Color(fColor);
		}
		_b += db;
	}
	// stop the performance counter
	QueryPerformanceCounter(&tStop);
	//	m_tMandelbrotProcessDurationTotal.QuadPart 
	//		= m_tMandelbrotThreadDurationTotal.QuadPart 
	pApp->m_tThreadDuration[FractalType::Mand][iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	// tell the application that the thread is complete
	PostMessage(
		pApp->get_hWnd(), 
		pThreadInfo->iThreadCompleteMessage, 
		pThreadInfo->iThreadCompleteWParam, 
		pThreadInfo->iThreadCompleteLParam);

	return 0;
}


//
// Calculates a mandelbrot set using SSE instructions.
// 

DWORD WINAPI CJuliasmApp::CalculateMandSSE(void* pArguments)
{
	// get Application and thread inde information
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	if (pApp == NULL)
	{
		return 1; // return error
	}
	int iThreadIndex = pThreadInfo->iThreadIndex;
	__declspec(align(16))int max_i = pApp->get_MaxIterationsMand();

	// initialzie the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);

	// determine how much of the image this thread will calculate
	float fThreadHeight = (float)(pApp->m_b2 - pApp->m_b1) / pApp->m_iCalcThreadCount[FractalType::Mand];

	// declare calculation variables used by SSE
	__declspec(align(16)) float iterations_sse[POINTS_CONCURRENT_SSE];

	__declspec(align(16))float
		af = (float)pApp->m_a1,
		bf = (float)pApp->m_b1 + fThreadHeight * iThreadIndex,
		daf = (float)(pApp->m_a2 - pApp->m_a1) / pApp->get_MandWidth(),
		dbf = (float)(pApp->m_b2 - pApp->m_b1) / pApp->get_MandHeight();


	// get the bitmap bits
	__declspec(align(16)) unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpFractal[FractalType::Mand].get_bmpBits();

	if (l_ppvBits == NULL)
	{
		return 1; // return error
	}

	// RAP delete the following check
	if (l_ppvBits == NULL)
		MessageBox(NULL, "l_ppvBits is null", "Error", MB_ICONSTOP | MB_OK);

	int pixelHeight = pApp->get_MandHeight() / pApp->m_iCalcThreadCount[FractalType::Mand];
	int starty = iThreadIndex * pixelHeight;
	int endy = starty + pixelHeight;
	unsigned int iIndex = 0 + starty * pApp->get_MandWidth();

	// initialize SSE variables for loading into SSE data types
	float da_total = daf * POINTS_CONCURRENT_SSE;
	const __declspec(align(16)) float a1_sse[POINTS_CONCURRENT_SSE] = {af + daf * 0, af + daf * 1, af + daf * 2, af + daf * 3,};
	const __declspec(align(16)) float da_sse[POINTS_CONCURRENT_SSE] = {da_total, da_total, da_total, da_total,};
	const __declspec(align(16)) float db_sse[POINTS_CONCURRENT_SSE] = {dbf, dbf, dbf, dbf,};
	const __declspec(align(16)) float b_sse[POINTS_CONCURRENT_SSE] = {bf, bf, bf, bf,};
	const __declspec(align(16)) float max_sse[POINTS_CONCURRENT_SSE] = {MAXIMUM_MAND_MAGNITUDE, MAXIMUM_MAND_MAGNITUDE, MAXIMUM_MAND_MAGNITUDE, MAXIMUM_MAND_MAGNITUDE,};

	// load the real and imaginary pixel deltas
	__m128 da = _mm_load_ps(da_sse); // broadcast_ss(&da_total); // load_ps(da_sse);
	__m128 db = _mm_load_ps(db_sse); // _mm_load_ps(db_sse);

	// load the imaginary component
	__m128 b = _mm_load_ps(b_sse);

	// load the maximum vector magnitude
	__m128 max = _mm_load_ps(max_sse); //load_ps(maximum_sse);

	for (int y = starty; y < endy; ++y)
	{
		// load the real component
		__m128 a = _mm_load_ps(a1_sse);

		for (int x = 0; x < pApp->get_MandWidth(); x += POINTS_CONCURRENT_SSE)
		{

			// place 0 in c, d, and iteration count
			__m128 c = _mm_setzero_ps();
			__m128 d = _mm_setzero_ps();
			__m128 iterations = _mm_setzero_ps();

			//
			// start the main calculation loop
			//

			for (int i = 0; i < max_i; ++i)
			{
				// calculate c^2, d^2, and c^2 + d^2
				__m128 c_squ = _mm_mul_ps(c, c);
				__m128 d_squ = _mm_mul_ps(d, d);
				__m128 mag = _mm_add_ps(c_squ, d_squ);

				// bail out if all values are larger than the maximum magnitude (4.0f)
				__m128 cmp = _mm_cmplt_ps(mag, max);	// compare each component to the maximum value
				if (_mm_movemask_ps(cmp) == 0)
				{
					break;
				}

				// increment the iteration count for the pixels that are still less than the maxoimum magnitude
				__m128 tmp = _mm_and_ps(cmp, max);
				iterations = _mm_add_ps(iterations, tmp);

				// calculate 2 * c * d + b
				__m128 cd = _mm_mul_ps(c, d);
				__m128 cd2 = _mm_add_ps(cd, cd);
				d = _mm_add_ps(cd2, b);

				// calculate c^2 - d^2 + a
				__m128 diff = _mm_sub_ps(c_squ, d_squ);
				c = _mm_add_ps(diff, a);

				// calculate the magnitude of the n^2 new values and store them in XMM5
				__m128 result = _mm_add_ps(c_squ, d_squ);

			}
			// adjust the counts by 4
			iterations = _mm_div_ps(iterations, max);
			_mm_store_ps(iterations_sse, iterations);

			// RAP delete the following check
			if (l_ppvBits == NULL)
				MessageBox(NULL, "l_ppvBits is null", "Error", MB_ICONSTOP | MB_OK);
			// normalize the iteration values between 0 and 255
			l_ppvBits[iIndex++] = (((int)iterations_sse[0]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[0]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[1]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[1]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[2]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[2]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[3]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[3]);

			// update the real components
			a = _mm_add_ps(a, da);
		}
		// update the imaginary components
		b = _mm_add_ps(b, db);
	}

	// track the start time and calculate the thread working duration
	QueryPerformanceCounter(&tStop);
	pApp->m_tThreadDuration[FractalType::Mand][iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	// tell the application that the thread is complete
	// tell the application that the thread is complete
	PostMessage(
		pApp->get_hWnd(), 
		pThreadInfo->iThreadCompleteMessage, 
		pThreadInfo->iThreadCompleteWParam, 
		pThreadInfo->iThreadCompleteLParam);

	return 0;
}

//
// Calculates a mandelbrot set using SSE2 instructions.
// 

//unsigned __stdcall CalculateMandSSE( void* pArguments )
DWORD WINAPI CJuliasmApp::CalculateMandSSE2(void* pArguments)
{
	// get Application and thread inde information
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	int iThreadIndex = pThreadInfo->iThreadIndex;
	__declspec(align(16))int max_i = pApp->get_MaxIterationsMand();

	// initialzie the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);

	// determine how much of the image this thread will calculate
	double fThreadHeight = (pApp->m_b2 - pApp->m_b1) / pApp->m_iCalcThreadCount[FractalType::Mand];

	// declare calculation variables used by SSE
	__declspec(align(16)) double iterations_sse[POINTS_CONCURRENT_SSE];

	__declspec(align(16))double
		af = pApp->m_a1,
		bf = pApp->m_b1 + fThreadHeight * iThreadIndex,
		daf = (pApp->m_a2 - pApp->m_a1) / pApp->get_MandWidth(),
		dbf = (pApp->m_b2 - pApp->m_b1) / pApp->get_MandHeight();


	// get the bitmap bits
	__declspec(align(16)) unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpFractal[FractalType::Mand].get_bmpBits();

	int pixelHeight = pApp->get_MandHeight() / pApp->m_iCalcThreadCount[FractalType::Mand];
	int starty = iThreadIndex * pixelHeight;
	int endy = starty + pixelHeight;
	unsigned int iIndex = 0 + starty * pApp->get_MandWidth();

	// initialize SSE variables for loading into registers
	__declspec(align(16)) double a1_sse[POINTS_CONCURRENT_SSE2]	= {af + daf * 0, af + daf * 1};
	const double da_total[POINTS_CONCURRENT_SSE2]				= {daf * POINTS_CONCURRENT_SSE2, daf * POINTS_CONCURRENT_SSE2};
	const double dbf_sse[POINTS_CONCURRENT_SSE2]				= {dbf, dbf,};
	const double bf_sse[POINTS_CONCURRENT_SSE2]					= {bf, bf,};
	const double max_sse[POINTS_CONCURRENT_SSE2]				= {MAXIMUM_MAND_MAGNITUDE, MAXIMUM_MAND_MAGNITUDE,};

	// load the real and imaginary pixel deltas
	__m128d da = _mm_load_pd(da_total); 
	__m128d db = _mm_load_pd(dbf_sse);

	// load the imaginary component
	__m128d b = _mm_load_pd(bf_sse);

	// load the maximum vector magnitude
	__m128d max = _mm_load_pd(max_sse); //load_pd(maximum_sse);

	for (int y = starty; y < endy; ++y)
	{	
		// load the real component
		__m128d a = _mm_load_pd(a1_sse);

		for (int x = 0; x < pApp->get_MandWidth(); x += POINTS_CONCURRENT_SSE2)
		{

			// place 0 in c, d, and iteration count
			__m128d c = _mm_setzero_pd();
			__m128d d = _mm_setzero_pd();
			__m128d iterations = _mm_setzero_pd();

			//
			// start the main calculation loop
			//

			for (int i = 0; i < max_i; ++i)
			{
				// calculate c^2, d^2, and c^2 + d^2
				__m128d c_squ = _mm_mul_pd(c, c);
				__m128d d_squ = _mm_mul_pd(d, d);
				__m128d mag = _mm_add_pd(c_squ, d_squ);

				// bail out if all values are larger than the maximum magnitude (4.0f)
				__m128d cmp = _mm_cmplt_pd(mag, max);	// compare each component to the maximum value
				if (_mm_movemask_pd(cmp) == 0)
				{
					break;
				}

				// increment the iteration count for the pixels that are still less than the maxoimum magnitude
				__m128d tmp = _mm_and_pd(cmp, max);
				iterations = _mm_add_pd(iterations, tmp);

				// calculate 2 * c * d + b
				__m128d cd = _mm_mul_pd(c, d);
				__m128d cd2 = _mm_add_pd(cd, cd);
				d = _mm_add_pd(cd2, b);

				// calculate c^2 - d^2 + a
				__m128d diff = _mm_sub_pd(c_squ, d_squ);
				c = _mm_add_pd(diff, a);

				// calculate the magnitude of the n^2 new values and store them in XMM5
				__m128d result = _mm_add_pd(c_squ, d_squ);

			}
			// adjust the counts by 4
			iterations = _mm_div_pd(iterations, max);
			_mm_store_pd(iterations_sse, iterations);

			// normalize the iteration values between 0 and 255
			l_ppvBits[iIndex++] = (((int)iterations_sse[0]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[0]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[1]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[1]);

			// update the real components
			a = _mm_add_pd(a, da);
		}
		// update the imaginary components
		b = _mm_add_pd(b, db);
	}

	// track the start time and calculate the thread working duration
	QueryPerformanceCounter(&tStop);
	pApp->m_tThreadDuration[FractalType::Mand][iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	// tell the application that the thread is complete
	PostMessage(
		pApp->get_hWnd(), 
		pThreadInfo->iThreadCompleteMessage, 
		pThreadInfo->iThreadCompleteWParam, 
		pThreadInfo->iThreadCompleteLParam);

	return 0;
}

//
// Calculates a mandelbrot set using AVX 32-bit instructions.
// 

DWORD WINAPI CJuliasmApp::CalculateMandAVX32(void* pArguments)
{
	// get Application and thread inde information// get Application and thread inde information
	TThreadInfo *pThread = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThread->pApp;
	int iThreadIndex = pThread->iThreadIndex;
	int max_i = pApp->get_MaxIterationsMand();

	// initialzie the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);

	// determine how much of the image this thread will calculate
	float fThreadHeight = (float)(pApp->m_b2 - pApp->m_b1) / pApp->m_iCalcThreadCount[FractalType::Mand];

	// declare calculation variables used by SSE
	__declspec(align(32)) float iterations_sse[POINTS_CONCURRENT_AVX32];

	__declspec(align(32))float
		af = (float)pApp->m_a1,
		bf = (float)pApp->m_b1 + fThreadHeight * iThreadIndex,
		daf = (float)(pApp->m_a2 - pApp->m_a1) / pApp->get_MandWidth(),
		dbf = (float)(pApp->m_b2 - pApp->m_b1) / pApp->get_MandHeight();


	// get the bitmap bits
	unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpFractal[FractalType::Mand].get_bmpBits();

	int pixelHeight = pApp->get_MandHeight() / pApp->m_iCalcThreadCount[FractalType::Mand];
	int starty = iThreadIndex * pixelHeight;
	int endy = starty + pixelHeight;
	unsigned int iIndex = 0 + starty * pApp->get_MandWidth();

	// initialize SSE variables for loading into SSE data types
	float da_total = daf * POINTS_CONCURRENT_AVX32;
	const __declspec(align(32)) float a1_sse[POINTS_CONCURRENT_AVX32] = {af + daf * 0, af + daf * 1, af + daf * 2, af + daf * 3,af + daf * 4,af + daf * 5,af + daf * 6,af + daf * 7,};
	const __declspec(align(32)) float max_sse = MAXIMUM_MAND_MAGNITUDE;

	// load the real and imaginary pixel deltas
//	__m256 da = _mm256_broadcast_ss(&daf); // broadcast_ss(&da_total); // load_ps(da_sse);
	__m256 da = _mm256_broadcast_ss(&da_total); // broadcast_ss(&da_total); // load_ps(da_sse);
	__m256 db = _mm256_broadcast_ss(&dbf); // _mm256_load_ps(db_sse);

	// load the imaginary component
	__m256 b = _mm256_broadcast_ss(&bf);

	// load the maximum vector magnitude
	__m256 max = _mm256_broadcast_ss(&max_sse); //load_ps(maximum_sse);

	for (int y = starty; y < endy; ++y)
	{

		// load the real component
		__m256 a = _mm256_load_ps(a1_sse);

		for (int x = 0; x < pApp->get_MandWidth(); x += POINTS_CONCURRENT_AVX32)
		{

			// place 0 in c, d, and iteration count
			__m256 c = _mm256_setzero_ps();
			__m256 d = _mm256_setzero_ps();
			__m256 iterations = _mm256_setzero_ps();

			//
			// start the main calculation loop
			//

			for (int i = 0; i < max_i; ++i)
			{
				// calculate c^2, d^2, and c^2 + d^2
				__m256 c_squ = _mm256_mul_ps(c, c);
				__m256 d_squ = _mm256_mul_ps(d, d);
				__m256 mag = _mm256_add_ps(c_squ, d_squ);

				// bail out if all values are larger than the maximum magnitude (4.0f)
				__m256 cmp = _mm256_cmp_ps(mag, max, _CMP_LT_OQ);	// compare each component to the maximum value
				if (_mm256_movemask_ps(cmp) == 0)
				{
					break;
				}

				// increment the iteration count for the pixels that are still less than the maxoimum magnitude
				__m256 tmp = _mm256_and_ps(cmp, max);
				iterations = _mm256_add_ps(iterations, tmp);

				// calculate 2 * c * d + b
				__m256 cd = _mm256_mul_ps(c, d);
				__m256 cd2 = _mm256_add_ps(cd, cd);
				d = _mm256_add_ps(cd2, b);

				// calculate c^2 - d^2 + a
				__m256 diff = _mm256_sub_ps(c_squ, d_squ);
				c = _mm256_add_ps(diff, a);

				// calculate the magnitude of the n^2 new values and store them in XMM5
				__m256 result = _mm256_add_ps(c_squ, d_squ);

			}
			// adjust the counts by 4
			iterations = _mm256_div_ps(iterations, max);
			_mm256_store_ps(iterations_sse, iterations);

			// normalize the iteration values between 0 and 255
			l_ppvBits[iIndex++] = (((int)iterations_sse[0]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[0]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[1]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[1]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[2]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[2]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[3]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[3]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[4]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[4]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[5]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[5]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[6]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[6]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[7]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[7]);

			// update the real components
			a = _mm256_add_ps(a, da);
		}
		// update the imaginary components
		b = _mm256_add_ps(b, db);
	}

	// track the start time and calculate the thread working duration
	QueryPerformanceCounter(&tStop);
	pApp->m_tThreadDuration[FractalType::Mand][iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	//	// tell the application that the thread is complete
	PostMessage(
		pApp->get_hWnd(), 
		pThread->iThreadCompleteMessage, 
		pThread->iThreadCompleteWParam, 
		pThread->iThreadCompleteLParam);

	return 0;
}


//
// Calculates a mandelbrot set using AVX 64-bit instructions.
// 

DWORD WINAPI CJuliasmApp::CalculateMandAVX64(void* pArguments)
{
	// get Application and thread inde information
	TThreadInfo *pThread = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThread->pApp;
	int iThreadIndex = pThread->iThreadIndex;
	__declspec(align(32))int max_i = pApp->get_MaxIterationsMand();

	// initialzie the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);

	// determine how much of the image this thread will calculate
	double fThreadHeight = (double)(pApp->m_b2 - pApp->m_b1) / pApp->m_iCalcThreadCount[FractalType::Mand];

	// declare calculation variables used by SSE
	__declspec(align(32)) double iterations_sse[POINTS_CONCURRENT_AVX64];

	__declspec(align(32))double
		af = (double)pApp->m_a1,
		bf = (double)pApp->m_b1 + fThreadHeight * iThreadIndex,
		daf = (double)(pApp->m_a2 - pApp->m_a1) / pApp->get_MandWidth(),
		dbf = (double)(pApp->m_b2 - pApp->m_b1) / pApp->get_MandHeight();


	// get the bitmap bits
	__declspec(align(32)) unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpFractal[FractalType::Mand].get_bmpBits();

	int pixelHeight = pApp->get_MandHeight() / pApp->m_iCalcThreadCount[FractalType::Mand];
	int starty = iThreadIndex * pixelHeight;
	int endy = starty + pixelHeight;
	unsigned int iIndex = 0 + starty * pApp->get_MandWidth();

	// initialize SSE variables for loading into SSE data types
	double da_total = daf * POINTS_CONCURRENT_AVX64;
	const __declspec(align(32)) double a1_sse[POINTS_CONCURRENT_AVX64] = {af + daf * 0, af + daf * 1, af + daf * 2, af + daf * 3,};
	const __declspec(align(32)) double max_sse = MAXIMUM_MAND_MAGNITUDE;

	// load the real and imaginary pixel deltas
	__m256d da = _mm256_broadcast_sd(&da_total); // broadcast_ss(&da_total); // load_pd(da_sse);
	__m256d db = _mm256_broadcast_sd(&dbf); // _mm256_load_pd(db_sse);

	// load the imaginary component
	__m256d b = _mm256_broadcast_sd(&bf);

	// load the maximum vector magnitude
	__m256d max = _mm256_broadcast_sd(&max_sse); //load_pd(maximum_sse);

	for (int y = starty; y < endy; ++y)
	{
		// load the real component
		__m256d a = _mm256_load_pd(a1_sse);

		for (int x = 0; x < pApp->get_MandWidth(); x += POINTS_CONCURRENT_AVX64)
		{

			// place 0 in c, d, and iteration count
			__m256d c = _mm256_setzero_pd();
			__m256d d = _mm256_setzero_pd();
			__m256d iterations = _mm256_setzero_pd();

			//
			// start the main calculation loop
			//

			for (int i = 0; i < max_i; ++i)
			{
				// calculate c^2, d^2, and c^2 + d^2
				__m256d c_squ = _mm256_mul_pd(c, c);
				__m256d d_squ = _mm256_mul_pd(d, d);
				__m256d mag = _mm256_add_pd(c_squ, d_squ);

				// bail out if all values are larger than the maximum magnitude (4.0f)
				__m256d cmp = _mm256_cmp_pd(mag, max, _CMP_LT_OQ);	// compare each component to the maximum value
				if (_mm256_movemask_pd(cmp) == 0)
				{
					break;
				}

				// increment the iteration count for the pixels that are still less than the maxoimum magnitude
				__m256d tmp = _mm256_and_pd(cmp, max);
				iterations = _mm256_add_pd(iterations, tmp);

				// calculate 2 * c * d + b
				__m256d cd = _mm256_mul_pd(c, d);
				__m256d cd2 = _mm256_add_pd(cd, cd);
				d = _mm256_add_pd(cd2, b);

				// calculate c^2 - d^2 + a
				__m256d diff = _mm256_sub_pd(c_squ, d_squ);
				c = _mm256_add_pd(diff, a);

				// calculate the magnitude of the n^2 new values and store them in XMM5
				__m256d result = _mm256_add_pd(c_squ, d_squ);

			}
			// adjust the counts by 4
			iterations = _mm256_div_pd(iterations, max);
			_mm256_store_pd(iterations_sse, iterations);

			// normalize the iteration values between 0 and 255
			l_ppvBits[iIndex++] = (((int)iterations_sse[0]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[0]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[1]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[1]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[2]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[2]);
			l_ppvBits[iIndex++] = (((int)iterations_sse[3]) == max_i) ? 0 : pApp->m_PaletteDefault.get_Color((int)iterations_sse[3]);

			// update the real components
			a = _mm256_add_pd(a, da);
		}
		// update the imaginary components
		b = _mm256_add_pd(b, db);
	}

	// track the start time and calculate the thread working duration
	QueryPerformanceCounter(&tStop);
	pApp->m_tThreadDuration[FractalType::Mand][iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	// tell the application that the thread is complete
	PostMessage(
		pApp->get_hWnd(), 
		pThread->iThreadCompleteMessage, 
		pThread->iThreadCompleteWParam, 
		pThread->iThreadCompleteLParam);
	
	return 0;
}
DWORD WINAPI CJuliasmApp::CalculateMandFMA(void* pArguments)
{
	TThreadInfo *pThread = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThread->pApp;

	// tell the application that the thread is complete
	PostMessage(
		pApp->get_hWnd(), 
		pThread->iThreadCompleteMessage, 
		pThread->iThreadCompleteWParam, 
		pThread->iThreadCompleteLParam);

	return 0;
}
DWORD WINAPI CJuliasmApp::CalculateMandOpenCL(void* pArguments)
{
	TThreadInfo *pThread = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThread->pApp;

	// Execute the calculation
	cl_int error;
	if (false == pApp->m_OCLMand.ExecuteProgram(pThread->iKernelNumber, &error))
	{
		char szBuf[64];
		sprintf_s(szBuf, _countof(szBuf), "Error %d executing OpenCL kernel.", error);
		MessageBox(NULL, szBuf, "Error", MB_ICONEXCLAMATION);
	}
	pThread->oclError = error;

	// tell the application that the thread is complete
	PostMessage(
		pApp->get_hWnd(), 
		pThread->iThreadCompleteMessage, 
		pThread->iThreadCompleteWParam, 
		pThread->iThreadCompleteLParam);

	return 0;
}
