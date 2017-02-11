class CDIBitmap {
	protected:
	BITMAPINFO m_bmi;
	HBITMAP m_hbmpCanvas;
	LPVOID m_ppvBits;
	int m_iWidth, m_iHeight;
	virtual bool CreateDIBitmap(HWND hWnd, int m_iWidth, int m_iHeight, BITMAPINFO *bmi, HBITMAP *hbmpCanvas, VOID **ppvBits) = 0;

public:
	CDIBitmap();
	~CDIBitmap();

	virtual bool Resize(HWND hWnd, int iWidth, int iHeight, bool bCopyOriginal = false) = 0;
	virtual bool Erase(unsigned int rgbColor) = 0;

	inline BITMAPINFO *get_bmpInfo(void) {
		assert(m_ppvBits != NULL);
		assert(m_hbmpCanvas != NULL);
		return &m_bmi;
	}
	inline HBITMAP get_Bitmap(void) const {
		assert(m_hbmpCanvas != NULL); // canvas can't be null
		return m_hbmpCanvas;
	}
	inline VOID *get_bmpBits(void) {
		assert(m_hbmpCanvas != NULL);
		assert(m_ppvBits != NULL);
		return m_ppvBits;
	}
	inline int get_Height(void) const {
		assert(m_hbmpCanvas != NULL);
		assert(m_ppvBits != NULL);
		assert(m_iHeight > 0); // height must be greater than zero
		return m_iHeight;
	}
	inline int get_Width(void) const {
		assert(m_hbmpCanvas != NULL);
		assert(m_ppvBits != NULL);
		assert(m_iWidth >= 0);
		return m_iWidth;
	}
	int Show(HDC hdcDest, int iDestX, int iDestY, int iDestWidth, int iDestHeight, int iSourceX, int iSourceY, int iSourceWidth, int iSourceHeight, DWORD dwROP);
	int Show(HDC hdcDest, int iDestX, int iDestY);
};

class CBitmap256Color : public CDIBitmap {
protected:
	bool CreateDIBitmap(HWND hWnd, int m_iWidth, int m_iHeight, BITMAPINFO *bmi, HBITMAP *hbmpCanvas, VOID **ppvBits);
public:
	CBitmap256Color();
	~CBitmap256Color();
	virtual bool Resize(HWND hWnd, int iWidth, int iHeight, bool bCopyOriginal = false);
	virtual bool Erase(unsigned int rgbColor);
};