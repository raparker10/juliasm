#include "stdafx.h"

// Message handler for about box.
INT_PTR CALLBACK CJuliasmApp::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

// Message handler for about box.
INT_PTR CALLBACK CJuliasmApp::HelpCPU(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CJuliasmApp *pApp;
	char szBuf[64];
	char szFamily[32];
	char szModel[8];
	char szStepping[8];

	switch (message)
	{
	case WM_INITDIALOG:
		pApp = (CJuliasmApp*)lParam;
		// set the vendor ID
		pApp->cpu.get_VendorID(szBuf, _countof(szBuf));
		SendDlgItemMessage(hDlg, IDC_CPU_VENDOR, WM_SETTEXT, 0, (LPARAM)szBuf);

		// set the CPU Stepping
		//		SendDlgItemMessage(hDlg, IDC_CPU_STEPPING, WM_SETTEXT, 0, (LPARAM)szBuf);

		// set the CPU Model
		//		SendDlgItemMessage(hDlg, IDC_CPU_MODEL, WM_SETTEXT, 0, (LPARAM)szBuf);

		// set the Family
		pApp->cpu.get_FamilyName(szFamily, _countof(szFamily));
		pApp->cpu.get_ModelName(szModel, _countof(szModel));
		pApp->cpu.get_SteppingName(szStepping, _countof(szStepping));
		strcat_s(szFamily, _countof(szFamily), "h_");
		strcat_s(szFamily, _countof(szFamily), szModel);
		strcat_s(szFamily, _countof(szFamily), "h_");
		strcat_s(szFamily, _countof(szFamily), szStepping);
		strcat_s(szFamily, _countof(szFamily), "h");

		SendDlgItemMessage(hDlg, IDC_CPU_FAMILY, WM_SETTEXT, 0, (LPARAM)szFamily);

		pApp->cpu.get_MicroarchitectureName(szBuf, _countof(szBuf));
		SendDlgItemMessage(hDlg, IDC_CPU_MICROARCHITECTURE, WM_SETTEXT, 0, (LPARAM)szBuf);


		// set the Processor Type
		pApp->cpu.get_ProcessorTypeName(szBuf, _countof(szBuf));
		SendDlgItemMessage(hDlg, IDC_CPU_PROCESSOR_TYPE, WM_SETTEXT, 0, (LPARAM)szBuf);

		SendDlgItemMessage(hDlg, IDC_HAS_x87, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_x87() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_MMX, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_MMX() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_SSE, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_SSE() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_SSE2, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_SSE2() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_SSSE3, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_SSSE3() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_SSE41, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_SSE4_1() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_SSE42, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_SSE4_2() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_AVX, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_AVX() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_AVX2, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_AVX2() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_AVX512, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_AVX512F() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_CLMUL, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_CLMUL() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_FMA3, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_FMA3() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_ADX, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_ADX() ? "X" : ""));
		SendDlgItemMessage(hDlg, IDC_HAS_SHA, WM_SETTEXT, 0, (LPARAM)(pApp->cpu.has_SHA() ? "X" : ""));


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
