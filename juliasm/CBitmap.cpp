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
	// release any existing bitmap
	if (m_hbmpCanvas != NULL)
	{
		DeleteObject(m_hbmpCanvas);
		m_hbmpCanvas = NULL;
	}

}
int CDIBitmap::Show(HDC hdcDest, int iDestX, int iDestY, int iDestWidth, int iDestHeight, int iSourceX, int iSourceY, int iSourceWidth, int iSourceHeight, DWORD dwROP)
{
		assert(m_hbmpCanvas != NULL);
		assert(m_ppvBits != NULL);
		assert(m_iWidth >= 0);
		assert(m_iHeight >= 0);
		assert(iDestWidth > 0);
		assert(iDestHeight > 0);

		int iLinesDisplayed = StretchDIBits(
			hdcDest,
			iDestX, iDestY, iDestWidth, iDestHeight,
			iSourceX, iSourceY, iSourceWidth, iSourceHeight,
			m_ppvBits,
			&m_bmi,
			DIB_RGB_COLORS,
			dwROP);
		assert(iLinesDisplayed == iDestHeight); // must display all lines
		return iLinesDisplayed;
	}
int CDIBitmap::Show(HDC hdcDest, int iDestX, int iDestY)
	{
		int iLinesDisplayed =  Show(hdcDest,
			iDestX, iDestY, m_iWidth, m_iHeight,
			0, 0, m_iWidth, m_iHeight,
			SRCCOPY);
		assert(iLinesDisplayed == m_iHeight); // must display all lines
		return iLinesDisplayed;
	}

CBitmap256Color::CBitmap256Color()
{
}

CBitmap256Color::~CBitmap256Color()
{
}

bool CBitmap256Color::CreateDIBitmap(HWND hWnd, int iWidth, int iHeight, BITMAPINFO *bmi, HBITMAP *hbmpCanvas, VOID **ppvBits)
{
	assert(bmi != NULL);
	assert(hbmpCanvas != NULL);
	assert(ppvBits != NULL);

	// release any existing bitmap
	if (m_hbmpCanvas != NULL)
	{
		DeleteObject(m_hbmpCanvas);
		m_hbmpCanvas = NULL;
	}

	if (bmi == NULL || hbmpCanvas == NULL || ppvBits == NULL)
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
	assert(*hbmpCanvas != NULL);
	ReleaseDC(hWnd, hdc);

	m_hbmpCanvas = *hbmpCanvas;

	return (hbmpCanvas != NULL);
}

bool CBitmap256Color::Resize(HWND hWnd, int iWidth, int iHeight, bool bCopyOriginal)
{
	BITMAPINFO bmi;
	HBITMAP hbmpCanvas = NULL;
	LPVOID ppvBits = NULL;

	// return success if the requested height is the same as the current height
	if (iWidth == m_iWidth && iHeight == m_iHeight)
		return true;

	// if either the height or width are 0, simple free any existing bitmap
	if (iWidth == 0 || iHeight == 0)
	{
		if (m_hbmpCanvas != NULL)
		{
			DeleteObject(m_hbmpCanvas);
			m_hbmpCanvas = NULL;
			memset(&m_bmi, 0, sizeof(m_bmi));
			m_ppvBits = NULL;
			m_iWidth = 0;
			m_iHeight = 0;
		}
		return true; // returns success, but there is no bitmap
	}

	if (NULL != CreateDIBitmap(hWnd, iWidth, iHeight, &bmi, &hbmpCanvas, &ppvBits))
	{
		// copy the locatl values into the member values
		memcpy(&m_bmi, &bmi, sizeof(bmi));
		memcpy(&m_hbmpCanvas, &hbmpCanvas, sizeof(hbmpCanvas));
//		memcpy(&m_ppvBits, &ppvBits, sizeof(ppvBits));
		m_ppvBits = ppvBits;

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