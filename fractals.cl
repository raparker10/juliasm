/* This is a test */
constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;


__kernel void image_mandelbrot(write_only image2d_t image, read_only float4 num_rect, read_only float maxi, __global __read_only float4 *palette)
{
	int offset_x = get_global_id(0), 
		offset_y = get_global_id(1);

	uint width = get_image_width(image);
	uint height = get_image_height(image);

	float a1 = num_rect.x, // -2.0f, 
		a2 = num_rect.z, // 2.0f		
		b1 = num_rect.y, // -2.0f, 
		b2 = num_rect.w, // 2.0f,
		da = (a2 - a1) / width,
		db = (b2 - b1) / height;

	float a, b, c, c2, d;
	c = d = 0.0f;
	a = a1 + da * offset_x;
	b = b1 + db * offset_y;

	float4 color;

	int i_maxi = (int)maxi;
	int i;
	unsigned char iColor;
	for (i = 0, iColor=0; i < i_maxi; ++i, ++iColor)
	{
		if (c * c + d * d > 4.0f)
		{
			color = palette[iColor];
			break;
		}
		c2 = 2 * c * d;
		c = c * c - d * d + a;
		d = c2 + b;
	}

	write_imagef(image, (int2)(offset_x, offset_y), color);
}

__kernel void image_julia(write_only image2d_t image, read_only float2 pt, read_only float4 num_rect, read_only float maxi, __global __read_only float4 *palette)
{
	int offset_x = get_global_id(0), 
		offset_y = get_global_id(1);

	uint width = get_image_width(image);
	uint height = get_image_height(image);

	float c1 = num_rect.x, // -2.0f, 
		c2 = num_rect.z, // 2.0f, 
		d1 = num_rect.y, // -2.0f, 
		d2 = num_rect.w, // 2.0f,
		dc = (c2 - c1) / width,
		dd = (d2 - d1) / height;

	float a = pt.x,
		b = pt.y;

	float c = c1 + offset_x * dc;
	float d = d1 + offset_y * dd;

	float4 color;

	int i_maxi = (int)maxi;
	int i;
	unsigned char iColor;
	for (i = 0, iColor=0; i < i_maxi; ++i, ++iColor)
	{
		if (c * c + d * d > 4.0f)
		{
			color = palette[iColor];
//			color = palette[offset_x % 255];
			break;
		}
		c2 = 2 * c * d;
		c = c * c - d * d + a;
		d = c2 + b;
	}

	write_imagef(image, (int2)(offset_x, offset_y), color);
}
/* ********************************************************
__kernel void image_julia(write_only image2d_t image, read_only float2 pt, read_only float4 num_rect, read_only float maxi, __global __read_only float4 *palette)
{
	int offset_x = get_global_id(0), 
		offset_y = get_global_id(1);

	uint width = get_image_width(image);
	uint height = get_image_height(image);

	float a1 = num_rect.x, // -2.0f, 
		a2 = num_rect.z, // 2.0f, 
		b1 = num_rect.y, // -2.0f, 
		b2 = num_rect.w, // 2.0f,
		da = (a2 - a1) / width,
		db = (b2 - b1) / height;

	float a, b, c, c2, d;
	c = d = 0.0f;
	a = a1 + da * offset_x;
	b = b1 + db * offset_y;

	float4 color;

	int i_maxi = (int)maxi;
	int i;
	unsigned char iColor;
	for (i = 0, iColor=0; i < i_maxi; ++i, ++iColor)
	{
		if (c * c + d * d > 4.0f)
		{
//			color = palette[iColor];
			color = palette[offset_x % 255];
			break;
		}
		c2 = 2 * c * d;
		c = c * c - d * d + a;
		d = c2 + b;
	}

	write_imagef(image, (int2)(offset_x, offset_y), color);
}



*/
__kernel void image_mandelbrot_worksize(write_only image2d_t image, read_only float4 num_rect, read_only float maxi, __global __read_only float4 *palette)
{
	int offset_x = get_global_id(0), 
		offset_y = get_global_id(1);

	uint width = get_image_width(image);
	uint height = get_image_height(image);

	float a1 = num_rect.x, // -2.0f, 
		a2 = num_rect.z, // 2.0f, 
		b1 = num_rect.y, // -2.0f, 
		b2 = num_rect.w, // 2.0f,
		da = (a2 - a1) / width,
		db = (b2 - b1) / height;

	float a, b, c, c2, d;
	c = d = 0.0f;
	a = a1 + da * offset_x;
	b = b1 + db * offset_y;

	float4 color;

	int i_maxi = (int)maxi;
	int i;
	unsigned char iColor;
	for (i = 0, iColor=0; i < i_maxi; ++i, ++iColor)
	{
		if (c * c + d * d > 4.0f)
		{
			color = palette[iColor];
			break;
		}
		c2 = 2 * c * d;
		c = c * c - d * d + a;
		d = c2 + b;
	}

	write_imagef(image, (int2)(offset_x, offset_y), color);
}
__kernel void image_mandelbrot_nopal(
	write_only image2d_t image, 
	__private read_only float a1, 
	__private read_only float b1,
	__private read_only float da, 
	__private read_only float db, 
	__private read_only int max_iter)
{

	int offset_x = get_global_id(0), 
		offset_y = get_global_id(1);

	uint width = get_image_width(image);
	uint height = get_image_height(image);

	float a, b;

	a = a1 + offset_x * da;
	b = b1 + offset_y * db;

	float c, d;
	c = d = 0.0f;

	float4 color = 1.0f;
	int i;
	uchar uc;

	for (i = 0, c = 0; i < max_iter; ++i, ++uc)
	{
		float c2 = c * c;
		float d2 = d * d;
		if (c2 + d2 > 4.0f)
		{
			color = (float4)(uc / 128.0f, uc / 255.0f, uc / 64.0f, 1.0f); //palette[iColor];
			break;
		}
		float _c2 = 2 * c * d;
		c = c2 - d2 + a;
		d = _c2 + b;
	}

	write_imagef(image, (int2)(offset_x, offset_y), color);
}
__kernel void image_juliasin(write_only image2d_t image, read_only float2 pt, read_only float4 num_rect, read_only float maxi, __global __read_only float4 *palette)
{
	int offset_x = get_global_id(0), 
		offset_y = get_global_id(1);

	uint width = get_image_width(image);
	uint height = get_image_height(image);

	float dx = (num_rect.z - num_rect.x) / width;
	float dy = (num_rect.w - num_rect.y) / height;

	float x0 = num_rect.x + dx * offset_x;
	float y0 = num_rect.y + dy * offset_y;
	float x = x0;
	float y = y0;

	float4 color;

	int i_maxi = (int)maxi;
	int i;
	unsigned char iColor;
	float a = pt.x;
	float b = pt.y;
	for (i = 0, iColor=0; i < i_maxi; ++i, ++iColor)
	{

/*		float x1 = 2.967f * (sin(x) * (exp(y) + exp(-y)) / 2.0f);
		float y1 = 2.967f * (cos(x) * (exp(y) - exp(-y)) / 2.0f);
		*/
		float coshy = (exp(y) + exp(-y)) / 2.0f;
		float sinhy = (exp(y) - exp(-y)) / 2.0f;
		float x1 = a * sin(x) * coshy - b * cos(x) * sinhy;
		float y1 = b * sin(x) * coshy + a * cos(x) * sinhy;
		if (fabs(y1) > 50.0f)
		{ 
			break;
		}
		x = x1;
		y = y1;
	}

	color = palette[iColor];

	write_imagef(image, (int2)(offset_x, offset_y), color);
}

