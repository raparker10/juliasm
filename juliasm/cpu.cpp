#include "stdafx.h"

#include <memory.h>
#include <stdlib.h>


TProc CCPU::m_CPUList[] = {
	{ 0x06, 0x25, "Arrandale", "2010.1", "mobile uarch= westmere" },
	{ 0x06, 0x4D, "Avoton", "2013.3", "Atom SoC microserver, silvermont cores" },
	{ 0x06, 0x09, "Banias", "2003.2", "pentium M cpus, based on modified p6 uarch" },
	{ 0x06, 0x37, "Bay Trail", "2013.3", "atom SOC, silvermont cores" },
	{ 0x06, 0x1A, "Bloomfield", "2008.4", "quad core xeon, uarch= nehalem" },
	{ 0x00, 0x00, "Briarwood", "2013.2", "storage SoCs, Intel Atom S1000 series, uarch= saltwell" },
	{ 0x06, 0x3D, "Broadwell", "2014.3", "broadwell" },
	{ 0x06, 0x47, "Broadwell", "2014.3", "broadwell" },
	{ 0x06, 0x4F, "Broadwell", "2014.3", "broadwell" },
	{ 0x06, 0x56, "Broadwell", "2014.3", "broadwell" },
	{ 0x06, 0x07, "Cascades", "1999.4", "pentium iii xeon, coppermine cores" },
	{ 0x0F, 0x06, "Cedarmill", "2006.1", "pentium 4" },
	{ 0x06, 0x36, "Cedarview", "2011.3", "atom saltwell" },
	{ 0x06, 0x36, "Centerton", "2012.4", "atom saltwell" },
	{ 0x00, 0x00, "Chevelon", "2007.1", "Intel(r) IOP342 I/O Processor" },
	{ 0x05, 0x09, "Clanton", "2013.4", "quark" },
	{ 0x06, 0x25, "Clarkdale", "2010.1", "a westmere cpu" },
	{ 0x06, 0x1E, "Clarksfield", "2009.3", "nehalem uarch" },
	{ 0x06, 0x0F, "Clovertown", "2006.4", "uarch= core, Woodcrest, Tigerton, Kentfield, Clovertown" },
	{ 0x06, 0x35, "Cloverview", "2013.2", "Cloverview, saltwell" },
	{ 0x06, 0x0F, "Conroe", "2006.3", "" },
	{ 0x06, 0x08, "Coppermine", "2000.1", "pentium iii" },
	{ 0x0F, 0x04, "Cranford", "2005.2", "pentium 4, netburst" },
	{ 0x06, 0x46, "Crystal Well", "2013.2", "haswell based" },
	{ 0x0F, 0x06, "Dempsey", "2006.2", "pentium 4" },
	{ 0x06, 0x3C, "Devil's Canyon", "2014.2", "haswell based" },
	{ 0x06, 0x1C, "Diamondville", "2008.2", "bonnell core" },
	{ 0x06, 0x06, "Dixon", "1999.1", "pentium ii" },
	{ 0x06, 0x0D, "Dothan", "2004.2", "pentium M, uarch= p6 variant" },
	{ 0x06, 0x1D, "Dunnington", "2008.3", "quad core xeon, uarch= core" },
	{ 0x0F, 0x01, "Foster", "2001.1", "pentium 4" },
	{ 0x0F, 0x02, "Gallatin", "2003.2", "pentium 4" },
	{ 0x06, 0x2A, "Gladden", "2012.2", "sandy bridge" },
	{ 0x06, 0x2C, "Gulftown", "2010.1", "based on westmere" },
	{ 0x06, 0x17, "Harpertown", "2007.4", "uarch= penryn" },
	{ 0x06, 0x3C, "Haswell", "2013.2", "haswell" },
	{ 0x06, 0x3F, "Haswell E", "2014.3", "haswell e" },
	{ 0x0F, 0x07, "Irwindale", "2005.1", "pentium 4" },
	{ 0x06, 0x3A, "Ivy Bridge", "2012.2", "ivy bridge" },
	{ 0x06, 0x3E, "Ivy Bridge E", "2013.3", "ivy bridge-e, i7-4930K" },
	{ 0x06, 0x3E, "Ivy Bridge EN", "2014.1", "ivy bridge en" },
	{ 0x06, 0x3E, "Ivy Bridge EP", "2013.3", "ivy bridge ep" },
	{ 0x06, 0x1E, "Jasper Forest"	"2010.1", "xeon uarch= nehalem" },
	{ 0x06, 0x07, "Katmai", "1999.1", "pentium iii" },
	{ 0x06, 0x0F, "Kentsfield", "2006.4", "uarch= core" },
	{ 0x0B, 0x01, "Knights Corner", "2012.4", "xeon phi" },
	{ 0x06, 0x26, "Lincroft", "2010.2", "atom (bonnell)" },
	{ 0x06, 0x1E, "Lynnfield", "2009.3", "uarch= nehalem" },
	{ 0x1F, 0x03, "Madison", "2004.2", "itanium-2" },
	{ 0x06, 0x06, "Mendocino", "1999.1", "pentium ii" },
	{ 0x06, 0x0F, "Merom", "2006.3", "uarch= core" },
	{ 0x06, 0x4A, "Merrifield", "2014.1", "uarch= silvermont" },
	{ 0x20, 0x00, "Montecito", "2007.1", "uarch= itanium, after madison" },
	{ 0x20, 0x01, "Montvale", "2007.4", "uarch= itanium, after montecito" },
	{ 0x06, 0x5A, "Moorefield", "2014.2", "uarch= silvermont" },
	{ 0x06, 0x1A, "Nehalem EP", "2009.1", "uarch= nehalem" },
	{ 0x06, 0x2E, "Nehalem EX", "2010.1", "uarch= nehalem, beckton" },
	{ 0x0F, 0x03, "Nocona", "2004.2", "pentium 4" },
	{ 0x0F, 0x02, "Northwood", "2002.1", "pentium 4" },
	{ 0x0F, 0x04, "Paxville", "2005.3", "pentium 4" },
	{ 0x06, 0x17, "Penryn", "2008.1", "uarch= penryn" },
	{ 0x06, 0x27, "Penwell", "2012.2", "atom saltwell core" },
	{ 0x00, 0x00, "Pine Cove", "2014.3", "mobile communications chip such as Intel(r) Transcede(tm) T2150" },
	{ 0x06, 0x1C, "Pineview", "2010.1", "atom, bonnell core" },
	{ 0x0F, 0x04, "Potomac", "2005.2", "pentium 4" },
	{ 0x21, 0x00, "Poulson", "2012.4", "itanium, after tukwila" },
	{ 0x0F, 0x03, "Prescott", "2004.1", "pentium 4" },
	{ 0x0F, 0x06, "Presler", "2006.1", "pentium 4" },
	{ 0x0F, 0x02, "Prestonia", "2002.1", "I think the dfdm is right, pentium 4 xeon" },
	{ 0x06, 0x4D, "Rangeley", "2013.3", "communications chip based on avoton with silvermont cores" },
	{ 0x06, 0x2A, "Sandy Bridge", "2011.1", "uarch= sandy bridge" },
	{ 0x06, 0x2D, "Sandy Bridge E", "2011.4", "uarch= sandy bridge" },
	{ 0x06, 0x2D, "Sandy Bridge EN", "2012.2", "uarch= sandy bridge" },
	{ 0x06, 0x2D, "Sandy Bridge EP", "2012.1", "uarch= sandy bridge" },
	{ 0x06, 0x1C, "Silverthorne", "2008.2", "bonnell cores" },
	{ 0x0F, 0x04, "Smithfield", "2005.1", "pentium 4" },
	{ 0x06, 0x0E, "Sossaman", "2006.1", "xeon based on yonah" },
	{ 0x06, 0x1C, "Stellarton", "2010.4", "embedded atom, bonnell cores" },
	{ 0x00, 0x00, "Sunrise Lake", "2007.1", "ioprocessors like IOP348" },
	{ 0x06, 0x07, "Tanner", "1999.1", "pentium iii xeon" },
	{ 0x06, 0x0F, "Tigerton", "2007.3", "dual/quad core xeon, uarch= core" },
	{ 0x06, 0x0D, "Tolapai", "2008.3", "dothan cores" },
	{ 0x06, 0x0B, "Tualatin", "2001.4", "pentium iii" },
	{ 0x20, 0x02, "Tukwila", "2010.1", "itanium, after montvale, before poulson" },
	{ 0x0F, 0x06, "Tulsa", "2006.3", "pentium 4 dual core xeon" },
	{ 0x06, 0x26, "Tunnel Creek", "2010.3", "Intel Atom Z670" },
	{ 0x00, 0x00, "Val Vista", "2007.1", "Intel(r) IOC340 I/O Controller" },
	{ 0x06, 0x2C, "Westmere EP", "2010.1", "DP xeon, uarch= westmere" },
	{ 0x06, 0x2F, "Westmere EX", "2011.2", "4socket xeon, uarch= westmere" },
	{ 0x0F, 0x01, "Willamette", "2000.4", "pentium 4" },
	{ 0x06, 0x17, "Wolfdale", "2007.4", "uarch= penryn, shrink of core uarch" },
	{ 0x06, 0x0F, "Woodcrest", "2006.2", "xeon uarch= core" },
	{ 0x06, 0x0E, "Yonah", "2006.1", "based on Banias/Dothan-core Pentium M microarchitecture" },
	{ 0x06, 0x17, "Yorkfield", "2007.4", "quad core xeon, uarch= core" },
};

