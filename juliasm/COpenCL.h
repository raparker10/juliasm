#pragma once
//
//  COpenCL.h
//
// Implements COpenCL which calculate a Mandelbrot Set and Julia Set
// using either the CPU or GPU OpenCL resources.
//

// OpenCL platform constants
#define MAX_PLATFORMS	32
#define MAX_PLATFORM_NAME_LEN	128

// OpenCL device constants
#define MAX_DEVICES		32
#define MAX_DEVICE_NAME_LEN	MAX_PLATFORM_NAME_LEN

// OpenCL Kernel and Program constants
#define MAX_KERNELS		32
#define MAX_LKERNEL_NAME_LEN	128
#define KERNEL_ATTRIBUTE_LEN 128
#define BUILD_LOG_LEN (1024 * 10)

// OpenCL Image constants
#define MAX_IMAGES	16

//
// COpenCL
//

class COpenCL {
private:
	//
	// availability
	//
	__declspec(align(32)) LONG m_iPlatformsReady;
	__declspec(align(32)) LONG m_iDevicesReady;
	__declspec(align(32)) LONG m_iKernelsReady;
	__declspec(align(32)) LONG m_iBuffersReady;

protected:

	//
	// availability
	//
	inline void put_PlatformsReady(bool bReady) {
		m_iPlatformsReady = (bReady) ? 1 : 0;
	}
	inline void put_DevicesReady(bool bReady) {
		assert(get_PlatformsReady());
		m_iDevicesReady = (bReady) ? 1 : 0;
	}
	inline void put_KernelsReady(bool bReady) {
		assert(get_DevicesReady());
		m_iKernelsReady = (bReady) ? 1 : 0;
	}
	inline void put_BuffersReady(bool bReady) {
//		assert(get_KernelsReady());
		m_iBuffersReady = (bReady) ? 1 : 0;
	}


	//
	// Callback information
	//
	HWND m_hWndCallback;
	int m_wmIDCallback;  // the wParam of the WM_COMMAND message
	//
	// platform information
	//

	int m_iCurrentPlatformIndex;		// which OpenCL platform is currently being used
	int m_iNumberPlatforms;				// the total number of OpenCL platforms
	cl_platform_id m_PlatformID[MAX_PLATFORMS];	// list of OpenCL platform IDs
	char m_szPlatformName[MAX_PLATFORMS][MAX_PLATFORM_NAME_LEN];	// list of OpenCL platfor names

	//
	// device information
	//

	int m_iCurrentDeviceIndex[MAX_KERNELS];			// current OpenCL device.  this is an index into the following arrays
	cl_uint m_iNumberDevices;			// the total number of OpenCL devices for the current platform
	cl_device_id m_DeviceID[MAX_DEVICES]; // OpenCL device IDs for the current platform
	char m_szDeviceName[MAX_DEVICES][MAX_DEVICE_NAME_LEN];	// OpenCL device names for the current platform
	cl_device_type m_DeviceType[MAX_DEVICES];	// OpenCL device types for the current platform (eg CPU, GPU, ...)

	//
	// program information
	//
	char m_szProgramPath[MAX_PATH + 1];		// path of the program currently being used
	char *m_szProgramText[1];				// text of the program currently being used
	size_t m_iProgramLength[1];				// current program text length

	//
	// kernel information
	//
	cl_kernel m_Kernel[MAX_KERNELS];		// kernel handles for alll kernels in the current program
	cl_uint m_iNumberKernels;				// total number of kernels currenty available
	char m_szKernelName[MAX_KERNELS][MAX_LKERNEL_NAME_LEN];		// kernel names in the current program
	char m_KernelAttributes[MAX_KERNELS][KERNEL_ATTRIBUTE_LEN];	// kernel attributes in the current program
	cl_uint m_KernelNumberArgs[MAX_KERNELS];	// nuber of arguments for each kernel in the current program

	cl_command_queue m_CommandQueue[MAX_DEVICES];	// command queue for the current program

