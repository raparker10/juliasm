#include "stdafx.h"

//	BITMAPINFO l_bmi;
//	HBITMAP l_hbmpCanvas;
//	VOID *l_ppvBits;

CDIBitmap::CDIBitmap() :
	m_hbmpCanvas(NULL),
	m_ppvBits((VOID*)NULL),
	m_iWidth(0),
	m_iHeight(0)
{
	ZeroMemory(&m_hbmpCanvas, sizeof(m_hbmpCanvas));
};
CDIBitmap::~CDIBitmap()
{

}
CBitmap256Color::CBitmap256Color()
{
}

CBitmap256Color::~CBitmap256Color()
{
}

bool CBitmap256Color::CreateDIBitmap(HWND hWnd, int iWidth, int iHeight, BITMAPINFO *bmi, HBITMAP *hbmpCanvas, VOID **ppvBits)
{
	if (bmi == NULL || hbmpCanvas == NULL || *ppvBits == NULL)
		return false;

	// initialize the bitmapinfo structure
	HDC hdc = GetDC(hWnd);
	bmi->bmiHeader.biSize = sizeof(bmi->bmiHeader);
	bmi->bmiHeader.biWidth = iWidth;
	bmi->bmiHeader.biHeight = -iHeight;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = 32;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biSizeImage = 0;
	bmi->bmiHeader.biXPelsPerMeter = 0;
	bmi->bmiHeader.biYPelsPerMeter = 0;
	bmi->bmiHeader.biClrUsed = 0;
	bmi->bmiHeader.biClrImportant = 0;
	*hbmpCanvas = CreateDIBSection(hdc, bmi, DIB_RGB_COLORS, ppvBits, NULL, 0);
	ReleaseDC(hWnd, hdc);

	return (hbmpCanvas != NULL);
}

bool CBitmap256Color::Resize(HWND hWnd, int iWidth, int iHeight, bool bCopyOriginal)
{
	BITMAPINFO bmi;
	HBITMAP hbmpCanvas;
	VOID *ppvBits;

	// return success if the requested height is the same as the current height
	if (iWidth == m_iWidth && iHeight == m_iHeight)
		return true;

	if (NULL != CreateDIBitmap(hWnd, iWidth, iHeight, &bmi, &hbmpCanvas, &ppvBits))
	{
		// copy the locatl values into the member values
		memcpy(&m_bmi, &bmi, sizeof(bmi));
		memcpy(&m_hbmpCanvas, &hbmpCanvas, sizeof(hbmpCanvas));
		memcpy(&m_ppvBits, &ppvBits, sizeof(ppvBits));
		m_iHeight = iHeight;
		m_iWidth = iWidth;

		// clear the screen to WHITE by default
		return Erase(RGB(255, 255, 255));
	}

	return false;
}

bool CBitmap256Color::Erase(unsigned int rgbColor)
{		
	if (m_ppvBits == NULL)
		return false;

	for (int i = 0; i < m_iHeight * m_iWidth; ++i)
	{
		((unsigned int*)m_ppvBits)[i] = rgbColor;
	}
	return true;
}