int CCPU::m_iCPUCount = sizeof(m_CPUList) / sizeof(m_CPUList[0]);


//#include "cpu.h"
//
// returns 1 if the processor is an Intel processor
//
int CCPU::is_genuine_intel(void)
{
	// returns largest function # supported by CPUID if it is a Geniune Intel processor AND it supports
	// the CPUID instruction, 0 if not
	CPUIDinfo Info;
	int rVal = 0;
	char procString[] = "GenuineIntel";

	if (is_cpuid_supported())
	{
		// execute CPUID with eax = 0, subleaf doesn't matter so set it to zero
		get_cpuid_info(&Info, 0x0, 0x0);
		if ((Info.EBX == ((int *)procString)[0]) &&
			(Info.EDX == ((int *)procString)[1]) && (Info.ECX == ((int *)procString)[2]))
		{
			rVal = Info.EAX;
		}
	}
	return(rVal);
}

//
// returns 1 if the CPUID instruction is supported
//
int CCPU::is_cpuid_supported(void)
{
	// returns 1 if CPUID instruction supported on this processor, zero otherwise
	// This isn't necessary on 64 bit processors because all 64 bit Intel processors support CPUID
	__asm
	{
		push ecx; save ecx
			pushfd;			// push original EFLAGS
		pop eax;		// get original EFLAGS
		mov ecx, eax;	// save original EFLAGS
		xor eax, 200000h; // flip bit 21 in EFLAGS
		push eax;		// save new EFLAGS value on stack
		popfd;			// replace current EFLAGS value
		pushfd;			// get new EFLAGS
		pop eax;		// store new EFLAGS in EAX
		xor eax, ecx;	// Bit 21 of flags at 200000h will be 1 if CPUID exists
		shr eax, 21;	// Shift bit 21 bit 0 and return it
		push ecx
			popfd;			// restore bit 21 in EFLAGS first
		pop ecx;		// restore ecx
	}
}