	//
	// build error message information
	//
	cl_int m_iLastBuildStatus[MAX_DEVICES];
	char *m_pszLastBuildMessage[MAX_DEVICES];
	bool put_LastBuildMessage(int iDeviceIndex, char *szBuildMessage)
	{
		// check for erroneous input
		assert(szBuildMessage[iDeviceIndex] != NULL);

		// free any current error message
		if (m_pszLastBuildMessage[iDeviceIndex] != NULL)
		{
			free(m_pszLastBuildMessage[iDeviceIndex]);
			m_pszLastBuildMessage[iDeviceIndex] = NULL;
		}
		
		// return error if the input message is NULL
		if (szBuildMessage == NULL)
			return false;

		// return true if the input message is empty
		if (lstrlen(szBuildMessage) == 0)
			return true;

		m_pszLastBuildMessage[iDeviceIndex] = _strdup(szBuildMessage);
		assert(m_pszLastBuildMessage[iDeviceIndex] != NULL);
		return m_pszLastBuildMessage[iDeviceIndex] != NULL;
	}

	bool Initialize(void);	// initializes the OpenCL program
	int BuildPlatformList(void);				// gets the platform list
	int BuildDeviceList(int iPlatformIndex);	// gets the device list 

public:
	COpenCL();
	~COpenCL();

	//
	// availability
	//
	inline bool get_PlatformsReady(void) const {
		return 0 != m_iPlatformsReady;
	}
	inline bool get_DevicesReady(void) const {
		return get_PlatformsReady() && (0 != m_iDevicesReady);
	}
	inline bool get_KernelsReady(void) const {
		return get_DevicesReady() && (0 != m_iKernelsReady);
	}
	inline bool get_BuffersReady(void) const {
		return get_KernelsReady() && (0 != m_iBuffersReady);
	}


	//
	// platform functions
	//

	// returns the number of installed OpenCL platforms
	inline int get_PlatformCount(void) const
	{
		assert(get_PlatformsReady());
		return m_iNumberPlatforms;
	}

	// returns the OpenCL platform ID of the indicated platform
	inline cl_platform_id get_PlatformID(int iIndex) {
		assert(get_PlatformsReady());
		if (iIndex < get_PlatformCount())
			return m_PlatformID[iIndex];
		return (cl_platform_id)CL_INVALID_PLATFORM;
	}

	// gets the index into the platform arrays of the pprovided platform id
	int get_PlatformIndex(cl_platform_id id)
	{
		assert(get_PlatformsReady());
		for (int i = 0; i < get_PlatformCount(); ++i)
		{
			if (id == m_PlatformID[i])
				return i;
		}
		return -1;	// return error
	}

	// uses the platform indicated by the platform ID
	bool UsePlatformByID(cl_platform_id ID);

	// uses the OpenCL platform indicated by the platform index
	inline bool UsePlatformByIndex(int iIndex) {
		assert(get_PlatformsReady());
		if (iIndex >=0 && iIndex < get_PlatformCount())
		{
			cl_platform_id id = get_PlatformID(iIndex);
			if (id == (cl_platform_id)CL_INVALID_PLATFORM)
				return false;
			return UsePlatformByID(id);
		}
		return false;
	}

	// uses the default platform (simply uses the first one)
	inline bool UsePlatformDefault(void) {
		assert(get_PlatformsReady());
		return UsePlatformByIndex(0);
	}

	//
	// device functions
	//

	// returns the number of OpenCL devices available with the current platform
	inline int get_DeviceCount(void) const
	{
		assert(get_DevicesReady());
		return m_iNumberDevices;
	}

	// gets the OpenCL device ID from the provided array index
	inline cl_device_id get_DeviceID(int iIndex) {
		assert(get_DevicesReady());
		if (iIndex >= 0 && iIndex < get_DeviceCount())
			return m_DeviceID[iIndex];
		return (cl_device_id)CL_INVALID_DEVICE;	// return error if not found
	}

