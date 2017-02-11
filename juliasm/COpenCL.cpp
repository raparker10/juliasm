#include "stdafx.h"

COpenCLImage::COpenCLImage()
{
	m_Image = (cl_mem)CL_INVALID_MEM_OBJECT;
	m_iWidth = m_iHeight = 0;
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

	// initialzie platform variables
	m_iCurrentPlatformIndex = -1;		// which OpenCL platform is currently being used
	m_iNumberPlatforms = 0;				// the total number of OpenCL platforms
	for (i = 0; i < MAX_PLATFORMS; ++i)
	{
		m_PlatformID[i] = (cl_platform_id)CL_INVALID_PLATFORM;
	}
	memset(m_szPlatformName, 0, sizeof(m_szPlatformName));
	
	// initialize device variables
	m_iCurrentDeviceIndex = -1;			// current OpenCL device.  this is an index into the following arrays
	m_iNumberDevices = 0;			// the total number of OpenCL devices for the current platform
	for (i = 0; i  < MAX_DEVICES; ++i)
	{
		m_DeviceID[i] = (cl_device_id)CL_INVALID_DEVICE; // OpenCL device IDs for the current platform
		m_DeviceType[i] = (cl_device_type)CL_INVALID_DEVICE_TYPE;	// OpenCL device types for the current platform (eg CPU, GPU, ...)
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

	// command queue variables
	m_CommandQueue = (cl_command_queue)CL_INVALID_COMMAND_QUEUE;	// command queue for the current program

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

	return m_iNumberPlatforms;
}

// selects the specified OpenCL platform for use`
bool COpenCL::UsePlatformByID(cl_platform_id platform_id)
{
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
	return m_iNumberDevices;

}

// select an OpenCL device for use by it's OpenCL device ID
bool COpenCL::UseDeviceByID(cl_device_id device_id)
{
	int iIndex = get_DeviceIndex(device_id);
	if (iIndex < 0)
		return false;

	m_iCurrentDeviceIndex = iIndex;
	return true;
}

// select an OpenCL device for use by it's OpenCL device type 
bool COpenCL::UseDeviceByType(cl_device_type device_type)
{
	for (int i = 0; i < get_DeviceCount(); ++i)
	{
		if (m_DeviceType[i] == device_type)
		{
			return UseDeviceByIndex(i);
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
	FILE *fh = fopen(m_szProgramPath, "rt");
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
	if (m_szProgramText[0] != NULL)
	{
		fread(m_szProgramText[0], m_iProgramLength[0], 1, fh);
		m_szProgramText[0][m_iProgramLength[0] - 1] = 0;
	}
	fclose(fh);
	return m_szProgramText[0] != NULL;
}

// prepare the OpenCL program for use
bool COpenCL::PrepareProgram(void)
{
	// first, cleanup any existing program
	CleanupProgram();

	cl_int error;

	// create a Context for the Program
	m_Context = clCreateContext(NULL, m_iNumberDevices, m_DeviceID, NULL, NULL, &error);
	if (error == CL_SUCCESS)
	{
		m_Program = clCreateProgramWithSource(m_Context, 1, (const char **)m_szProgramText, m_iProgramLength, &error);
		if (error == CL_SUCCESS)
		{
			// build the program for the CURRENT device
//			cl_int build_error = clBuildProgram(m_Program, 1, &m_DeviceID[m_iCurrentDeviceIndex],	"-s \"C:/Users/Robert/Documents/Visual Studio 2013/Projects/juliasm/fractals.cl\"", NULL, NULL);
			cl_int build_error = clBuildProgram(m_Program, 1, &m_DeviceID[m_iCurrentDeviceIndex],	NULL, NULL, NULL);
			m_iLastBuildStatus = build_error;

			// get the build log for the CURRENT device
			char build_log[BUILD_LOG_LEN];
			size_t size_ret;
			int d = m_iCurrentDeviceIndex;

			// get the build log
			error = clGetProgramBuildInfo(m_Program, m_DeviceID[m_iCurrentDeviceIndex], CL_PROGRAM_BUILD_LOG, 0, NULL, &size_ret);
			if (error == CL_SUCCESS)
			{
				char *ptr = (char*)calloc(size_ret + 1, 1);
				if (ptr != NULL)
				{
					error = clGetProgramBuildInfo(m_Program, m_DeviceID[d], CL_PROGRAM_BUILD_LOG, size_ret, ptr, &size_ret);
					if (size_ret > 0)
					{
						put_LastBuildMessage(ptr);
					}
					else
					{
						put_LastBuildMessage("");
					}
					free(ptr);
				}

			}

			if (build_error == CL_SUCCESS)
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

					m_CommandQueue = clCreateCommandQueue(m_Context, m_DeviceID[m_iCurrentDeviceIndex], 0, &error);


					//
					// execute each kernel on each device
					//
					return PrepareProgramBuffers();

				}
			}
		}
	}
	return false;
}
bool COpenCLImage::put_ImageSize(int width, int height)
{
	cl_int error;

	// release n existing mandelbrot image
	if (m_Image != (cl_mem)CL_INVALID_MEM_OBJECT)
	{
		clReleaseMemObject(m_Image);
		m_Image = (cl_mem)CL_INVALID_MEM_OBJECT;
	}

	m_iHeight = height;
	m_iWidth = width;

	if (width == 0 || height == 0)
		return true; // returns true, but there is no image

	m_ImageFormat.image_channel_data_type = CL_UNORM_INT8;
	m_ImageFormat.image_channel_order = CL_RGBA;

	m_Image = clCreateImage2D(m_Context, CL_MEM_WRITE_ONLY, &m_ImageFormat, width, height, 0, NULL, &error);
	if (error != CL_SUCCESS)
	{
		return false;
	}


	return error == CL_SUCCESS;
}

bool COpenCLMand::PrepareProgramBuffers(void)
{
	cl_int error;

	m_ImageFormat.image_channel_data_type = CL_UNORM_INT8;
	m_ImageFormat.image_channel_order = CL_RGBA;

	if (m_Image != (cl_mem)CL_INVALID_MEM_OBJECT)
	{
		clReleaseMemObject(m_Image);
		m_Image = (cl_mem)CL_INVALID_MEM_OBJECT;
	}
	m_Image = clCreateImage2D(m_Context, CL_MEM_WRITE_ONLY, &m_ImageFormat, DEFAULT_IMAGE_SIZE, DEFAULT_IMAGE_SIZE, 0, NULL, &error);
	if (error != CL_SUCCESS)
		return false;

	m_PaletteBuffer = clCreateBuffer(m_Context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, sizeof(m_Palette), m_Palette, &error);
	if (error != CL_SUCCESS)
		return false;
	error = clEnqueueWriteBuffer(m_CommandQueue, m_PaletteBuffer, CL_TRUE, 0, sizeof(m_PaletteBuffer), m_PaletteBuffer, 0, NULL, NULL);

	return true;
}
bool COpenCL::CleanupProgram(void)
{
	CleanupProgramBuffers();

	cl_int error;

	if (m_CommandQueue != (cl_command_queue)CL_INVALID_COMMAND_QUEUE)
	{
		error = clReleaseCommandQueue(m_CommandQueue);
		m_CommandQueue = (cl_command_queue)CL_INVALID_COMMAND_QUEUE;
	}

	for (int i = 0; i < m_iNumberKernels; ++i)
	{
		if (m_Kernel[i] != (cl_kernel)CL_INVALID_KERNEL)
		{
			error = clReleaseKernel(m_Kernel[i]);
			m_Kernel[i] = (cl_kernel)CL_INVALID_KERNEL;
			m_iNumberKernels = 0;
		}
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

	return true;
}
bool COpenCLMand::CleanupProgramBuffers(void)
{

	return true;
}
bool COpenCLMand::ExecuteProgram(int iKernel, cl_int *pError)
{
	cl_int error;

	m_NumericRect.s0 = m_ma1;
	m_NumericRect.s1 = m_mb1;
	m_NumericRect.s2 = m_ma2;
	m_NumericRect.s3 = m_mb2;

	if (m_Kernel[0] == (cl_kernel)CL_INVALID_KERNEL)
		return false;

	error = clSetKernelArg(m_Kernel[0], 3, sizeof(m_PaletteBuffer), &m_PaletteBuffer);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[0], 2, sizeof(m_fMaxIterations), &m_fMaxIterations);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[0], 1, sizeof(m_NumericRect), &m_NumericRect);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[0], 0, sizeof(cl_mem), &m_Image);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	size_t global_size[2] = { m_iWidth, m_iHeight };
	error = clEnqueueNDRangeKernel(m_CommandQueue, m_Kernel[0], 2, NULL, global_size, NULL, 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	size_t origin[3] = { 0, 0 };
	size_t region[3] = { m_iWidth, m_iHeight, 1 };
	error = clEnqueueReadImage(m_CommandQueue, m_Image, CL_TRUE, origin, region, 0, 0, (void*)m_BitmapBits, 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	return true;
}


/***************************************************/

bool COpenCLJulia::PrepareProgramBuffers(void)
{
	cl_int error;

	m_ImageFormat.image_channel_data_type = CL_UNORM_INT8;
	m_ImageFormat.image_channel_order = CL_RGBA;

	if (m_Image != (cl_mem)CL_INVALID_MEM_OBJECT)
	{
		clReleaseMemObject(m_Image);
		m_Image = (cl_mem)CL_INVALID_MEM_OBJECT;
	}
	m_Image = clCreateImage2D(m_Context, CL_MEM_WRITE_ONLY, &m_ImageFormat, DEFAULT_IMAGE_SIZE, DEFAULT_IMAGE_SIZE, 0, NULL, &error);
	if (error != CL_SUCCESS)
		return false;

	m_PaletteBuffer = clCreateBuffer(m_Context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, sizeof(m_Palette), m_Palette, &error);
	if (error != CL_SUCCESS)
		return false;
	error = clEnqueueWriteBuffer(m_CommandQueue, m_PaletteBuffer, CL_TRUE, 0, sizeof(m_PaletteBuffer), m_PaletteBuffer, 0, NULL, NULL);

	return true;
}
bool COpenCLJulia::CleanupProgramBuffers(void)
{
	if (m_PaletteBuffer != (cl_mem)CL_INVALID_MEM_OBJECT)
	{
		clReleaseMemObject(m_PaletteBuffer);
		m_PaletteBuffer = (cl_mem)CL_INVALID_MEM_OBJECT;
	}

	if (m_Image != (cl_mem)CL_INVALID_MEM_OBJECT)
	{
		clReleaseMemObject(m_Image);
		m_Image= (cl_mem)CL_INVALID_MEM_OBJECT;
	}

	return true;
}
bool COpenCLJulia::ExecuteProgram(int iKernel, cl_int *pError)
{
	cl_int error;

	if (m_Kernel[1] == (cl_kernel)CL_INVALID_KERNEL)
		return false;

	error = clSetKernelArg(m_Kernel[1], 4, sizeof(m_PaletteBuffer), &m_PaletteBuffer);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[1], 3, sizeof(m_fMaxIterations), &m_fMaxIterations);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[1], 2, sizeof(m_NumericRect), &m_NumericRect);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[1], 1, sizeof(m_Const), &m_Const);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	error = clSetKernelArg(m_Kernel[1], 0, sizeof(cl_mem), &m_Image);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	size_t global_size[2] = { m_iWidth, m_iHeight };
	error = clEnqueueNDRangeKernel(m_CommandQueue, m_Kernel[1], 2, NULL, global_size, NULL, 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	size_t origin[3] = { 0, 0 };
	size_t region[3] = { m_iWidth, m_iHeight, 1 };
	error = clEnqueueReadImage(m_CommandQueue, m_Image, CL_TRUE, origin, region, 0, 0, (void*)m_BitmapBits, 0, NULL, NULL);
	if (error != CL_SUCCESS)
	{
		*pError = error;
		return false;
	}

	return true;
}


