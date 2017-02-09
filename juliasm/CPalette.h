#ifndef __CPalette_H
#define __CPalette_H

#include "CPixelPoint.h"
class CPalette {
public:
	static const int MAX_COLORS=256;
	static const int MAX_COLOR_POINTS=32;
	static const unsigned char WHITE=0xFF;
	static const unsigned char BLACK=0x00;
	enum EColorChannels {CHANNEL_RED=0, CHANNEL_GREEN, CHANNEL_BLUE, NUMBER_COLOR_CHANNELS};

protected:
	// get the red, green, and blue color ramps
	unsigned char ucRed[MAX_COLORS];
	unsigned char ucGreen[MAX_COLORS];
	unsigned char ucBlue[MAX_COLORS];

	CPixelPoint l_ColorPoints[NUMBER_COLOR_CHANNELS][MAX_COLOR_POINTS];
	int l_iNumberColorPoints[NUMBER_COLOR_CHANNELS];
	COLORREF l_Colors[MAX_COLORS];
	bool l_bDirty;
	bool l_bBlackAndWhite;
	void FillPalette(unsigned char *ucPalette, CPixelPoint * pPalettePoints, unsigned int iNumberPalettePoints);
public:
	CPalette();
	void PushColorPoint(const int iChannel, const CPixelPoint &p);
	COLORREF get_Color(int iPaletteEntry);	
	void UpdateColors(void);
	CPalette & operator=(const CPalette &p);
	inline void put_BlackAndWhite(const bool bBlackAndWhite) { 
		l_bDirty = l_bBlackAndWhite != bBlackAndWhite;;
		l_bBlackAndWhite = bBlackAndWhite; 
	}
	inline bool get_BlackAndWhite(void) const { return l_bBlackAndWhite; }
	unsigned char *get_RedChannel(void) { return ucRed; }
	unsigned char *get_GreenChannel(void) { return ucGreen; }
	unsigned char *get_BlueChannel(void) { return ucBlue; }
};
#endif