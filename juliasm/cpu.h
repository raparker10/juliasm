#define SSE4_1_FLAG		0x080000
#define SSE4_2_FLAG		0x100000

typedef struct
{
	unsigned __int32 EAX, EBX, ECX, EDX;
} CPUIDinfo;

/*void get_cpuid_info(CPUIDinfo *, const unsigned int, const unsigned int);
int is_cpuid_supported(void);
int is_genuine_intel(void);
int isSSE41andSSE42Supported(void);
*/

#define CPUID_INTEL_STEPPING_MASK		 (0x0000000F)
#define CPUID_INTEL_MODEL_MASK			 (0x000000F0)
#define CPUID_INTEL_FAMILY_MASK			 (0x00000F00)
#define CPUID_INTEL_PROCESSOR_TYPE_MASK	 (0x00003000)
#define CPUID_INTEL_EXTENDED_MODEL_MASK	 (0x000F0000)
#define CPUID_INTEL_EXTENDED_FAMILY_MASK (0x0FF00000)

#define CPUID_INTEL_STEPPING_SHIFT			0
#define CPUID_INTEL_MODEL_SHIFT				4
#define CPUID_INTEL_FAMILY_SHIFT			8
#define CPUID_INTEL_PROCESSOR_TYPE_SHIFT	12
#define CPUID_INTEL_EXTENDED_MODEL_SHIFT	16
#define CPUID_INTEL_EXTENDED_FAMILY_SHIFT	20


#define CPUID_INTEL_FPU		(1 << 00)
#define CPUID_INTEL_VME		(1 << 01)
#define CPUID_INTEL_DE		(1 << 02)
#define CPUID_INTEL_PSE		(1 << 03)
#define CPUID_INTEL_TSC		(1 << 04)
#define CPUID_INTEL_MSR		(1 << 05)
#define CPUID_INTEL_PAE		(1 << 06)
#define CPUID_INTEL_MCE		(1 << 07)
#define CPUID_INTEL_CX8		(1 <<  8)
#define CPUID_INTEL_APIC	(1 <<  9)
//#define CPUID_INTEL_		(1 << 10)
#define CPUID_INTEL_SEP		(1 << 11)
#define CPUID_INTEL_MTRR	(1 << 12)
#define CPUID_INTEL_PGE		(1 << 13)
#define CPUID_INTEL_MCA		(1 << 14)
#define CPUID_INTEL_CMOV	(1 << 15)
#define CPUID_INTEL_PAT		(1 << 16)
#define CPUID_INTEL_PSE_36	(1 << 17)
#define CPUID_INTEL_PSN		(1 << 18)
#define CPUID_INTEL_CLFSH	(1 << 19)
//#define CPUID_INTEL_MSR	(1 << 20)
#define CPUID_INTEL_DS		(1 << 21)
#define CPUID_INTEL_ACPI	(1 << 22)
#define CPUID_INTEL_MMX		(1 << 23)
#define CPUID_INTEL_FXSR	(1 << 24)
#define CPUID_INTEL_SSE		(1 << 25)
#define CPUID_INTEL_SSE2	(1 << 26)
#define CPUID_INTEL_SS		(1 << 27)
#define CPUID_INTEL_HTT		(1 << 28)
#define CPUID_INTEL_TM		(1 << 29)
#define CPUID_INTEL_IA64	(1 << 30)
#define CPUID_INTEL_PBE		(1 << 31)