	// get the array index from the provided OpenCL device id
	int get_DeviceIndex(cl_device_id id)
	{
		assert(get_DevicesReady());
		for (int i = 0; i < get_DeviceCount(); ++i)
		{
			if (id == m_DeviceID[i])
				return i;
		}
		return -1;	// return error (-1) if not found
	}
	// returns the first device that matches the requested device type
	int get_DeviceIndexByType(cl_device_type device_type)
	{
		assert(get_DevicesReady());
		for (int i = 0; i < get_DeviceCount(); ++i)
		{
			if (device_type == m_DeviceType[i])
				return i;
		}
		return -1;	// return error (-1) if not found
	}

	// selects a specific OpenCL device for use by its OpenCL device ID
	bool UseDeviceByID(int iKernelIndex, cl_device_id ID);

	// selects a specific OpenCL device for use by its array index
	inline bool UseDeviceByIndex(int iKernelIndex, int iIndex) {
		assert(get_DevicesReady());
		return UseDeviceByID(iKernelIndex, get_DeviceID(iIndex));
	}

	// selects the default OpenCL device ID, in this case it just selects the first GPU device
	inline bool UseDeviceDefault(int iKernelIndex) {
		assert(get_DevicesReady());
		return UseDeviceByType(iKernelIndex, CL_DEVICE_TYPE_CPU);
	}

	// selects the first device of the specified type (e.g. CPU, GPU, ...)
	bool UseDeviceByType(int iKernelIndex, cl_device_type device_type);

	cl_device_type get_DeviceType(int iIndex)
	{
		assert(get_DevicesReady());
		if (iIndex >= 0 && iIndex < get_DeviceCount())
		{
			return m_DeviceType[iIndex];
		}
		return (cl_device_type)CL_INVALID_DEVICE_TYPE;
	}
	cl_device_type get_CurrentDeviceTypeByKernel(int iKernelIndex)
	{
		assert(get_DevicesReady());
		assert(iKernelIndex >= 0 && iKernelIndex <= get_KernelCount());

		if (m_iCurrentDeviceIndex[iKernelIndex] >= 0 && m_iCurrentDeviceIndex[iKernelIndex] < get_DeviceCount())
		{
			return m_DeviceType[m_iCurrentDeviceIndex[iKernelIndex]];
		}
		return (cl_device_type)CL_INVALID_DEVICE_TYPE;
	}
	cl_device_type get_DeviceType(cl_device_id DeviceID)
	{
		assert(get_DevicesReady());
		int iIndex = get_DeviceIndex(DeviceID);
		if (iIndex >= 0)
		{
			return get_DeviceType(iIndex);
		}
		return (cl_device_type)CL_INVALID_DEVICE_TYPE;
	}

	//
	// program functions
	//
	cl_context m_Context;	// the OpenCL context
	cl_program m_Program;	// the OpenCL program

	// loads an OpenCL program from the specified file.  It will load if from the program's current path if indicated
	bool LoadProgram(char * szProgramPath, bool bUseProgramPath = true);

	static DWORD WINAPI PrepareProgramAsyncWorker(void* pOCL);
	bool PrepareProgram(HWND hWndCallback, int wmIDCallback);			// prepare the current OpenCL program's environment for use
	virtual bool PrepareProgramBuffers(void) { return false; }	// prepare program buffers for use
	bool CleanupProgram(void);			// cleanup the current OpoenCL program environment
	virtual bool CleanupProgramBuffers(void);	// delete OpenCL buffers
	bool ExecuteProgram(int iDeviceIndex, int iKernel, int iImageIndex, cl_int *pError);	// execute the current OpenCL pprogram

	//
	// Kernel functions
	//
	inline int get_KernelCount(void) const {
		assert(get_KernelsReady());
		return m_iNumberKernels;
	}

