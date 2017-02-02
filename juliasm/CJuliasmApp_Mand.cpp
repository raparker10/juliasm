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
//	get_CalcPlatformMand()
DWORD WINAPI CJuliasmApp::CalculateMandX87(void* pArguments)
{
	// get Application and thread inde information
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;
	pApp->put_CalcPlatformMand(CalcPlatform::x87);
	int iThreadIndex = pThreadInfo->iThreadIndex;


	// initialzie the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);


	// determine how much of the image this thread will calculate
	double fThreadHeight = (pApp->m_b2 - pApp->m_b1) / pApp->m_iMandelbrotThreadCount;

	double
		_a = pApp->m_a1,
		_b = pApp->m_b1 + fThreadHeight * iThreadIndex,
		da = (pApp->m_a2 - pApp->m_a1) / pApp->m_iMandWidth,
		db = (pApp->m_b2 - pApp->m_b1) / pApp->m_iMandHeight;

	pApp->m_da = da;
	pApp->m_db = db;

	// get the bitmap bits
	unsigned int* l_ppvBits = (unsigned int*)pApp->m_bmpMandelbrot.get_bmpBits();

	int pixelHeight = pApp->m_iMandHeight / pApp->m_iMandelbrotThreadCount;
	int starty = iThreadIndex * pixelHeight;
	int endy = starty + pixelHeight;

	unsigned int iIndex = 0 + starty * pApp->m_iMandWidth;

	// calculate each row...
	for (int y = starty; y < endy; ++y)
	{
		// calculate each pixel...
		_a = pApp->m_a1;
		for (int x = 0; x < pApp->m_iMandWidth; x += 1)
		{
			// calculate each iteration...
			double c = 0.0f;
			double d = 0.0f;
			double c2, d2, cd2;
			double mag;
			int i;
			for (i = 0; i < pApp->get_MaxIterationsMand(); ++i)
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
			l_ppvBits[iIndex++] = (i == pApp->get_MaxIterationsMand()) ? 0 : pApp->m_PaletteDefault.get_Color(i);
		}
		_b += db;
	}
	// stop the performance counter
	QueryPerformanceCounter(&tStop);
//	m_tMandelbrotProcessDurationTotal.QuadPart 
//		= m_tMandelbrotThreadDurationTotal.QuadPart 
	pApp->m_tMandelbrotThreadDuration[iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	// tell the application that the thread is complete
	PostMessage(pApp->get_hWnd(), WM_COMMAND, IDM_THREADCOMPLETE, 0);

	return 0;
}


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
void CJuliasmApp::CalculateMandX87SingleThread(void)
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
	m_tMandelbrotProcessDurationTotal.QuadPart 
		= m_tMandelbrotThreadDurationTotal.QuadPart 
		= m_tMandelbrotThreadDuration[iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;
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
		mov ax, WORD PTR tmp

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

	// initialzie the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);


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

	// track the start time and calculate the thread working duration
	QueryPerformanceCounter(&tStop);
	pApp->m_tMandelbrotThreadDuration[iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

	// tell the application that the thread is complete
	PostMessage(pApp->get_hWnd(), WM_COMMAND, IDM_THREADCOMPLETE, 0);

	return 0;
}
DWORD WINAPI CJuliasmApp::CalculateMandSSE2(void* pArguments)
{
	TThreadInfo *pThreadInfo = (TThreadInfo*)pArguments;
	CJuliasmApp *pApp = pThreadInfo->pApp;

	// initialize the performance counter
	LARGE_INTEGER tStart, tStop;
	QueryPerformanceCounter(&tStart);

	// tell the Application that the calculation is being done with SSE2
	pApp->put_CalcPlatformMand(CalcPlatform::SSE2);

	// get the thread index (for storing information back to the Application object
	// and subdividing the task
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
	pApp->m_tMandelbrotThreadDuration[iThreadIndex].QuadPart = tStop.QuadPart - tStart.QuadPart;

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





