constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void image_mandelbrot(write_only image2d_t image, read_only float4 num_rect, read_only float maxi, __global __read_only float4 *palette)
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