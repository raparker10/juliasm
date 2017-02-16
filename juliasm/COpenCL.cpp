#include "stdafx.h"

COpenCLImage::COpenCLImage()
{
	for (int i = 0 ; i < MAX_IMAGES; ++i)
	{
		m_Image[i] = (cl_mem)CL_INVALID_MEM_OBJECT;
		m_iImageWidth[i] = m_iImageHeight[i] = 0;
	}
		m_iNumberPaletteColors = 0;
}

// object constructor
COpenCL::COpenCL()
{
	Initialize();
	BuildPlatformList();
}

// object destructor
COpenCL::~COpenCL()
{
}

// initializes the object
bool COpenCL::Initialize(void)
{
	int i;

	m_iPlatformsReady = 0;
	m_iKernelsReady = 0; // indicate that Kernels are not yet ready for execution
	m_iDevicesReady = 0;

	// initialzie platform variables
	m_iCurrentPlatformIndex = -1;		// which OpenCL platform is currently being used
	m_iNumberPlatforms = 0;				// the total number of OpenCL platforms
	for (i = 0; i < MAX_PLATFORMS; ++i)
	{
		m_PlatformID[i] = (cl_platform_id)CL_INVALID_PLATFORM;
	}
	memset(m_szPlatformName, 0, sizeof(m_szPlatformName));
	
	// initialize kernel variables
	for (i = 0; i < _countof(m_iCurrentDeviceIndex); ++i)
	{
		m_iCurrentDeviceIndex[i] = -1;

	}

	// initialize device variables
	m_iNumberDevices = 0;			// the total number of OpenCL devices for the current platform
	for (i = 0; i  < MAX_DEVICES; ++i)
	{
		m_DeviceID[i] = (cl_device_id)CL_INVALID_DEVICE; // OpenCL device IDs for the current platform
		m_DeviceType[i] = (cl_device_type)CL_INVALID_DEVICE_TYPE;	// OpenCL device types for the current platform (eg CPU, GPU, ...)
		// command queue variables
		m_CommandQueue[i] = (cl_command_queue)CL_INVALID_COMMAND_QUEUE;	// command queue for the current program
	}
	memset(m_szDeviceName, 0, sizeof(m_szDeviceName));

	// program variables
	memset(m_szProgramPath, 0, sizeof(m_szProgramPath));

	// kernel variables
	for (i = 0; i < MAX_KERNELS; ++i)
	{
		m_Kernel[i] = (cl_kernel)CL_INVALID_KERNEL;		
		m_KernelNumberArgs[i] = 0;
	}
	m_iNumberKernels = 0;				
	memset(m_KernelAttributes, 0, sizeof(m_KernelAttributes));
	memset(m_szKernelName, 0, sizeof(m_szKernelName));	


	return true;
}

/**********
/
/ Platform and Device functions
/
**********/



// builds the list of OpenCL platforms and then returns the number detected
int COpenCL::BuildPlatformList(void)
{
	cl_int err;
	cl_uint num_platforms;
	unsigned int u;

	// get the number of OpenCL platforms
	err = clGetPlatformIDs(1, NULL, &num_platforms);
	if (err != CL_SUCCESS)
		return 0;

	// make sure we don't get too many platforms (very unlikely)
	if (num_platforms > MAX_PLATFORMS)
	{
		num_platforms = MAX_PLATFORMS;
	}

	// get the list of OpenCL platform IDs
	err = clGetPlatformIDs(num_platforms, m_PlatformID, NULL);
	if (err != CL_SUCCESS)
		return 0;

	// get the OpenCL platform names
	for (u = 0; u < num_platforms; ++u)
	{
		err = clGetPlatformInfo(m_PlatformID[u], CL_PLATFORM_NAME, MAX_PLATFORM_NAME_LEN, m_szPlatformName[u], NULL);
		if (err != CL_SUCCESS)
			return 0;
	}

	// save (and then return) the platform count
	m_iNumberPlatforms = num_platforms;
	put_PlatformsReady(num_platforms > 0);

	return m_iNumberPlatforms;
}