__kernel void image_juliacos(write_only image2d_t image, read_only float2 pt, read_only float4 num_rect, read_only float maxi, __global __read_only float4 *palette)
{
	int offset_x = get_global_id(0), 
		offset_y = get_global_id(1);

	uint width = get_image_width(image);
	uint height = get_image_height(image);

	float dx = (num_rect.z - num_rect.x) / width;
	float dy = (num_rect.w - num_rect.y) / height;

	float x0 = num_rect.x + dx * offset_x;
	float y0 = num_rect.y + dy * offset_y;
	float x = x0;
	float y = y0;

	float4 color;

	int i_maxi = (int)maxi;
	int i;
	unsigned char iColor;
	for (i = 0, iColor=0; i < i_maxi; ++i, ++iColor)
	{
		float x1 = pt.x * (cos(x) * (exp(y) + exp(-y)) / 2.0f);
		float y1 = pt.x * (-sin(x) * (exp(y) - exp(-y)) / 2.0f);
		if (fabs(y1) > 50.0f)
		{ 
			break;
		}
		x = x1;
		y = y1;
	}

	color = palette[iColor];

	write_imagef(image, (int2)(offset_x, offset_y), color);
}
__kernel void image_juliaexp(write_only image2d_t image, read_only float2 pt, read_only float4 num_rect, read_only float maxi, __global __read_only float4 *palette)
{
	int offset_x = get_global_id(0), 
		offset_y = get_global_id(1);

	uint width = get_image_width(image);
	uint height = get_image_height(image);

	float dx = (num_rect.z - num_rect.x) / width;
	float dy = (num_rect.w - num_rect.y) / height;

	float x0 = num_rect.x + dx * offset_x;
	float y0 = num_rect.y + dy * offset_y;
	float x = x0;
	float y = y0;

	float4 color;

	int i_maxi = (int)maxi;
	int i;
	unsigned char iColor;
	for (i = 0, iColor=0; i < i_maxi; ++i, ++iColor)
	{
//		float x1 = pt.x * (exp(x) * cos(y));
//		float y1 = pt.x * (exp(x) * sin(y));
		float x1 = exp(x) * (pt.x * cos(y) - pt.y * sin(y));
		float y1 = exp(x) * (pt.y * cos(y) + pt.x * sin(y));
		if (fabs(y1) > 50.0f)
		{ 
			break;
		}
		x = x1;
		y = y1;
	}

	color = palette[iColor];

	write_imagef(image, (int2)(offset_x, offset_y), color);
}