	//
	// error message functions
	//
	inline const char * get_LastBuildMessage(int iDeviceIndex) {
		assert(get_DevicesReady());
		return m_pszLastBuildMessage[iDeviceIndex];
	}
	inline bool get_LastBuildMessage(int iDeviceIndex, char *szMessageBuffer, size_t iBufSize) {
		assert(get_DevicesReady());

		// check for erroneous input
		assert(szMessageBuffer != NULL);
		assert(iBufSize > 0);
		
		if (szMessageBuffer == NULL || iBufSize <= 0)
			return false;

		// initialize the buffer
		szMessageBuffer[0] = 0;
		if (m_pszLastBuildMessage != NULL)
			strcpy_s(szMessageBuffer, iBufSize, m_pszLastBuildMessage[iDeviceIndex]);

		return true;
	}
	inline cl_int get_LastBuildStatus(int iDeviceIndex) const { 
		assert(get_DevicesReady());
		return m_iLastBuildStatus[iDeviceIndex]; 
	}
};

class COpenCLImage : public COpenCL
{
protected:
	int m_iNumberPaletteColors;		// number of palette colors
	__declspec(align(64)) cl_float4 m_Palette[CPalette::MAX_COLORS];	// palette in float format
	__declspec(align(64)) cl_mem m_PaletteBuffer;	// OpenCL buffer object for holding the palette

	//
	// kernel information that should be in a derived class
	//
	cl_mem m_Image[MAX_IMAGES];					// OpenCL memory object for storing images
	cl_image_format m_ImageFormat[MAX_IMAGES];	// OpenCL image format
	const size_t DEFAULT_IMAGE_SIZE = 1; //640;
	unsigned char *m_BitmapBits[MAX_IMAGES];	// pointer to the Windows bitmap.  Image data will be read to that location from the OpenCL buffer
	int m_iImageWidth[MAX_IMAGES],		// width of each bitmap
		m_iImageHeight[MAX_IMAGES];		// height of each bitmap



public:
	COpenCLImage();
	~COpenCLImage() {}
	// indicate where to place the Mandelbrot set image
	bool put_Bitmap(int iBitmapIndex, LPVOID lpvBitmap) 
	{
		assert(iBitmapIndex >= 0 && iBitmapIndex < MAX_IMAGES);

		if (iBitmapIndex < 0 || iBitmapIndex >= MAX_IMAGES)
		{
			return false;
		}
		m_BitmapBits[iBitmapIndex] = (unsigned char*)lpvBitmap;
		return true;
	}

	// changes the mandelbrot bitmap image size
	bool put_ImageSize(int iBitmapIndex, int width, int height);

	// scale the palette from 0-255 to 0.0f-1.0f for use with OpenCL
	void put_Palette(int iCount, unsigned char *pcRed, unsigned char *pcGreen, unsigned char *pcBlue)
	{
		for (int i = 0; i < iCount; ++i)
		{
			float fRed = pcRed[i] * 1.0f / 255.0f;
			float fGreen = pcGreen[i] * 1.0f / 255.0f;
			float fBlue = pcBlue[i] * 1.0f / 255.0f;

			m_Palette[i].s0 = fRed;
			m_Palette[i].s1 = fGreen;
			m_Palette[i].s2 = fBlue;
			m_Palette[i].s3 = 1.0f;
		}
	}
};

class COpenCLFrac : public COpenCLImage
{
protected:
	//
	// Mandelbrot Set calculation variables
	float m_ma1[MAX_KERNELS], 
		m_ma2[MAX_KERNELS], 
		m_mb1[MAX_KERNELS], 
		m_mb2[MAX_KERNELS];	// numeric bounding box of the image
	float m_fMaxIterations[MAX_KERNELS];			// maximum number of calculation iterations
	float m_const_c[MAX_KERNELS], 
		m_const_d[MAX_KERNELS];					// constant point for which a julia set is calculated
	cl_float4 m_NumericRect[MAX_KERNELS];		// the numeric bounding box for the current program (e.g. (a1, b1)-(a2, b2))
	cl_float2 m_Const[MAX_KERNELS];						// the Mandelbrot set constant point for which the Julia set is being calculated

