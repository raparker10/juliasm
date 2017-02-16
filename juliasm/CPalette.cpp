#include "stdafx.h"
#include "CPalette.h"
/*
class CPalette {
	CPixelPoint l_ColorPoints[32];
	int l_iNumberColorPoints;
	COLORREF l_Colors[256];
	bool l_bDirty;
public:
*/
CPalette::CPalette() :
	l_bDirty(false)
{
	int i;

	l_bBlackAndWhite = false;

	// initialize the number of color points to 0
	for (i = 0; i < NUMBER_COLOR_CHANNELS; ++i)
	{
		l_iNumberColorPoints[i] =0;
	}

	// set all colors to black
	for (int i = 0; i < sizeof(l_Colors) / sizeof(l_Colors[0]); ++i)
	{
		l_Colors[i] = BLACK;
	}
}

void CPalette::PushColorPoint(const int iChannel, const CPixelPoint &p)
{
	if (iChannel < 0 || iChannel >= NUMBER_COLOR_CHANNELS)
		return;

	l_ColorPoints[iChannel][l_iNumberColorPoints[iChannel]++] = p;
	l_bDirty = true;
}
COLORREF CPalette::get_Color(int iPaletteEntry)
{
	return l_Colors[iPaletteEntry % 255];
}
COLORREF CPalette::get_Color(float fPaletteEntry)
{
	int iCol1 = (int)trunc(fPaletteEntry) % MAX_COLORS;
	int iCol2 = (iCol1 + 1) % MAX_COLORS;

	float col2pct = fPaletteEntry - trunc(fPaletteEntry);
	float col1pct = 1.0f - col2pct;

	int Red1 = l_Colors[iCol1] & 0x0FF;
	int Green1 = (l_Colors[iCol1] >> 8) & 0x0FF;
	int Blue1 = (l_Colors[iCol1] >> 16) & 0x0FF;

	int Red2 = l_Colors[iCol2] & 0x0FF;
	int Green2 = (l_Colors[iCol2] >> 8) & 0x0FF;
	int Blue2 = (l_Colors[iCol2] >> 16) & 0x0FF;

	if (Red1 > 0)
		Red1 = Red1;

	int Red = (int)min(MAX_COLORS - 1, Red1 * col1pct + Red2 * col2pct);
	int Green = (int)min(MAX_COLORS - 1, Green1 * col1pct + Green2 * col2pct);
	int Blue = (int)min(MAX_COLORS - 1, Blue1 * col1pct + Blue2 * col2pct);

	COLORREF c = RGB(Red, Green, Blue);
	return c;
}
		
void CPalette::UpdateColors(void)
{
	if (l_bDirty == false)
		return;

	// if this is a B&W palette, ignore the color arrays and just set all colors to alternating BW values
	if (get_BlackAndWhite())
	{
		for (int i = 0; i < MAX_COLORS; ++i)
		{
			l_Colors[i] = RGB(i % 2 * WHITE, i % 2 * WHITE, i % 2 * WHITE);
		}
		return;
	}

	FillPalette(ucRed, l_ColorPoints[CHANNEL_RED], l_iNumberColorPoints[CHANNEL_RED]);
	FillPalette(ucGreen, l_ColorPoints[CHANNEL_GREEN], l_iNumberColorPoints[CHANNEL_GREEN]);
	FillPalette(ucBlue, l_ColorPoints[CHANNEL_BLUE], l_iNumberColorPoints[CHANNEL_BLUE]);

	// combine color ramps into palette colors
	for (int i = 0; i < MAX_COLORS; ++i)
	{
		l_Colors[i] = RGB(ucRed[i], ucGreen[i], ucBlue[i]);
	}
}

void CPalette::FillPalette(unsigned char *ucPalette, CPixelPoint * pPalettePoints, unsigned int iNumberPalettePoints)
{
	int x;
	// set to black?
	if (iNumberPalettePoints == 0)
	{
		for (x = 0; x < MAX_COLORS; ++x)
			ucPalette[x] = 0;
		return;
	}

	// set to single color?
	if (iNumberPalettePoints == 1)
	{
		for (x = 0; x < MAX_COLORS; ++x)
			ucPalette[x] = pPalettePoints[0].get_y();
		return;
	}

	// interpolate colors
	for (unsigned int i = 0; i < iNumberPalettePoints - 1; ++i)
	{
		CPixelPoint *pCur = &pPalettePoints[i];
		CPixelPoint *pNext = &pPalettePoints[i + 1];

		for (x = pCur->get_x(); x <= pNext->get_x(); ++x)
		{
			ucPalette[x] = (unsigned char)(pCur->get_y() + (x - pCur->get_x()) * (pNext->get_y() - pCur->get_y()) / (pNext->get_x() - pCur->get_x()));
		}
	}
}

CPalette & CPalette::operator=(const CPalette &p)
{
	int i;

	// copy the dirty indicator
	l_bDirty = p.l_bDirty;

	for (int c = 0; c < NUMBER_COLOR_CHANNELS; ++c)
	{
		// copy the color counts
		l_iNumberColorPoints[c] = p.l_iNumberColorPoints[c];

		// copy the color points
		for (int i = 0; i < l_iNumberColorPoints[c]; ++i)
		{
			l_ColorPoints[c][i] = p.l_ColorPoints[c][i];
		}
	}
	for (i = 0; i < MAX_COLORS; ++i)
	{
		l_Colors[i] = p.l_Colors[i];
	}
	return *this;
}

CPaletteZebra::CPaletteZebra() : CPalette()
{
}
CPaletteZebra::~CPaletteZebra()
{
}

void CPaletteZebra::UpdateColors(void)
{
}