//Note need to make sure -use-msasm switch is used with Intel compiler for Linux to get the
// ASM code to compile for both windows and linux with one version source
void CCPU::get_cpuid_info(CPUIDinfo *Info, const unsigned int leaf, const unsigned int subleaf)
{
	// Stores CPUID return Info in the CPUIDinfo structure.
	// leaf and subleaf used as parameters to the CPUID instruction
	// parameters and registure usage designed to be safe for both Win and Linux
	// when using -use-msasm
	__asm
	{
		mov	edx, Info;		// addr of start of output array
		mov	eax, leaf;		// leaf
		mov	ecx, subleaf;	// subleaf
		push edi
			push ebx
			mov  edi, edx;		// edi has output addr
		cpuid
			mov	DWORD PTR[edi], eax
			mov	DWORD PTR[edi + 4], ebx
			mov	DWORD PTR[edi + 8], ecx
			mov	DWORD PTR[edi + 12], edx
			pop ebx
			pop edi
	}
}

CCPU::CCPU()
{
	// initialize the capabilities
	memset(&m_Caps, 0, sizeof(m_Caps));

	// make sure CPUID is supported
	m_Caps.has_CPUID = (is_cpuid_supported() == 0) ? false : true;

	// bail if no CPUID instruction
	if (has_CPUID() == false)
	{
		return;
	}

	// make sure we're on an Intel processor
	//	is_genuine_intel();


	//
	// get the Vendor ID
	//
	get_cpuid_info(&info_00, 0, 0);

	((unsigned int*)m_Caps.szVendorID)[0] = info_00.EBX;
	((unsigned int*)m_Caps.szVendorID)[1] = info_00.EDX;
	((unsigned int*)m_Caps.szVendorID)[2] = info_00.ECX;
	m_Caps.szVendorID[_countof(m_Caps.szVendorID) - 1] = 0;
	m_Caps.iHighestFunction = info_00.EAX;

	//
	// get the Processor Info and Feature Bits
	//
	get_cpuid_info(&info_01, 1, 0);

	// from EAX
	m_Caps.iStepping = (info_01.EAX & CPUID_INTEL_STEPPING_MASK) >> CPUID_INTEL_STEPPING_SHIFT;
	m_Caps.iModel = (info_01.EAX & CPUID_INTEL_MODEL_MASK) >> CPUID_INTEL_MODEL_SHIFT;
	m_Caps.iFamily = (info_01.EAX & CPUID_INTEL_FAMILY_MASK) >> CPUID_INTEL_FAMILY_SHIFT;
	m_Caps.iProcessorType = (info_01.EAX & CPUID_INTEL_PROCESSOR_TYPE_MASK) >> CPUID_INTEL_PROCESSOR_TYPE_SHIFT;
	m_Caps.iExtendedModel = (info_01.EAX & CPUID_INTEL_EXTENDED_MODEL_MASK) >> CPUID_INTEL_EXTENDED_MODEL_SHIFT;
	m_Caps.iExtendedFamily = (info_01.EAX & CPUID_INTEL_EXTENDED_FAMILY_MASK) >> CPUID_INTEL_EXTENDED_FAMILY_SHIFT;

	// from EDX
	m_Caps.has_x87 = (info_01.EDX & CPUID_INTEL_FPU) != 0;
	m_Caps.has_MMX = (info_01.EDX & CPUID_INTEL_MMX) != 0;
	m_Caps.has_SSE = (info_01.EDX & CPUID_INTEL_SSE) != 0;
	m_Caps.has_SSE2 = (info_01.EDX & CPUID_INTEL_SSE3) != 0;
	m_Caps.has_HyperThreading = (info_01.EDX & CPUID_INTEL_HTT) != 0;
	m_Caps.has_CompareAndSwap8 = (info_01.EDX & CPUID_INTEL_CX8) != 0;//		CMPXCHG8 (compare-and-swap) instruction

	// FROM ECX
	m_Caps.has_SSSE3 = (info_01.ECX & CPUID_INTEL_SSSE3) != 0;
	m_Caps.has_CLMUL = (info_01.ECX & CPUID_INTEL_PCLMULQDQ) != 0;//PCLMULQDQ support
	m_Caps.has_SSE4_1 = (info_01.ECX & CPUID_INTEL_SSE4_1) != 0;
	m_Caps.has_SSE4_2 = (info_01.ECX & CPUID_INTEL_SSE4_2) != 0;
	m_Caps.has_AVX = (info_01.ECX & CPUID_INTEL_AVX) != 0;
	m_Caps.has_CompareAndSwap16 = (info_01.EBX & CPUID_INTEL_CX16) != 0;//		CMPXCHG16B instruction
	m_Caps.has_FMA3 = (info_01.ECX & CPUID_INTEL_FMA) != 0;
	m_Caps.has_AES = (info_01.ECX & CPUID_INTEL_AES) != 0;
	m_Caps.has_Random = (info_01.ECX & CPUID_INTEL_RDRND) != 0;		// RDRAND

	//
	// 
	//
	get_cpuid_info(&info_07, 7, 0);

	// from EBX
	m_Caps.has_AVX2 = (info_07.EBX & CPUID_INTEL_AVX2) != 0;
	m_Caps.has_AVX512F = (info_07.EBX & CPUID_INTEL_AVX512F) != 0;
	m_Caps.has_BMI1 = (info_07.EBX & CPUID_INTEL_BMI1) != 0; // bit manipulation 1
	m_Caps.has_BMI2 = (info_07.EBX & CPUID_INTEL_BMI2) != 0;// bit manipulation 2
	m_Caps.has_ADX = (info_07.EBX & CPUID_INTEL_ADX) != 0;//	Intel ADX (Multi-Precision Add-Carry Instruction Extensions) for arbitrary-precision math
	m_Caps.has_SHA = (info_07.EBX & CPUID_INTEL_SHA) != 0;// suppport for Secure Hash Algorithm

	// not implemented at this time
	//	m_Caps.has_FMA4 =  (info_07.EBX & CPUID_INTEL_FPU) != 0;
	//	m_Caps.has_XOP =   (info_07.EBX & CPUID_INTEL_FPU) != 0;//AMD
	//	m_Caps.has_TSX = 	 (info_07.EBX & CPUID_INTEL_FPU) != 0;// Transactional Synchronization Extensions
}
bool CCPU::get_VendorID(char *szBuf, size_t iBufCount)
{
	if (szBuf != NULL && iBufCount > 0)
	{
		if (has_CPUID())
		{
			strcpy_s(szBuf, iBufCount, m_Caps.szVendorID);
			return true;
		}
	}
	return false;
}