// selects the specified OpenCL platform for use`
bool COpenCL::UsePlatformByID(cl_platform_id platform_id)
{
	assert(get_PlatformsReady());
	int iIndex = get_PlatformIndex(platform_id);
	if (iIndex < 0)
		return false;

	m_iCurrentPlatformIndex = iIndex;

	// return success if device information can be obtained
	return BuildDeviceList(m_iCurrentPlatformIndex) > 0;
}

// builds a list of all devices on the current platform
int COpenCL::BuildDeviceList(int iPlatformIndex)
{
	cl_int error;
	size_t iSize;

	assert(get_PlatformsReady());

	// bail if the platform is not valid
	if (iPlatformIndex < 0 || iPlatformIndex >= m_iNumberPlatforms)
		return 0; // return no devices for an unknown platform

	// get the device information for the specified platform
	cl_uint num_devices;
	error = clGetDeviceIDs(m_PlatformID[iPlatformIndex], CL_DEVICE_TYPE_ALL, sizeof(m_DeviceID) / sizeof(m_DeviceID[0]), m_DeviceID, &num_devices);
	if (error != CL_SUCCESS)
		return 0;

	// get the device names and types so that they can be displayed
	for (unsigned int u = 0; u < num_devices; ++u)
	{
		// get the device name
		error = clGetDeviceInfo(m_DeviceID[u], CL_DEVICE_NAME, sizeof(m_szDeviceName[0]), m_szDeviceName[u], &iSize);
		if (error != CL_SUCCESS)
			return 0;

		// get the device type
		error = clGetDeviceInfo(m_DeviceID[u], CL_DEVICE_TYPE, sizeof(m_DeviceType[0]), &m_DeviceType[u], &iSize);
		if (error != CL_SUCCESS)
			return 0;
	}

	// return the number of devices found
	m_iNumberDevices = num_devices;
	put_DevicesReady(num_devices > 0);
	return m_iNumberDevices;

}

// select an OpenCL device for use by it's OpenCL device ID
bool COpenCL::UseDeviceByID(int iKernelIndex, cl_device_id device_id)
{
	assert(get_DevicesReady());
	int iIndex = get_DeviceIndex(device_id);
	if (iIndex < 0)
		return false;

	m_iCurrentDeviceIndex[iKernelIndex] = iIndex;
	return true;
}

// select an OpenCL device for use by it's OpenCL device type 
bool COpenCL::UseDeviceByType(int iKernelIndex, cl_device_type device_type)
{
	assert(get_DevicesReady());

	for (int i = 0; i < get_DeviceCount(); ++i)
	{
		if (m_DeviceType[i] == device_type)
		{
			return UseDeviceByIndex(iKernelIndex, i);
		}
	}
	return false;
}

