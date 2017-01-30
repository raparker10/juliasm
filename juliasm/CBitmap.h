class CDIBitmap {
	protected:
	BITMAPINFO m_bmi;
	HBITMAP m_hbmpCanvas;
	VOID *m_ppvBits;
	int m_iWidth, m_iHeight;
	virtual bool CreateDIBitmap(HWND hWnd, int m_iWidth, int m_iHeight, BITMAPINFO *bmi, HBITMAP *hbmpCanvas, VOID **ppvBits) = 0;

public:
	CDIBitmap();
	~CDIBitmap();

	virtual bool Resize(HWND hWnd, int iWidth, int iHeight, bool bCopyOriginal = false) = 0;
	virtual bool Erase(unsigned int rgbColor) = 0;

	inline BITMAPINFO *get_bmpInfo(void) {
		return &m_bmi;
	}
	inline HBITMAP get_Bitmap(void) const {
		return m_hbmpCanvas;
	}
	inline VOID *get_bmpBits(void) {
		return m_ppvBits;
	}
	inline int get_Height(void) const {
		return m_iHeight;
	}
	inline int get_Width(void) const {
		return m_iWidth;
	}
	inline int Show(HDC hdcDest, int iDestX, int iDestY, int iDestWidth, int iDestHeight, int iSourceX, int iSourceY, int iSourceWidth, int iSourceHeight, DWORD dwROP)
	{
		return StretchDIBits(
			hdcDest,
			iDestX, iDestY, iDestWidth, iDestHeight,
			iSourceX, iSourceY, iSourceWidth, iSourceHeight,
			m_ppvBits,
			&m_bmi,
			DIB_RGB_COLORS,
			dwROP);
	}
	inline int Show(HDC hdcDest, int iDestX, int iDestY)
	{
		return Show(hdcDest,
			iDestX, iDestY, m_iWidth, m_iHeight,
			0, 0, m_iWidth, m_iHeight,
			SRCCOPY);
	}

};

class CBitmap256Color : public CDIBitmap {
	bool CreateDIBitmap(HWND hWnd, int m_iWidth, int m_iHeight, BITMAPINFO *bmi, HBITMAP *hbmpCanvas, VOID **ppvBits);
public:
	CBitmap256Color();
	~CBitmap256Color();
	virtual bool Resize(HWND hWnd, int iWidth, int iHeight, bool bCopyOriginal = false);
	virtual bool Erase(unsigned int rgbColor);
};