bool CCPU::get_SteppingName(char *szBuf, size_t iBufCount)
{
	if (szBuf != NULL && iBufCount > 0)
	{
		if (has_CPUID())
		{
			_itoa_s(m_Caps.iStepping, szBuf, iBufCount, 10);
			return true;
		}
	}
	return false;
}
bool CCPU::get_ModelName(char *szBuf, size_t iBufCount)
{
	if (szBuf != NULL && iBufCount > 0)
	{
		if (has_CPUID())
		{
			_itoa_s((m_Caps.iExtendedModel << 4) + m_Caps.iModel, szBuf, iBufCount, 16);
			return true;
		}
	}
	return false;
}
bool CCPU::get_FamilyName(char *szBuf, size_t iBufCount)
{
	if (szBuf != NULL && iBufCount > 0)
	{
		if (has_CPUID())
		{
			_itoa_s((m_Caps.iExtendedFamily << 4) + m_Caps.iFamily, szBuf, iBufCount, 16);
			return true;
		}
	}
	return false;
}
bool CCPU::get_ProcessorTypeName(char *szBuf, size_t iBufCount)
{
	if (szBuf != NULL && iBufCount > 0)
	{
		if (has_CPUID())
		{
			_itoa_s(m_Caps.iProcessorType, szBuf, iBufCount, 10);
			return true;
		}
	}
	return false;
}
bool CCPU::get_MicroarchitectureName(char *szBuf, size_t iBufCount)
{
	if (szBuf != NULL && iBufCount > 0)
	{
		*szBuf = 0;
		if (has_CPUID())
		{
			int iFamily = (m_Caps.iExtendedFamily << 4) + m_Caps.iFamily;
			int iModel = (m_Caps.iExtendedModel << 4) + m_Caps.iModel;

			// find the cpu in the cpu list
			for (int i = 0; i < m_iCPUCount; ++i)
			{
				if (m_CPUList[i].iFamily == iFamily && m_CPUList[i].iModel == iModel)
				{
					strcpy_s(szBuf, iBufCount, m_CPUList[i].szName);
					return true;
				}
			}
			strcpy_s(szBuf, iBufCount, "Unknown");
			return true;
		}
	}
	return false;
}