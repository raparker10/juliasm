#ifndef __CPixelPoint_H
#define __CPixelPoint_H
class CPixelPoint {
	int l_x, l_y;
public:
	inline void put_x(const int x) { l_x = x; }
	inline void put_y(const int y) { l_y = y; }
	inline void put_Point(const int x, const int y) { put_x(x); put_y(y); }
	inline int  get_x(void) const { return l_x; }
	inline int get_y(void) const { return l_y; }

	CPixelPoint() :
		l_x(0),
		l_y(0)
	{}

	CPixelPoint(const int x, const int y) :
		l_x(x),
		l_y(y)
	{}
	CPixelPoint(const CPixelPoint &p) :
		l_x(p.l_x), 
		l_y(p.l_y)
	{}

	CPixelPoint & operator=(const CPixelPoint &p)
	{
		l_x = p.l_x;
		l_y = p.l_y;
		return *this;
	}
};

class CPixelRect {
	CPixelPoint l_p[2];
public:
	inline void put_x1(const int x) { l_p[0].put_x(x); }
	inline void put_y1(const int y) { l_p[0].put_y(y); }
	inline void put_Point1(const int x, const int y) { l_p[0].put_x(x); l_p[0].put_y(y); }
	inline void put_Point1(const CPixelPoint &p) { l_p[0] = p; }
	inline int get_x1(void) const { return l_p[0].get_x(); }
	inline int get_y1(void) const { return l_p[0].get_y(); }

	inline void put_x2(const int x) { l_p[1].put_x(x); }
	inline void put_y2(const int y) { l_p[1].put_y(y); }
	inline void put_Point2(const int x, const int y) { l_p[1].put_x(x); l_p[1].put_y(y); }
	inline void put_Point2(const CPixelPoint &p) { l_p[1] = p; }
	inline int get_x2(void) const { return l_p[1].get_x(); }
	inline int get_y2(void) const { return l_p[1].get_y(); }

	inline int get_Width(void) const
	{
		return get_x2() - get_x1();
	}
	inline int get_Height(void) const
	{
		return get_y2() - get_y1();
	}

	CPixelRect() {}
	CPixelRect(const CPixelPoint &p1, const CPixelPoint &p2)
	{
		l_p[0] = p1;
		l_p[1] = p2;
	}
};
#endif