#define CPUID_INTEL_SSE3			(1 << 00)
#define CPUID_INTEL_PCLMULQDQ		(1 << 01)
#define CPUID_INTEL_DTES64			(1 << 02)
#define CPUID_INTEL_MONITOR			(1 << 03)
#define CPUID_INTEL_DS_CPL			(1 << 04)
#define CPUID_INTEL_VMX				(1 << 05)
#define CPUID_INTEL_SMX				(1 << 06)
#define CPUID_INTEL_EST				(1 << 07)
#define CPUID_INTEL_TM2				(1 << 08)
#define CPUID_INTEL_SSSE3			(1 <<  9)
#define CPUID_INTEL_CNXT_ID			(1 << 10)
#define CPUID_INTEL_SDBG			(1 << 11)
#define CPUID_INTEL_FMA				(1 << 12)
#define CPUID_INTEL_CX16			(1 << 13)
#define CPUID_INTEL_XTPR			(1 << 14)
#define CPUID_INTEL_PDCM			(1 << 15)
#define CPUID_INTEL_na16			(1 << 16)
#define CPUID_INTEL_PCID			(1 << 17)
#define CPUID_INTEL_DCA				(1 << 18)
#define CPUID_INTEL_SSE4_1			(1 << 19)
#define CPUID_INTEL_SSE4_2			(1 << 20)
#define CPUID_INTEL_X2APIC			(1 << 21)
#define CPUID_INTEL_MOVBE			(1 << 22)
#define CPUID_INTEL_POPCNT			(1 << 23)
#define CPUID_INTEL_TSC_DEADLINE	(1 << 24)
#define CPUID_INTEL_AES				(1 << 25)
#define CPUID_INTEL_XSAVE			(1 << 26)
#define CPUID_INTEL_OXSAVE			(1 << 27)
#define CPUID_INTEL_AVX				(1 << 28)
#define CPUID_INTEL_F16C			(1 << 29)
#define CPUID_INTEL_RDRND			(1 << 30)
#define CPUID_INTEL_HYPERVISOR		(1 << 31)

#define CPUID_INTEL_FSGSBASE	(1 << 00)
#define CPUID_INTEL_IA32_TSC_ADJUST		(1 << 01)
#define CPUID_INTEL_SGX			(1 << 02)
#define CPUID_INTEL_BMI1		(1 << 03)
#define CPUID_INTEL_TSX_HLE			(1 << 04)
#define CPUID_INTEL_AVX2		(1 << 05)
#define CPUID_INTEL_na6			(1 << 06)
#define CPUID_INTEL_SMEP		(1 << 07)
#define CPUID_INTEL_BMI2		(1 <<  8)
#define CPUID_INTEL_ERMS		(1 <<  9)
#define CPUID_INTEL_INVPCID		(1 << 10)
#define CPUID_INTEL_TSX_RTM			(1 << 11)
#define CPUID_INTEL_PQM			(1 << 12)
#define CPUID_INTEL_FPU_CS_DS_DEPRICATED		(1 << 13)
#define CPUID_INTEL_MPX			(1 << 14)
#define CPUID_INTEL_PQE			(1 << 15)
#define CPUID_INTEL_AVX512F		(1 << 16)
#define CPUID_INTEL_AVX512DQ	(1 << 17)
#define CPUID_INTEL_RDSEED		(1 << 18)
#define CPUID_INTEL_ADX			(1 << 19)
#define CPUID_INTEL_SMAP		(1 << 20)
#define CPUID_INTEL_AVX512IFMA	(1 << 21)
#define CPUID_INTEL_PCOMMIT		(1 << 22)
#define CPUID_INTEL_CLFLUSHOPT	(1 << 23)
#define CPUID_INTEL_CLWB		(1 << 24)
#define CPUID_INTEL_INTEL_PT	(1 << 25)
#define CPUID_INTEL_AVX512PF	(1 << 26)
#define CPUID_INTEL_AVX512ER	(1 << 27)
#define CPUID_INTEL_AVX512CD	(1 << 28)
#define CPUID_INTEL_SHA			(1 << 29)
#define CPUID_INTEL_AVX512BW	(1 << 30)
#define CPUID_INTEL_AVX512VL	(1 << 31)


struct TProc {
	int iFamily;
	int iModel;
	char *szName;
	char *szReleaseDate;
	char *szComment;
};
struct TProcCaps {
	bool has_CPUID;
	unsigned int iHighestFunction;

	char szVendorID[13];
	int iStepping;
	int iModel;
	int iFamily;
	int iProcessorType;
	int iExtendedModel;
	int iExtendedFamily;

	bool has_x87;
	bool has_MMX;
	bool has_SSE;
	bool has_SSE2;
	bool has_SSSE3;
	bool has_SSE4_1;
	bool has_SSE4_2;
	bool has_AVX;
	bool has_AVX2;
	bool has_AVX512F; // 512 foundation
	bool has_CLMUL; //PCLMULQDQ support

