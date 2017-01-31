#include <memory.h>
#include <stdlib.h>

#include "cpu.h"

/************ Beginning of source file sse41andsse42detection.cpp ********************/
/*	Copyright 2009 Intel Corporation
*	sse41andsse42detection.cpp
*	This file uses code first published by Intel as part of the processor enumeration
*	article available on the internet at:
*	http://software.intel.com/en-us/articles/intel-64-architecture-processor-topology-            *	enumeration/
*	Some of the original code from cpu_topo.c
*	has been removed, while other code has been added to illustrate the CPUID usage
* 	to determine if the processor supports the SSE 4.1 and SSE 4.2 instruction sets.
*	The reference code provided in this file is for demonstration purpose only. It assumes
*	the hardware topology configuration within a coherent domain does not change during
*	the life of an OS session. If an OS support advanced features that can change
*	hardware topology configurations, more sophisticated adaptation may be necessary
*	to account for the hardware configuration change that might have added and reduced
*  	the number of logical processors being managed by the OS.
*
*	Users of this code should be aware that the provided code
*	relies on CPUID instruction providing raw data reflecting the native hardware
*	configuration. When an application runs inside a virtual machine hosted by a
*	Virtual Machine Monitor (VMM), any CPUID instructions issued by an app (or a guest OS)
*	are trapped by the VMM and it is the VMM's responsibility and decision to emulate
*	CPUID return data to the virtual machines. When deploying topology enumeration code based
*	on CPUID inside a VM environment, the user must consult with the VMM vendor on how an VMM
*	will emulate CPUID instruction relating to topology enumeration.
*
*	Original code written by Patrick Fay, Ronen Zohar and Shihjong Kuo .
* 	Modified by Garrett Drysdale for current application note.
*/

// #include "sse41andsse42detection.h"

/*
int isSSE41andSSE42Supported(void)
{
	// returns 1 if is a Nehalem or later processor, 0 if prior to Nehalem

	CPUIDinfo Info;
	int rVal = 0;
	// The code first determines if the processor is an Intel Processor.  If it is, then 
	// feature flags bit 19 (SSE 4.1) and 20 (SSE 4.2) in ECX after CPUID call with EAX = 0x1
	// are checked.
	// If both bits are 1 (indicating both SSE 4.1 and SSE 4.2 exist) then 
	// the function returns 1 
	const int CHECKBITS = SSE4_1_FLAG | SSE4_2_FLAG;

	if (is_genuine_intel() >= 1)
	{
		// execute CPUID with eax (leaf) = 1 to get feature bits, 
		// subleaf doesn't matter so set it to zero
		get_cpuid_info(&Info, 0x1, 0x0);
		if ((Info.ECX & CHECKBITS) == CHECKBITS)
		{
			rVal = 1;
		}
	}
	return(rVal);
}
*/

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

/*
#if (defined(__x86_64__) || defined(_M_X64))
// This code is assembly for 64 bit target OS.
// Assembly code must be compiled with the –use-msasm switch for Linux targets with the 
// Intel compiler. 
int isCPUIDsupported(void)
{
	// returns 1 if CPUID instruction supported on this processor, zero otherwise
	// This isn't necessary on 64 bit processors because all 64 bit processor support CPUID
	return((int)1);
}

void get_cpuid_info(CPUIDinfo *Info, const unsigned int leaf, const unsigned int subleaf)
{
	// Stores CPUID return Info in the CPUIDinfo structure.
	// leaf and subleaf used as parameters to the CPUID instruction
	// parameters and register usage designed to be safe for both Windows and Linux
	// Use the Intel compiler option -use-msasm when the target is Linux
	__asm
	{
		mov r10d, subleaf; arg2, subleaf(in R8 on WIN, in RDX on Linux)
			mov r8, Info; arg0, array addr(in RCX on WIN, in RDI on Linux)
			mov r9d, leaf; arg1, leaf(in RDX on WIN, in RSI on Linux)
			push rax
			push rbx
			push rcx
			push rdx
			mov eax, r9d
			mov ecx, r10d
			cpuid
			mov	DWORD PTR[r8], eax
			mov	DWORD PTR[r8 + 4], ebx
			mov	DWORD PTR[r8 + 8], ecx
			mov	DWORD PTR[r8 + 12], edx
			pop rdx
			pop rcx
			pop rbx
			pop rax
	}
}

#else
*/
// 32 bit
//Note need to make sure -use-msasm switch is used with Intel compiler for Linux to get the
// ASM code to compile for both windows and linux with one version source


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
/*
#endif
*/
/************ End of source file sse41andsse42detection.cpp *******************************/
/************ Beginning of source file sse41andsse42detection.h ***************************/
/*		Copyright 2008 Intel Corporation
*	The source code contained or described herein and all documents related
*  	to the source code ("Material") are owned by Intel Corporation or
*	its suppliers or licensors. Use of this material must comply with the
*	rights and restrictions set forth in the accompnied license terms set
*  	forth in file "license.rtf".
*
*	Original code contained in cputopology.h.
* 	This file has been renamed to cpuid.h for this app note, code removed, and some
* 	code added.
*
*  	This is the header file that contain type definitions
*  	and prototypes of functions in the file cpuid.cpp
*	The source files can be compiled under 32-bit and 64-bit Windows and Linux.
*
*	Original code written by Patrick Fay and Shihjong Kuo
* 	Modified by Garrett Drysdale for this application note.
*/