//  Load an OpenCL program file and prepare it for use
#pragma warning(disable:4996)// RAP: get rid of insecure function warning
bool COpenCL::LoadProgram(char * szProgramPath, bool bUseProgramPath)
{
	char szPath[_MAX_PATH + 1];
	char szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
	char szPathName[_MAX_PATH + 1];

	szPathName[0] = 0; // always null-terminate the return path


	// if the program path is to be used, get it and create the path
	if (bUseProgramPath)
	{
		// get the current directory
		if (0 == GetCurrentDirectoryA(_countof(szPath), szPath))
			return false;

		// get the directory and program name components
		_splitpath_s(szPath, szDrive, _countof(szDrive), szDir, _countof(szDir), NULL, 0, NULL, 0);
		_splitpath_s(szProgramPath, NULL, 0, NULL, 0, szFName, _countof(szFName), szExt, _countof(szExt));

		// make the new path
		_makepath_s(szPathName, _countof(szPathName), szDrive, szDir, szFName, szExt);
		strcpy_s(m_szProgramPath, _countof(m_szProgramPath), szPathName);
	}
	else
	{
		strcpy_s(m_szProgramPath, _countof(m_szProgramPath), szProgramPath);
	}


	//
	// load the program
	//
	// free any existing memory
	if (m_szProgramText != NULL)
	{
		free(m_szProgramText[0]);
		m_szProgramText[0] = NULL;
	}

	// open the file
	FILE *fh = fopen(m_szProgramPath, "r");
	assert(fh != NULL);
	if (fh == NULL)
	{
		MessageBox(NULL, szProgramPath, "Unable to open OpcnCL file", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	// get the file length
	fseek(fh, 0, SEEK_END);
	m_iProgramLength[0] = ftell(fh);
	fseek(fh, 0, SEEK_SET);

	// load the file
	m_szProgramText[0] = (char*)calloc(1, m_iProgramLength[0] + 1);
	size_t len;
	if (m_szProgramText[0] != NULL)
	{
		len = fread(m_szProgramText[0], 1, m_iProgramLength[0] , fh);
//		m_szProgramText[0][m_iProgramLength[0] - 1] = 0;
		m_szProgramText[0][len - 1] = 0;
		m_iProgramLength[0] = len;
	}
	fclose(fh);
	return m_szProgramText[0] != NULL;
}

DWORD WINAPI COpenCL::PrepareProgramAsyncWorker(void* pOCL)
{
	COpenCL *ocl = (COpenCL*)pOCL;

	// first, cleanup any existing program
	ocl->CleanupProgram();

	cl_int error;
	bool bAnyBuildFailed = false;

	// create a Context for the Program
	ocl->m_Context = clCreateContext(NULL, ocl->get_DeviceCount(), ocl->m_DeviceID, NULL, NULL, &error);
	if (error == CL_SUCCESS)
	{
		ocl->m_Program = clCreateProgramWithSource(ocl->m_Context, 1, (const char **)ocl->m_szProgramText, ocl->m_iProgramLength, &error);
		if (error == CL_SUCCESS)
		{
			// build the program for the CURRENT device
//			cl_int build_error = clBuildProgram(m_Program, 1, &m_DeviceID[m_iCurrentDeviceIndex],	"-s \"C:/Users/Robert/Documents/Visual Studio 2013/Projects/juliasm/fractals.cl\"", NULL, NULL);
			//
			// build the program separately for each device.  This enables using different compile options
			//	for different device type (e.g. double with the CPU device)
			//
			for (int dev = 0; dev < ocl->get_DeviceCount(); ++dev)
			{
				cl_int build_error = clBuildProgram(ocl->m_Program, 1, &ocl->m_DeviceID[dev],	NULL, NULL, NULL);
				ocl->m_iLastBuildStatus[dev] = build_error;

				// get the build log for the CURRENT device
				size_t size_ret;
//				int d = m_iCurrentDeviceIndex;

				// get the build log
				error = clGetProgramBuildInfo(ocl->m_Program, ocl->m_DeviceID[dev], CL_PROGRAM_BUILD_LOG, 0, NULL, &size_ret);
				if (error == CL_SUCCESS)
				{
					char *ptr = (char*)calloc(size_ret + 1, 1);
					if (ptr != NULL)
					{
						error = clGetProgramBuildInfo(ocl->m_Program, ocl->m_DeviceID[dev], CL_PROGRAM_BUILD_LOG, size_ret, ptr, &size_ret);
						if (size_ret > 0)
						{
							ocl->put_LastBuildMessage(dev, ptr);
						}
						else
						{
							ocl->put_LastBuildMessage(dev, "");
						}
						free(ptr);
					}
					ocl->m_CommandQueue[dev] = clCreateCommandQueue(ocl->m_Context, ocl->m_DeviceID[dev], 0, &error);

				}
				bAnyBuildFailed = bAnyBuildFailed || (build_error != CL_SUCCESS);
			}

			if (bAnyBuildFailed == false)
			{
				error = clCreateKernelsInProgram(ocl->m_Program, MAX_KERNELS, ocl->m_Kernel, &ocl->m_iNumberKernels);
				if (error == CL_SUCCESS)
				{
					int i;
	
					ocl->put_KernelsReady(true);

					// get information for each kernel
					for (i = 0; i < ocl->get_KernelCount(); ++i)
					{

						size_t ret_size;

						clGetKernelInfo(ocl->m_Kernel[i], CL_KERNEL_FUNCTION_NAME, sizeof(ocl->m_szKernelName[i]), &ocl->m_szKernelName[i], &ret_size);
						clGetKernelInfo(ocl->m_Kernel[i], CL_KERNEL_NUM_ARGS, sizeof(ocl->m_KernelNumberArgs[i]), &ocl->m_KernelNumberArgs[i], &ret_size);
						clGetKernelInfo(ocl->m_Kernel[i], CL_KERNEL_ATTRIBUTES, sizeof(ocl->m_KernelAttributes[i]), &ocl->m_KernelAttributes[i], &ret_size);
					}


				}
				ocl->put_KernelsReady(true);
			}
		}
		int retval = ocl->PrepareProgramBuffers() ? 1 : 0;
		PostMessage(ocl->m_hWndCallback, WM_COMMAND, ocl->m_wmIDCallback, retval);
		return retval;
	}
	PostMessage(ocl->m_hWndCallback, WM_COMMAND, ocl->m_wmIDCallback, 0);
	return 0; // false
}

// prepare the OpenCL program for use
bool COpenCL::PrepareProgram(HWND hWndCallback, int wmIDCallback)
{
	assert(get_PlatformsReady());
	assert(get_DevicesReady());

	m_hWndCallback = hWndCallback;
	m_wmIDCallback = wmIDCallback;

	CreateThread(NULL, 1024 * 10, PrepareProgramAsyncWorker, (LPVOID)this, 0, NULL);
	return true;

	// first, cleanup any existing program
	CleanupProgram();

	cl_int error;
	bool bAnyBuildFailed = false;

	// create a Context for the Program
	m_Context = clCreateContext(NULL, m_iNumberDevices, m_DeviceID, NULL, NULL, &error);
	if (error == CL_SUCCESS)
	{
		m_Program = clCreateProgramWithSource(m_Context, 1, (const char **)m_szProgramText, m_iProgramLength, &error);
		if (error == CL_SUCCESS)
		{
			// build the program for the CURRENT device
//			cl_int build_error = clBuildProgram(m_Program, 1, &m_DeviceID[m_iCurrentDeviceIndex],	"-s \"C:/Users/Robert/Documents/Visual Studio 2013/Projects/juliasm/fractals.cl\"", NULL, NULL);
			//
			// build the program separately for each device.  This enables using different compile options
			//	for different device type (e.g. double with the CPU device)
			//
			for (int dev = 0; dev < this->m_iNumberDevices; ++dev)
			{
				cl_int build_error = clBuildProgram(m_Program, 1, &m_DeviceID[dev],	NULL, NULL, NULL);
				m_iLastBuildStatus[dev] = build_error;

				// get the build log for the CURRENT device
				size_t size_ret;
//				int d = m_iCurrentDeviceIndex;

				// get the build log
				error = clGetProgramBuildInfo(m_Program, m_DeviceID[dev], CL_PROGRAM_BUILD_LOG, 0, NULL, &size_ret);
				if (error == CL_SUCCESS)
				{
					char *ptr = (char*)calloc(size_ret + 1, 1);
					if (ptr != NULL)
					{
						error = clGetProgramBuildInfo(m_Program, m_DeviceID[dev], CL_PROGRAM_BUILD_LOG, size_ret, ptr, &size_ret);
						if (size_ret > 0)
						{
							put_LastBuildMessage(dev, ptr);
						}
						else
						{
							put_LastBuildMessage(dev, "");
						}
						free(ptr);
					}
					m_CommandQueue[dev] = clCreateCommandQueue(m_Context, m_DeviceID[dev], 0, &error);

				}
				bAnyBuildFailed = bAnyBuildFailed || (build_error != CL_SUCCESS);
			}

			if (bAnyBuildFailed == false)
			{
				error = clCreateKernelsInProgram(m_Program, MAX_KERNELS, m_Kernel, &m_iNumberKernels);
				if (error == CL_SUCCESS)
				{
					int i;

					// get information for each kernel
					for (i = 0; i < m_iNumberKernels; ++i)
					{

						size_t ret_size;

						clGetKernelInfo(m_Kernel[i], CL_KERNEL_FUNCTION_NAME, sizeof(m_szKernelName[i]), &m_szKernelName[i], &ret_size);
						clGetKernelInfo(m_Kernel[i], CL_KERNEL_NUM_ARGS, sizeof(m_KernelNumberArgs[i]), &m_KernelNumberArgs[i], &ret_size);
						clGetKernelInfo(m_Kernel[i], CL_KERNEL_ATTRIBUTES, sizeof(m_KernelAttributes[i]), &m_KernelAttributes[i], &ret_size);
					}


				}
				put_KernelsReady(true);
			}
		}
		return PrepareProgramBuffers();
	}
	return false;
}
bool COpenCLImage::put_ImageSize(int iBitmapIndex, int width, int height)
{
	cl_int error;

	assert(get_BuffersReady());

	if (false == get_BuffersReady())
	{
		return false;
	}

		assert(iBitmapIndex >= 0 && iBitmapIndex < MAX_IMAGES);

		if (iBitmapIndex < 0 || iBitmapIndex >= MAX_IMAGES)
		{
			return false;
		}

	// release n existing mandelbrot image
	if (m_Image[iBitmapIndex] != (cl_mem)CL_INVALID_MEM_OBJECT)
	{
		clReleaseMemObject(m_Image[iBitmapIndex]);
		m_Image[iBitmapIndex] = (cl_mem)CL_INVALID_MEM_OBJECT;
	}

	m_iImageHeight[iBitmapIndex] = height;
	m_iImageWidth[iBitmapIndex] = width;

	if (width == 0 || height == 0)
		return true; // returns true, but there is no image

	m_ImageFormat[iBitmapIndex].image_channel_data_type = CL_UNORM_INT8;
	m_ImageFormat[iBitmapIndex].image_channel_order = CL_RGBA;

	m_Image[iBitmapIndex] = clCreateImage2D(m_Context, CL_MEM_WRITE_ONLY, &m_ImageFormat[iBitmapIndex], width, height, 0, NULL, &error);
	if (error != CL_SUCCESS)
	{
		return false;
	}


	return error == CL_SUCCESS;
}

bool COpenCLFrac::PrepareProgramBuffers(void)
{
	cl_int error;
	int i;

	assert(get_KernelsReady());

	for (i = 0; i < MAX_IMAGES; ++i)
	{
		m_ImageFormat[i].image_channel_data_type = CL_UNORM_INT8;
		m_ImageFormat[i].image_channel_order = CL_RGBA;

		if (m_Image[i] != (cl_mem)CL_INVALID_MEM_OBJECT)
		{
			clReleaseMemObject(m_Image[i]);
			m_Image[i] = (cl_mem)CL_INVALID_MEM_OBJECT;
		}
		m_Image[i] = clCreateImage2D(m_Context, CL_MEM_WRITE_ONLY, &m_ImageFormat[i], DEFAULT_IMAGE_SIZE, DEFAULT_IMAGE_SIZE, 0, NULL, &error);
		if (error != CL_SUCCESS)
			return false;
	}

	m_PaletteBuffer = clCreateBuffer(m_Context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, sizeof(m_Palette), m_Palette, &error);
	if (error != CL_SUCCESS)
		return false;

	// RAP: for now, all the images use the same palette.  this will evenually have to change
	for (i = 0; i < m_iNumberDevices; ++i)
	{
		error = clEnqueueWriteBuffer(m_CommandQueue[i], m_PaletteBuffer, CL_TRUE, 0, sizeof(m_PaletteBuffer), m_PaletteBuffer, 0, NULL, NULL);
	}

	put_BuffersReady(true);

	return true;
}
bool COpenCL::CleanupProgram(void)
{
	CleanupProgramBuffers();
	put_BuffersReady(false);

	cl_int error;
	int i;

	if (get_DevicesReady())
	{
		for (i = 0; i < get_DeviceCount(); ++i)
		{
			if (m_CommandQueue[i] != (cl_command_queue)CL_INVALID_COMMAND_QUEUE)
			{
				error = clReleaseCommandQueue(m_CommandQueue[i]);
				m_CommandQueue[i] = (cl_command_queue)CL_INVALID_COMMAND_QUEUE;
			}
		}
	}

	if (get_KernelsReady())
	{
		for (i = 0; i < get_KernelCount(); ++i)
		{
			if (m_Kernel[i] != (cl_kernel)CL_INVALID_KERNEL)
			{
				error = clReleaseKernel(m_Kernel[i]);
				m_Kernel[i] = (cl_kernel)CL_INVALID_KERNEL;
				m_iNumberKernels = 0;
			}
		}
		put_KernelsReady(false);
	}

	if (m_Program != (cl_program)CL_INVALID_PROGRAM)
	{
		error = clReleaseProgram(m_Program);
		m_Program = (cl_program)CL_INVALID_PROGRAM;
	}

	if (m_Context != (cl_context)CL_INVALID_CONTEXT)
	{
		error = clReleaseContext(m_Context);
		m_Context = (cl_context)CL_INVALID_CONTEXT;
	}


	return true;
}
bool COpenCL::CleanupProgramBuffers(void)
{
	put_BuffersReady(false);

	return true;
}
bool COpenCLFrac::CleanupProgramBuffers(void)
{

	return true;
}
bool COpenCLFrac::ExecuteProgramMand(int iDeviceIndex, int iKernelIndex, int iImageIndex, cl_int *pError)
{
	assert(get_BuffersReady());
	cl_int error;

	m_NumericRect[iKernelIndex].s0 = m_ma1[iKernelIndex];
	m_NumericRect[iKernelIndex].s1 = m_mb1[iKernelIndex];
	m_NumericRect[iKernelIndex].s2 = m_ma2[iKernelIndex];
	m_NumericRect[iKernelIndex].s3 = m_mb2[iKernelIndex];

	if (m_Kernel[iKernelIndex] == (cl_kernel)CL_INVALID_KERNEL)
		return false;

	error = clSetKernelArg(m_Kernel[iKernelIndex], 3, sizeof(m_PaletteBuffer), &m_PaletteBuffer);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[iKernelIndex], 2, sizeof(m_fMaxIterations[iKernelIndex]), &m_fMaxIterations[iKernelIndex]);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[iKernelIndex], 1, sizeof(m_NumericRect[iKernelIndex]), &m_NumericRect[iKernelIndex]);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[iKernelIndex], 0, sizeof(cl_mem), &m_Image[iImageIndex]);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	size_t global_size[2] = { m_iImageWidth[iImageIndex], m_iImageHeight[iImageIndex] };
	error = clEnqueueNDRangeKernel(m_CommandQueue[iDeviceIndex], m_Kernel[iKernelIndex], 2, NULL, global_size, NULL, 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	size_t origin[3] = { 0, 0 };
	size_t region[3] = { m_iImageWidth[iImageIndex], m_iImageHeight[iImageIndex], 1 };
	error = clEnqueueReadImage(m_CommandQueue[iDeviceIndex], m_Image[iImageIndex], CL_TRUE, origin, region, 0, 0, (void*)m_BitmapBits[iImageIndex], 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	return true;
}


/***************************************************/
/*
bool COpenCLJulia::PrepareProgramBuffers(void)
{
	cl_int error;
	int i;

	for (i = 0; i < MAX_IMAGES; ++i)
	{
		m_ImageFormat[i].image_channel_data_type = CL_UNORM_INT8;
		m_ImageFormat[i].image_channel_order = CL_RGBA;

		if (m_Image[i] != (cl_mem)CL_INVALID_MEM_OBJECT)
		{
			clReleaseMemObject(m_Image[i]);
			m_Image[i] = (cl_mem)CL_INVALID_MEM_OBJECT;
		}
		m_Image[i] = clCreateImage2D(m_Context, CL_MEM_WRITE_ONLY, &m_ImageFormat[i], DEFAULT_IMAGE_SIZE, DEFAULT_IMAGE_SIZE, 0, NULL, &error);
		if (error != CL_SUCCESS)
			return false;

		m_PaletteBuffer = clCreateBuffer(m_Context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, sizeof(m_Palette), m_Palette, &error);
		if (error != CL_SUCCESS)
			return false;
	}

	// RAP: for now all images share one palette.  Enable each image to have its own (on the to-do list)
	for (i = 0; i < m_iNumberDevices; ++i)
	{
		error = clEnqueueWriteBuffer(m_CommandQueue[i], m_PaletteBuffer, CL_TRUE, 0, sizeof(m_PaletteBuffer), m_PaletteBuffer, 0, NULL, NULL);
	}

	return true;
}
bool COpenCLJulia::CleanupProgramBuffers()
{
	if (m_PaletteBuffer != (cl_mem)CL_INVALID_MEM_OBJECT)
	{
		clReleaseMemObject(m_PaletteBuffer);
		m_PaletteBuffer = (cl_mem)CL_INVALID_MEM_OBJECT;
	}

	for (int i = 0; i < MAX_IMAGES; ++i)
	{
		if (m_Image[i] != (cl_mem)CL_INVALID_MEM_OBJECT)
		{
			clReleaseMemObject(m_Image[i]);
			m_Image[i]= (cl_mem)CL_INVALID_MEM_OBJECT;
		}
	}

	return true;
}
*/
bool COpenCLFrac::ExecuteProgramJulia(int iDeviceIndex, int iKernelIndex, int iImageIndex, cl_int *pError)
{
	cl_int error;

	if (m_Kernel[iKernelIndex] == (cl_kernel)CL_INVALID_KERNEL)
		return false;

	m_NumericRect[iKernelIndex].s0 = m_ma1[iKernelIndex];
	m_NumericRect[iKernelIndex].s1 = m_mb1[iKernelIndex];
	m_NumericRect[iKernelIndex].s2 = m_ma2[iKernelIndex];
	m_NumericRect[iKernelIndex].s3 = m_mb2[iKernelIndex];

	error = clSetKernelArg(m_Kernel[iKernelIndex], 4, sizeof(m_PaletteBuffer), &m_PaletteBuffer);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[iKernelIndex], 3, sizeof(m_fMaxIterations[iKernelIndex]), &m_fMaxIterations[iKernelIndex]);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[iKernelIndex], 2, sizeof(m_NumericRect[iKernelIndex]), &m_NumericRect[iKernelIndex]);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[iKernelIndex], 1, sizeof(m_Const[iKernelIndex]), &m_Const[iKernelIndex]);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[iKernelIndex], 0, sizeof(cl_mem), &m_Image[iImageIndex]);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	size_t global_size[2] = { m_iImageWidth[iImageIndex], m_iImageHeight[iImageIndex] };
	error = clEnqueueNDRangeKernel(m_CommandQueue[iDeviceIndex], m_Kernel[iKernelIndex], 2, NULL, global_size, NULL, 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	size_t origin[3] = { 0, 0 };
	size_t region[3] = { m_iImageWidth[iImageIndex], m_iImageHeight[iImageIndex], 1 };
	error = clEnqueueReadImage(m_CommandQueue[iDeviceIndex], m_Image[iImageIndex], CL_TRUE, origin, region, 0, 0, (void*)m_BitmapBits[iImageIndex], 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	return true;
}