	virtual bool PrepareProgramBuffers(void);	// prepare program buffers for use
	virtual bool CleanupProgramBuffers(void);

public:
	COpenCLFrac() {}
	~COpenCLFrac() {}
	// sets the numerical bounding box for the mandelbrot calculation
	void put_Boundary(int iKernelIndex, float a1, float b1, float a2, float b2)
	{
		assert(iKernelIndex >= 0 && iKernelIndex < MAX_KERNELS);
		if (iKernelIndex < 0 && iKernelIndex >= MAX_KERNELS)
			return;

		m_ma1[iKernelIndex] = a1;
		m_ma2[iKernelIndex] = a2;
		m_mb1[iKernelIndex] = b1;
		m_mb2[iKernelIndex] = b2;
	}

	void put_ConstPoint(int iKernelIndex, float const_c, float const_d)
	{
		assert(iKernelIndex >= 0 && iKernelIndex < MAX_KERNELS);
		if (iKernelIndex < 0 && iKernelIndex >= MAX_KERNELS)
			return;

		m_const_c[iKernelIndex] = const_c;
		m_const_d[iKernelIndex] = const_d;
		m_Const[iKernelIndex].s0 = const_c;
		m_Const[iKernelIndex].s1 = const_d;
	}

	// sets the maximum number of Mandelbrot set iterations
	inline void put_MaxIterations(int iIterations, int iKernelIndex = -1)
	{
		if (iKernelIndex == -1)
		{
			for (int i = 0; i < MAX_KERNELS; ++i)
			{
				{
					m_fMaxIterations[i] = (float)iIterations;
				}
			}
			return;
		}
		assert(iKernelIndex >= -1 && iKernelIndex < MAX_KERNELS);
		if (iKernelIndex < 0 || iKernelIndex >= MAX_KERNELS)
			return;

		m_fMaxIterations[iKernelIndex] = (float)iIterations;
	}
	bool ExecuteProgramMand(int iDeviceIndex, int iKernelIndex, int iImageIndex, cl_int *pError);	// execute the current OpenCL pprogram
	bool ExecuteProgramJulia(int iDeviceIndex, int iKernelIndex, int iImageIndex, cl_int *pError);	// execute the current OpenCL pprogram
};

/*
class COpenCLJulia : public COpenCLImage
{
protected:
	//
	// Mandelbrot Set calculation variables
	float m_ja1, m_ja2, m_jb1, m_jb2;	// numeric bounding box of the image
	float m_const_c, m_const_d;
	float m_fMaxIterations;			// maximum number of calculation iterations
	cl_float4 m_NumericRect;		// the numeric bounding box for the current program (e.g. (a1, b1)-(a2, b2))
	cl_float2 m_Const;				// the Mandelbrot set constant point for which the Julia set is being calculated

	virtual bool PrepareProgramBuffers(void);	// prepare program buffers for use
	virtual bool CleanupProgramBuffers(void);

public:
	COpenCLJulia() {}
	~COpenCLJulia() {}
	// sets the constant Mandelbrot Set point for which the Julia set is being caluclated
	void put_ConstPoint(float const_c, float const_d)
	{
		m_const_c = const_c;
		m_const_d = const_d;
		m_Const.s0 = m_const_c;
		m_Const.s1 = m_const_d;
	}
	// sets the numerical bounding box for the julia calculation
	void put_Boundary(float a1, float b1, float a2, float b2)
	{
		m_ja1 = a1;
		m_ja2 = a2;
		m_jb1 = b1;
		m_jb2 = b2;

		m_NumericRect.s0 = a1;
		m_NumericRect.s1 = b1;
		m_NumericRect.s2 = a2;
		m_NumericRect.s3 = b2;
	}

	// sets the maximum number of Mandelbrot set iterations
	inline void put_MaxIterations(int iIterations)
	{
		m_fMaxIterations = (float)iIterations;
	}
	bool ExecuteProgram(int iDeviceIndex, int iKernel, int iImageIndex, cl_int *pError);	// execute the current OpenCL pprogram


};
*/