/************ End of source file sse41andsse42detection.h ****************************/


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
	m_Caps.szVendorID[_countof(m_Caps.szVendorID)-1]=0;
	m_Caps.iHighestFunction = info_00.EAX;

	//
	// get the Processor Info and Feature Bits
	//
	get_cpuid_info(&info_01, 1, 0);

	// from EAX
	m_Caps.iStepping		= (info_01.EAX & CPUID_INTEL_STEPPING_MASK) >> CPUID_INTEL_STEPPING_SHIFT;
	m_Caps.iModel			= (info_01.EAX & CPUID_INTEL_MODEL_MASK) >> CPUID_INTEL_MODEL_SHIFT;
	m_Caps.iFamily			= (info_01.EAX & CPUID_INTEL_FAMILY_MASK) >> CPUID_INTEL_FAMILY_SHIFT;
	m_Caps.iProcessorType	= (info_01.EAX & CPUID_INTEL_PROCESSOR_TYPE_MASK) >> CPUID_INTEL_PROCESSOR_TYPE_SHIFT;
	m_Caps.iExtendedModel	= (info_01.EAX & CPUID_INTEL_EXTENDED_MODEL_MASK) >> CPUID_INTEL_EXTENDED_MODEL_SHIFT;
	m_Caps.iExtendedFamily	= (info_01.EAX & CPUID_INTEL_EXTENDED_FAMILY_MASK) >> CPUID_INTEL_EXTENDED_FAMILY_SHIFT;

	// from EDX
	m_Caps.has_x87 =	(info_01.EDX & CPUID_INTEL_FPU) != 0;
	m_Caps.has_MMX =	(info_01.EDX & CPUID_INTEL_MMX) != 0;
	m_Caps.has_SSE =	(info_01.EDX & CPUID_INTEL_SSE) != 0;
	m_Caps.has_SSE2 =	(info_01.EDX & CPUID_INTEL_SSE3) != 0;
	m_Caps.has_HyperThreading =  (info_01.EDX & CPUID_INTEL_HTT) != 0;
	m_Caps.has_CompareAndSwap8 =   (info_01.EDX & CPUID_INTEL_CX8) != 0;//		CMPXCHG8 (compare-and-swap) instruction

	// FROM ECX
	m_Caps.has_SSSE3 =		(info_01.ECX & CPUID_INTEL_SSSE3) != 0;
	m_Caps.has_CLMUL =		(info_01.ECX & CPUID_INTEL_PCLMULQDQ) != 0;//PCLMULQDQ support
	m_Caps.has_SSE4_1 =		(info_01.ECX & CPUID_INTEL_SSE4_1) != 0;
	m_Caps.has_SSE4_2 =		(info_01.ECX & CPUID_INTEL_SSE4_2) != 0;
	m_Caps.has_AVX =		(info_01.ECX & CPUID_INTEL_AVX) != 0;
	m_Caps.has_CompareAndSwap16 =   (info_01.EBX & CPUID_INTEL_CX16) != 0;//		CMPXCHG16B instruction
	m_Caps.has_FMA3 =		(info_01.ECX & CPUID_INTEL_FMA) != 0;
	m_Caps.has_AES =		(info_01.ECX & CPUID_INTEL_AES) != 0;
	m_Caps.has_Random =		(info_01.ECX & CPUID_INTEL_RDRND) != 0;		// RDRAND

	//
	// 
	//
	get_cpuid_info(&info_07, 7, 0);

	// from EBX
	m_Caps.has_AVX2 =		(info_07.EBX & CPUID_INTEL_AVX2) != 0;
	m_Caps.has_AVX512F =	(info_07.EBX & CPUID_INTEL_AVX512F) != 0;
	m_Caps.has_BMI1 =		(info_07.EBX & CPUID_INTEL_BMI1) != 0; // bit manipulation 1
	m_Caps.has_BMI2 =		(info_07.EBX & CPUID_INTEL_BMI2) != 0;// bit manipulation 2
	m_Caps.has_ADX = 		(info_07.EBX & CPUID_INTEL_ADX) != 0;//	Intel ADX (Multi-Precision Add-Carry Instruction Extensions) for arbitrary-precision math
	m_Caps.has_SHA = 		(info_07.EBX & CPUID_INTEL_SHA) != 0;// suppport for Secure Hash Algorithm

	// not implemented at this time
//	m_Caps.has_FMA4 =  (info_07.EBX & CPUID_INTEL_FPU) != 0;
//	m_Caps.has_XOP =   (info_07.EBX & CPUID_INTEL_FPU) != 0;//AMD
//	m_Caps.has_TSX = 	 (info_07.EBX & CPUID_INTEL_FPU) != 0;// Transactional Synchronization Extensions
}