	bool has_FMA3;
//	bool has_FMA4;
//	bool has_XOP; //AMD

	bool has_HyperThreading;
	bool has_CompareAndSwap8; //		CMPXCHG8 (compare-and-swap) instruction
	bool has_CompareAndSwap16; //		CMPXCHG16B instruction
	bool has_AES;
	bool has_Random;		// RDRAND

	bool has_BMI1; // bit manipulation 1
	bool has_BMI2; // bit manipulation 2

	bool has_TSX;	// Transactional Synchronization Extensions
	bool has_ADX;	//	Intel ADX (Multi-Precision Add-Carry Instruction Extensions) for arbitrary-precision math
	bool has_SHA;	// suppport for Secure Hash Algorithm

};

class CCPU {
protected:
	CPUIDinfo info_00;
	CPUIDinfo info_01;
	CPUIDinfo info_07;

	TProcCaps m_Caps;
	static TProc m_CPUList[];
	static int m_iCPUCount;

	static void get_cpuid_info(CPUIDinfo *, const unsigned int, const unsigned int);
	static int is_cpuid_supported(void);
	static int is_genuine_intel(void);
public:
	CCPU();
	bool get_VendorID(char *szBuf, size_t iBufCount);
	bool get_SteppingName(char *szBuf, size_t iBufCount);
	bool get_ModelName(char *szBuf, size_t iBufCount);
	bool get_FamilyName(char *szBuf, size_t iBufCount);
	bool get_ProcessorTypeName(char *szBuf, size_t iBufCount);
	bool get_MicroarchitectureName(char *szBuf, size_t iBufCount);
	
	bool has_CPUID(void) const { return m_Caps.has_CPUID; }
	bool has_x87(void) const { return m_Caps.has_x87;  }
	bool has_MMX(void) const  { return m_Caps.has_MMX; }
	bool has_SSE(void) const { return m_Caps.has_SSE;}
	bool has_SSE2(void) const { return m_Caps.has_SSE2; }
	bool has_SSSE3(void) const { return m_Caps.has_SSSE3; }
	bool has_SSE4_1(void) const { return m_Caps.has_SSE4_1; }
	bool has_SSE4_2(void) const { return m_Caps.has_SSE4_2; }
	bool has_AVX(void) const { return m_Caps.has_AVX; }
	bool has_AVX2(void) const { return m_Caps.has_AVX2; }
	bool has_AVX512F(void) const { return m_Caps.has_AVX512F; }
	bool has_CLMUL(void) const {return m_Caps.has_CLMUL; }//PCLMULQDQ support

	bool has_FMA3(void) const {	return m_Caps.has_FMA3;	}
//	bool has_FMA4(void) const { return m_Caps.has_FMA4; }
//	bool has_XOP(void) const { return m_Caps.has_XOP; } //AMD
	
	bool has_HyperThreading(void) const { return m_Caps.has_HyperThreading; }
	bool has_CompareAndSwap8(void) const{ return m_Caps.has_CompareAndSwap8; } //		CMPXCHG8 (compare-and-swap) instruction
	bool has_CompareAndSwap16(void) const{ return m_Caps.has_CompareAndSwap16; } //		CMPXCHG16B instruction
	bool has_AES(void) const{ return m_Caps.has_AES; }
	bool has_Random(void) const{ return m_Caps.has_Random; }		// RDRAND

	bool has_BMI1(void) const{ return m_Caps.has_BMI1; } // bit manipulation 1
	bool has_BMI2(void) const{ return m_Caps.has_BMI2; } // bit manipulation 2

	bool has_TSX(void) const{ return m_Caps.has_TSX; }	// Transactional Synchronization Extensions
	bool has_ADX(void) const{ return m_Caps.has_ADX; }	//	Intel ADX (Multi-Precision Add-Carry Instruction Extensions) for arbitrary-precision math
	bool has_SHA(void) const{ return m_Caps.has_SHA; }	// suppport for Secure Hash Algorithm
};