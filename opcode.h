/* OPCODE.H	(c) Copyright Jan Jaeger, 2000-2001		     */
/*		Instruction decoding macros and prototypes	     */

/* Interpretive Execution - (c) Copyright Jan Jaeger, 1999-2001      */
/* z/Architecture support - (c) Copyright Jan Jaeger, 1999-2001      */

#if !defined(_OPCODE_H)

#define _OPCODE_H


#define GEN_MAXARCH	3


#define GENx___x___x___ \
    { \
	&s370_operation_exception, \
	&s390_operation_exception, \
	&z900_operation_exception \
    }

#define GENx370x___x___(_name) \
    { \
	&s370_ ## _name, \
	&s390_operation_exception, \
	&z900_operation_exception \
    }

#define GENx___x390x___(_name) \
    { \
	&s370_operation_exception, \
	&s390_ ## _name, \
	&z900_operation_exception \
    }

#define GENx370x390x___(_name) \
    { \
	&s370_ ## _name, \
	&s390_ ## _name, \
	&z900_operation_exception \
    }

#define GENx___x___x900(_name) \
    { \
	&s370_operation_exception, \
	&s390_operation_exception, \
	&z900_ ## _name \
    }

#define GENx370x___x900(_name) \
    { \
	&s370_ ## _name, \
	&s390_operation_exception, \
	&z900_ ## _name \
    }

#define GENx___x390x900(_name) \
    { \
	&s370_operation_exception, \
	&s390_ ## _name, \
	&z900_ ## _name \
    }

#define GENx370x390x900(_name) \
    { \
	&s370_ ## _name, \
	&s390_ ## _name, \
	&z900_ ## _name \
    }


typedef ATTR_REGPARM(3) void (*zz_func) (BYTE inst[], int execflag, REGS *regs);

extern zz_func opcode_table[][GEN_MAXARCH];
extern zz_func opcode_01xx[][GEN_MAXARCH];
extern zz_func v_opcode_a4xx[][GEN_MAXARCH];
extern zz_func opcode_a5xx[][GEN_MAXARCH];
extern zz_func v_opcode_a5xx[][GEN_MAXARCH];
extern zz_func v_opcode_a6xx[][GEN_MAXARCH];
extern zz_func opcode_a7xx[][GEN_MAXARCH];
extern zz_func opcode_b2xx[][GEN_MAXARCH];
extern zz_func opcode_b3xx[][GEN_MAXARCH];
extern zz_func opcode_b9xx[][GEN_MAXARCH];
extern zz_func opcode_c0xx[][GEN_MAXARCH];
extern zz_func opcode_e3xx[][GEN_MAXARCH];
extern zz_func opcode_e4xx[][GEN_MAXARCH];
extern zz_func v_opcode_e4xx[][GEN_MAXARCH];
extern zz_func opcode_e5xx[][GEN_MAXARCH];
extern zz_func opcode_ebxx[][GEN_MAXARCH];
extern zz_func opcode_ecxx[][GEN_MAXARCH];
extern zz_func opcode_edxx[][GEN_MAXARCH];


#if defined(OPTION_INSTRUCTION_COUNTING)

#define COUNT_INST(_inst, _regs) \
do { \
int used; \
    switch((_inst)[0]) { \
	case 0x01: \
	    used = sysblk.imap01[(_inst)[1]]++; \
	    break; \
	case 0xA4: \
	    used = sysblk.imapa4[(_inst)[1]]++; \
	    break; \
	case 0xA5: \
	    used = sysblk.imapa5[(_inst)[1] & 0x0F]++; \
	    break; \
	case 0xA6: \
	    used = sysblk.imapa6[(_inst)[1]]++; \
	    break; \
	case 0xA7: \
	    used = sysblk.imapa7[(_inst)[1] & 0x0F]++; \
	    break; \
	case 0xB2: \
	    used = sysblk.imapb2[(_inst)[1]]++; \
	    break; \
	case 0xB3: \
	    used = sysblk.imapb3[(_inst)[1]]++; \
	    break; \
	case 0xB9: \
	    used = sysblk.imapb9[(_inst)[1]]++; \
	    break; \
	case 0xC0: \
	    used = sysblk.imapc0[(_inst)[1] & 0x0F]++; \
	    break; \
	case 0xE3: \
	    used = sysblk.imape3[(_inst)[5]]++; \
	    break; \
	case 0xE4: \
	    used = sysblk.imape4[(_inst)[1]]++; \
	    break; \
	case 0xE5: \
	    used = sysblk.imape5[(_inst)[1]]++; \
	    break; \
	case 0xEB: \
	    used = sysblk.imapeb[(_inst)[5]]++; \
	    break; \
	case 0xEC: \
	    used = sysblk.imapec[(_inst)[5]]++; \
	    break; \
	case 0xED: \
	    used = sysblk.imaped[(_inst)[5]]++; \
	    break; \
	default: \
	    used = sysblk.imapxx[(_inst)[0]]++; \
    } \
    if(!used) \
    { \
	logmsg("First use: "); \
	ARCH_DEP(display_inst) ((_regs), (_inst)); \
    } \
} while(0)

#else

#define COUNT_INST(_inst, _regs)

#endif


#define UNROLLED_EXECUTE(_regs) \
do { \
    (_regs)->instvalid = 0; \
    INSTRUCTION_FETCH((_regs)->inst, (_regs)->psw.IA, (_regs)); \
    (_regs)->instvalid = 1; \
    EXECUTE_INSTRUCTION ((_regs)->inst, 0, (_regs)); \
} while(0)


/* Main storage access locking 
   This routine will ensure that a given CPU 
   has exclusive access to main storage and hence
   will be able to perform as what appears as an
   interlocked update to other CPU's - Jan Jaeger */

/* OBTAIN_MAINLOCK() may cause a retry of the instruction */

#if MAX_CPU_ENGINES == 1
 #define OBTAIN_MAINLOCK(_regs)
 #define RELEASE_MAINLOCK(_regs)
 #define BROADCAST_PTLB(_regs) \
         ARCH_DEP(purge_tlb)((_regs))
 #define BROADCAST_PALB(_regs) \
         ARCH_DEP(purge_alb)((_regs))
 #define SYNCHRONIZE_BROADCAST(_regs)
#else
#define OBTAIN_MAINLOCK(_register_context) \
do { \
    pthread_mutex_lock(&sysblk.mainlock); \
    (_register_context)->mainlock = 1; \
} while(0)

#define RELEASE_MAINLOCK(_register_context) \
do { \
    (_register_context)->mainlock = 0; \
    pthread_mutex_unlock(&sysblk.mainlock); \
} while(0)

#define BROADCAST_PTLB(_regs) \
do { \
    pthread_mutex_lock(&sysblk.intlock); \
    sysblk.brdcstptlb++; \
    pthread_mutex_unlock(&sysblk.intlock); \
} while(0)

#define BROADCAST_PALB(_regs) \
do { \
    pthread_mutex_lock(&sysblk.intlock); \
    sysblk.brdcstpalb++; \
    pthread_mutex_unlock(&sysblk.intlock); \
} while(0)

#define SYNCHRONIZE_BROADCAST(_regs) \
do { \
    pthread_mutex_lock(&sysblk.intlock); \
    ARCH_DEP(synchronize_broadcast)((_regs)); \
    pthread_mutex_unlock(&sysblk.intlock); \
} while(0)
#endif


/* The footprint_buffer option saves a copy of the register context
   every time an instruction is executed.  This is for problem
   determination only, as it severely impacts performance.	 *JJ */

#if !defined(OPTION_FOOTPRINT_BUFFER)

#define EXECUTE_INSTRUCTION(_instruction, _execflag, _regs) \
do { \
    COUNT_INST((_instruction), (_regs)); \
    opcode_table[((_instruction)[0])][ARCH_MODE]((_instruction), (_execflag), (_regs)); \
} while(0)

#else /*defined(OPTION_FOOTPRINT_BUFFER)*/

#define EXECUTE_INSTRUCTION(_instruction, _execflag, _regs) \
do { \
    sysblk.footprregs[(_regs)->cpuad][sysblk.footprptr[(_regs)->cpuad]] = *(_regs); \
    memcpy(&sysblk.footprregs[(_regs)->cpuad][sysblk.footprptr[(_regs)->cpuad]++].inst,(_instruction),6); \
    sysblk.footprptr[(_regs)->cpuad] &= OPTION_FOOTPRINT_BUFFER - 1; \
    COUNT_INST((_instruction), (_regs)); \
    opcode_table[((_instruction)[0])][ARCH_MODE]((_instruction), (_execflag), (_regs)); \
} while(0)

#endif /*defined(OPTION_FOOTPRINT_BUFFER)*/

#if defined(OPTION_CPU_UNROLL)
#define RETURN_INTCHECK(_regs) \
        longjmp((_regs)->progjmp, SIE_NO_INTERCEPT)
#else
#define RETURN_INTCHECK(_regs) \
        return
#endif


#define ODD_CHECK(_r, _regs) \
	if( (_r) & 1 ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

#define ODD2_CHECK(_r1, _r2, _regs) \
	if( ((_r1) & 1) || ((_r2) & 1) ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

#define FW_CHECK(_value, _regs) \
	if( (_value) & 3 ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

#define DW_CHECK(_value, _regs) \
	if( (_value) & 7 ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

#define QW_CHECK(_value, _regs) \
	if( (_value) & 15 ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

#if defined(FEATURE_BASIC_FP_EXTENSIONS)

	/* Program check if r1 is not 0, 2, 4, or 6 */
#define HFPREG_CHECK(_r, _regs) \
	if( !((_regs)->CR(0) & CR0_AFP) ) { \
	    if( (_r) & 9 ) { \
                (_regs)->dxc = DXC_AFP_REGISTER; \
		ARCH_DEP(program_interrupt)( (_regs), PGM_DATA_EXCEPTION); \
	    } \
	}

	/* Program check if r1 and r2 are not 0, 2, 4, or 6 */
#define HFPREG2_CHECK(_r1, _r2, _regs) \
	if( !((_regs)->CR(0) & CR0_AFP) ) { \
	    if( ((_r1) & 9) || ((_r2) & 9) ) { \
                (_regs)->dxc = DXC_AFP_REGISTER; \
		ARCH_DEP(program_interrupt)( (_regs), PGM_DATA_EXCEPTION); \
	    } \
	}

	/* Program check if r1 is not 0 or 4 */
#define HFPODD_CHECK(_r, _regs) \
	if( (_r) & 2 ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION); \
	else if( !((_regs)->CR(0) & CR0_AFP) ) { \
	    if( (_r) & 9 ) { \
                (_regs)->dxc = DXC_AFP_REGISTER; \
		ARCH_DEP(program_interrupt)( (_regs), PGM_DATA_EXCEPTION); \
	    } \
	}

	/* Program check if r1 and r2 are not 0 or 4 */
#define HFPODD2_CHECK(_r1, _r2, _regs) \
	if( ((_r1) & 2) || ((_r2) & 2) ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION); \
	else if( !((_regs)->CR(0) & CR0_AFP) ) { \
	    if( ((_r1) & 9) || ((_r2) & 9) ) { \
                (_regs)->dxc = DXC_AFP_REGISTER; \
		ARCH_DEP(program_interrupt)( (_regs), PGM_DATA_EXCEPTION); \
	    } \
	}

	/* Convert fpr to index */
#define FPR2I(_r) \
	((_r) << 1)

	/* Offset of extended register */
#define FPREX 4

#else /*!defined(FEATURE_BASIC_FP_EXTENSIONS)*/

	/* Program check if r1 is not 0, 2, 4, or 6 */
#define HFPREG_CHECK(_r, _regs) \
	if( (_r) & 9 ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

	/* Program check if r1 and r2 are not 0, 2, 4, or 6 */
#define HFPREG2_CHECK(_r1, _r2, _regs) \
	if( ((_r1) & 9) || ((_r2) & 9) ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

	/* Program check if r1 is not 0 or 4 */
#define HFPODD_CHECK(_r, _regs) \
	if( (_r) & 11 ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

	/* Program check if r1 and r2 are not 0 or 4 */
#define HFPODD2_CHECK(_r1, _r2, _regs) \
	if( ((_r1) & 11) || ((_r2) & 11) ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

	/* Convert fpr to index */
#define FPR2I(_r) \
	(_r)

	/* Offset of extended register */
#define FPREX 2

#endif /*!defined(FEATURE_BASIC_FP_EXTENSIONS)*/

	/* Program check if m is not 0, 1, or 4 to 7 */
#define HFPM_CHECK(_m, _regs) \
	if (((_m) == 2) || ((_m) == 3) || ((_m) & 8)) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)

#define BFPINST_CHECK(_regs) \
        if( !((_regs)->CR(0) & CR0_AFP) ) { \
            (_regs)->dxc = DXC_BFP_INSTRUCTION; \
            ARCH_DEP(program_interrupt)( (_regs), PGM_DATA_EXCEPTION); \
        }

#define PRIV_CHECK(_regs) \
	if( (_regs)->psw.prob ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_PRIVILEGED_OPERATION_EXCEPTION)

#if defined(FEATURE_BINARY_FLOATING_POINT)
#define BFPREGPAIR2_CHECK(_r1, _r2, _regs) \
	if( ((_r1) & 2) || ((_r2) & 2) ) \
	    ARCH_DEP(program_interrupt)( (_regs), PGM_SPECIFICATION_EXCEPTION)
#endif /* defined(FEATURE_BINARY_FLOATING_POINT) */


#if __BYTE_ORDER == __LITTLE_ENDIAN
 #define CSWAP16(_x) bswap_16(_x)
 #define CSWAP32(_x) bswap_32(_x)
 #define CSWAP64(_x) bswap_64(_x)
#elif __BYTE_ORDER == __BIG_ENDIAN
 #define CSWAP16(_x) (_x)
 #define CSWAP32(_x) (_x)
 #define CSWAP64(_x) (_x)
#else
 #error Cannot determine byte order
#endif

#define STORE_HW(_storage, _value) \
	*((U16*)(_storage)) = CSWAP16((_value))

#define FETCH_HW(_value, _storage) \
	(_value) = CSWAP16(*((U16*)(_storage)))

#define STORE_FW(_storage, _value) \
	*((U32*)(_storage)) = CSWAP32((_value))

#define FETCH_FW(_value, _storage) \
	(_value) = CSWAP32(*((U32*)(_storage)))

#define STORE_DW(_storage, _value) \
	*((U64*)(_storage)) = CSWAP64((_value))

#define FETCH_DW(_value, _storage) \
	(_value) = CSWAP64(*((U64*)(_storage)))


#endif /*!defined(_OPCODE_H)*/


#if defined(OPTION_AIA_BUFFER)

#undef	INSTRUCTION_FETCH
#undef	INVALIDATE_AIA

#define INSTRUCTION_FETCH(_dest, _addr, _regs) \
do { \
    if( (_regs)->VI == ((_addr) & PAGEFRAME_PAGEMASK) \
      && ((_addr) & PAGEFRAME_BYTEMASK) <= PAGEFRAME_PAGESIZE - 6) \
    { \
        if((_addr) & 0x01) \
            ARCH_DEP(program_interrupt)((_regs), PGM_SPECIFICATION_EXCEPTION); \
        memcpy ((_dest), sysblk.mainstor + (_regs)->AI + \
				    ((_addr) & PAGEFRAME_BYTEMASK) , 6); \
    } \
    else \
        ARCH_DEP(instfetch) ((_dest), (_addr), (_regs)); \
} while(0)

#define INVALIDATE_AIA(_regs) \
    (_regs)->VI = 1

#else /*!defined(OPTION_AIA_BUFFER)*/

#define INSTRUCTION_FETCH(_dest, _addr, _regs) \
    ARCH_DEP(instfetch) ((_dest), (_addr), (_regs))

#define INVALIDATE_AIA(_regs)

#endif /*!defined(OPTION_AIA_BUFFER)*/


#if defined(OPTION_AEA_BUFFER)

#define LOGICAL_TO_ABS(_addr, _arn, _regs, _acctype, _akey)	      \
    (((_addr) & PAGEFRAME_PAGEMASK) == (_regs)->VE((_arn))) &&	    \
    ((_arn) >= 0) &&						      \
    (((_regs)->aekey[(_arn)] == (_akey)) || (_akey) == 0) &&	      \
    ((_regs)->aeacc[(_arn)] >= (_acctype)) ?			      \
    ((_acctype) == ACCTYPE_READ) ?				      \
    (STORAGE_KEY((_regs)->AE((_arn))) |= STORKEY_REF,		      \
	((_regs)->AE((_arn)) | ((_addr) & PAGEFRAME_BYTEMASK)) ) :  \
    (STORAGE_KEY((_regs)->AE((_arn))) |= (STORKEY_REF | STORKEY_CHANGE), \
	((_regs)->AE((_arn)) | ((_addr) & PAGEFRAME_BYTEMASK)) ) :  \
    ARCH_DEP(logical_to_abs) ((_addr), (_arn), (_regs), (_acctype), (_akey))

#define INVALIDATE_AEA(_arn, _regs) \
	(_regs)->VE((_arn)) = 1

#define INVALIDATE_AEA_ALL(_regs) \
do { \
    int i; \
    for(i = 0; i < 16; i++) \
        (_regs)->VE(i) = 1; \
} while(0)

#else /*!defined(OPTION_AEA_BUFFER)*/

#define LOGICAL_TO_ABS(_addr, _arn, _regs, _acctype, _akey) \
	ARCH_DEP(logical_to_abs) ((_addr), (_arn), (_regs), (_acctype), (_akey))

#define INVALIDATE_AEA(_arn, _regs)

#define INVALIDATE_AEA_ALL(_regs)

#endif /*!defined(OPTION_AEA_BUFFER)*/


/* E implied operands and extended op code */
#undef E
#define E(_inst, _execflag, _regs) \
	{ \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 2; \
		(_regs)->psw.IA += 2; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RR register to register */
#undef RR
#define RR(_inst, _execflag, _regs, _r1, _r2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_r2) = (_inst)[1] & 0x0F; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 2; \
		(_regs)->psw.IA += 2; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RR special format for SVC instruction */
#undef RR_SVC
#define RR_SVC(_inst, _execflag, _regs, _svc) \
	{ \
	    (_svc) = (_inst)[1]; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 2; \
		(_regs)->psw.IA += 2; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RRE register to register with extended op code */
#undef RRE
#define RRE(_inst, _execflag, _regs, _r1, _r2) \
	{ \
	    (_r1) = (_inst)[3] >> 4; \
	    (_r2) = (_inst)[3] & 0x0F; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RRF register to register with additional R3 field */
#undef RRF_R
#define RRF_R(_inst, _execflag, _regs, _r1, _r2, _r3) \
	{ \
	    (_r1) = (_inst)[2] >> 4; \
	    (_r3) = (_inst)[3] >> 4; \
	    (_r2) = (_inst)[3] & 0x0F; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RRF register to register with additional M3 field */
#undef RRF_M
#define RRF_M(_inst, _execflag, _regs, _r1, _r2, _m3) \
	{ \
	    (_m3) = (_inst)[2] >> 4; \
	    (_r1) = (_inst)[3] >> 4; \
	    (_r2) = (_inst)[3] & 0x0F; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RRF register to register with additional R3 and M4 fields */
#undef RRF_RM
#define RRF_RM(_inst, _execflag, _regs, _r1, _r2, _r3, _m4) \
	{ \
	    (_r3) = (_inst)[2] >> 4; \
	    (_m4) = (_inst)[2] & 0x0F; \
	    (_r1) = (_inst)[3] >> 4; \
	    (_r2) = (_inst)[3] & 0x0F; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RX register and indexed storage */
#undef RX
#define RX(_inst, _execflag, _regs, _r1, _b2, _effective_addr2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_b2) = (_inst)[1] & 0x0F; \
	    (_effective_addr2) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    (_b2) = (_inst)[2] >> 4; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RXE register and indexed storage with extended op code */
#undef RXE
#define RXE(_inst, _execflag, _regs, _r1, _b2, _effective_addr2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_b2) = (_inst)[1] & 0x0F; \
	    (_effective_addr2) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    (_b2) = (_inst)[2] >> 4; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 6; \
		(_regs)->psw.IA += 6; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RXF register and indexed storage with ext.opcode and additional R3 */
#undef RXF
#define RXF(_inst, _execflag, _regs, _r1, _r3, _b2, _effective_addr2) \
	{ \
	    (_r1) = (_inst)[4] >> 4; \
	    (_r3) = (_inst)[1] >> 4; \
	    (_b2) = (_inst)[1] & 0x0F; \
	    (_effective_addr2) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    (_b2) = (_inst)[2] >> 4; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 6; \
		(_regs)->psw.IA += 6; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RS register and storage with additional R3 or M3 field */
#undef RS
#define RS(_inst, _execflag, _regs, _r1, _r3, _b2, _effective_addr2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_r3) = (_inst)[1] & 0x0F; \
	    (_b2) = (_inst)[2] >> 4; \
	    (_effective_addr2) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RSE register and storage with extended op code and additional
   R3 or M3 field (note, this is NOT the ESA/390 vector RSE format) */
#undef RSE
#define RSE(_inst, _execflag, _regs, _r1, _r3, _b2, _effective_addr2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_r3) = (_inst)[1] & 0x0F; \
	    (_b2) = (_inst)[2] >> 4; \
	    (_effective_addr2) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 6; \
		(_regs)->psw.IA += 6; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RSL storage operand with extended op code and 4-bit L field */
#undef RSL
#define RSL(_inst, _execflag, _regs, _l1, _b1, _effective_addr1) \
        { \
            (_l1) = (_inst)[1] >> 4; \
            (_b1) = (_inst)[2] >> 4; \
            (_effective_addr1) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
            if((_b1) != 0) \
            { \
                (_effective_addr1) += (_regs)->GR((_b1)); \
                (_effective_addr1) &= ADDRESS_MAXWRAP((_regs)); \
            } \
            if( !(_execflag) ) \
            { \
                (_regs)->psw.ilc = 6; \
                (_regs)->psw.IA += 6; \
                (_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
            } \
        }

/* RSI register and immediate with additional R3 field */
#undef RSI
#define RSI(_inst, _execflag, _regs, _r1, _r3, _i2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_r3) = (_inst)[1] & 0x0F; \
	    (_i2) = ((_inst)[2] << 8) | (_inst)[3]; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RI register and immediate with extended 4-bit op code */
#undef RI
#define RI(_inst, _execflag, _regs, _r1, _op, _i2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_op) = (_inst)[1] & 0x0F; \
	    (_i2) = ((_inst)[2] << 8) | (_inst)[3]; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RIE register and immediate with ext.opcode and additional R3 */
#undef RIE
#define RIE(_inst, _execflag, _regs, _r1, _r3, _i2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_r3) = (_inst)[1] & 0x0F; \
	    (_i2) = ((_inst)[2] << 8) | (_inst)[3]; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 6; \
		(_regs)->psw.IA += 6; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* RIL register and longer immediate with extended 4 bit op code */
#undef RIL
#define RIL(_inst, _execflag, _regs, _r1, _op, _i2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_op) = (_inst)[1] & 0x0F; \
	    (_i2) = ((_inst)[2] << 24) \
		  | ((_inst)[3] << 16) \
		  | ((_inst)[4] << 8) \
		  | (_inst)[5]; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 6; \
		(_regs)->psw.IA += 6; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* SI storage and immediate */
#undef SI
#define SI(_inst, _execflag, _regs, _i2, _b1, _effective_addr1) \
	{ \
	    (_i2) = (_inst)[1]; \
	    (_b1) = (_inst)[2] >> 4; \
	    (_effective_addr1) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b1) != 0) \
	    { \
		(_effective_addr1) += (_regs)->GR((_b1)); \
		(_effective_addr1) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* S storage operand only */
#undef S
#define S(_inst, _execflag, _regs, _b2, _effective_addr2) \
	{ \
	    (_b2) = (_inst)[2] >> 4; \
	    (_effective_addr2) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* SS storage to storage with two 4-bit L or R fields */
#undef SS
#define SS(_inst, _execflag, _regs, _r1, _r3, \
	    _b1, _effective_addr1, _b2, _effective_addr2) \
	{ \
	    (_r1) = (_inst)[1] >> 4; \
	    (_r3) = (_inst)[1] & 0x0F; \
	    (_b1) = (_inst)[2] >> 4; \
	    (_effective_addr1) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b1) != 0) \
	    { \
		(_effective_addr1) += (_regs)->GR((_b1)); \
		(_effective_addr1) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    (_b2) = (_inst)[4] >> 4; \
	    (_effective_addr2) = (((_inst)[4] & 0x0F) << 8) | (_inst)[5]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 6; \
		(_regs)->psw.IA += 6; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* SS storage to storage with one 8-bit L field */
#undef SS_L
#define SS_L(_inst, _execflag, _regs, _l, \
	    _b1, _effective_addr1, _b2, _effective_addr2) \
	{ \
	    (_l) = (_inst)[1]; \
	    (_b1) = (_inst)[2] >> 4; \
	    (_effective_addr1) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b1) != 0) \
	    { \
		(_effective_addr1) += (_regs)->GR((_b1)); \
		(_effective_addr1) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    (_b2) = (_inst)[4] >> 4; \
	    (_effective_addr2) = (((_inst)[4] & 0x0F) << 8) | (_inst)[5]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 6; \
		(_regs)->psw.IA += 6; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* SSE storage to storage with extended op code */
#undef SSE
#define SSE(_inst, _execflag, _regs, _b1, _effective_addr1, \
				     _b2, _effective_addr2) \
	{ \
	    (_b1) = (_inst)[2] >> 4; \
	    (_effective_addr1) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b1) != 0) \
	    { \
		(_effective_addr1) += (_regs)->GR((_b1)); \
		(_effective_addr1) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    (_b2) = (_inst)[4] >> 4; \
	    (_effective_addr2) = (((_inst)[4] & 0x0F) << 8) | (_inst)[5]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 6; \
		(_regs)->psw.IA += 6; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}


#undef SIE_TRANSLATE_ADDR
#undef SIE_LOGICAL_TO_ABS
#undef SIE_INTERCEPT
#undef SIE_TRANSLATE
#undef SIE_ONLY_INSTRUCTION


#if defined(_FEATURE_SIE)

#if !defined(_GEN_ARCH) || _GEN_ARCH == 900 || (_GEN_ARCH == 390 && !defined(_FEATURE_ZSIE))

#define SIE_TRANSLATE_ADDR(_parms...) \
	ARCH_DEP(translate_addr)(_parms)

#define SIE_LOGICAL_TO_ABS(_parms...) \
	ARCH_DEP(logical_to_abs)(_parms)

#else /*_GEN_ARCH == 390 || _GEN_ARCH == 370*/

#define SIE_TRANSLATE_ADDR(_addr, _arn, _regs, _parms...)	\
	( ((_regs)->arch_mode == ARCH_390) ?			\
	s390_translate_addr((_addr), (_arn), (_regs), _parms) : \
	z900_translate_addr((_addr), (_arn), (_regs), _parms) )

#define SIE_LOGICAL_TO_ABS(_addr, _arn, _regs, _parms...)	\
	( ((_regs)->arch_mode == ARCH_390) ?			\
	s390_logical_to_abs((_addr), (_arn), (_regs), _parms) : \
	z900_logical_to_abs((_addr), (_arn), (_regs), _parms) )

#endif

#define SIE_INTERCEPT(_regs) \
do { \
    if((_regs)->sie_state) \
	longjmp((_regs)->progjmp, SIE_INTERCEPT_INST); \
} while(0)

#define SIE_TRANSLATE(_addr, _acctype, _regs) \
do { \
    if((_regs)->sie_state && !(_regs)->sie_pref) \
	*(_addr) = SIE_LOGICAL_TO_ABS ((_regs)->sie_mso + *(_addr), \
	  USE_PRIMARY_SPACE, (_regs)->hostregs, (_acctype), 0); \
} while(0)

#define SIE_ONLY_INSTRUCTION(_regs) \
    if(!(_regs)->sie_state) \
	ARCH_DEP(program_interrupt)((_regs), PGM_OPERATION_EXCEPTION);

#else /*!defined(_FEATURE_SIE)*/

#define SIE_TRANSLATE_ADDR(_parms...)
#define SIE_LOGICAL_TO_ABS(_parms...)
#define SIE_INTERCEPT(_parms...)
#define SIE_TRANSLATE(_parms...)
#define SIE_ONLY_INSTRUCTION(_regs) \
	ARCH_DEP(program_interrupt)((_regs), PGM_OPERATION_EXCEPTION)

#endif /*!defined(_FEATURE_SIE)*/


#undef SIE_MODE_XC_OPEX
#undef SIE_MODE_XC_SOPEX

#if defined(FEATURE_MULTIPLE_CONTROLLED_DATA_SPACE)

#define SIE_MODE_XC_OPEX(_regs) \
	if(((_regs)->sie_state && ((_regs)->siebk->mx & SIE_MX_XC))) \
	    ARCH_DEP(program_interrupt)((_regs), PGM_OPERATION_EXCEPTION)
#define SIE_MODE_XC_SOPEX(_regs) \
	if(((_regs)->sie_state && ((_regs)->siebk->mx & SIE_MX_XC))) \
	    ARCH_DEP(program_interrupt)((_regs), PGM_SPECIAL_OPERATION_EXCEPTION)

#else /*!defined(FEATURE_MULTIPLE_CONTROLLED_DATA_SPACE)*/

#define SIE_MODE_XC_OPEX(_regs)
#define SIE_MODE_XC_SOPEX(_regs)

#endif /*!defined(FEATURE_MULTIPLE_CONTROLLED_DATA_SPACE)*/


#if defined(FEATURE_CRYPTO)

#if !defined(_CRDEFS)

#define _CRDEFS

#define COP_CHECK(_regs) \
	if(!((_regs)->CR(0) & CR0_CRYPTO) ) \
	    ARCH_DEP(program_interrupt)((_regs), PGM_CRYPTO_OPERATION_EXCEPTION)

#endif /*!defined(_CRDEFS)*/

#endif /*defined(FEATURE_CRYPTO)*/


#if defined(FEATURE_VECTOR_FACILITY)

#if !defined(_VFDEFS)

#define _VFDEFS

#define VOP_CHECK(_regs) \
	if(!((_regs)->CR(0) & CR0_VOP) || !(_regs)->vf->online) \
	    ARCH_DEP(program_interrupt)((_regs), PGM_VECTOR_OPERATION_EXCEPTION)

#define VR_INUSE(_vr, _regs) \
	((_regs)->vf->vsr & (VSR_VIU0 >> ((_vr) >> 1)))

#define VR_CHANGED(_vr, _regs) \
	((_regs)->vf->vsr & (VSR_VCH0 >> ((_vr) >> 1)))

#define SET_VR_INUSE(_vr, _regs) \
	(_regs)->vf->vsr |= (VSR_VIU0 >> ((_vr) >> 1))

#define SET_VR_CHANGED(_vr, _regs) \
	(_regs)->vf->vsr |= (VSR_VCH0 >> ((_vr) >> 1))

#define RESET_VR_INUSE(_vr, _regs) \
	(_regs)->vf->vsr &= ~(VSR_VIU0 >> ((_vr) >> 1))

#define RESET_VR_CHANGED(_vr, _regs) \
	(_regs)->vf->vsr &= ~(VSR_VCH0 >> ((_vr) >> 1))

#define VMR_SET(_section, _regs) \
	((_regs)->vf->vmr[(_section) >> 3] & (0x80 >> ((_section) & 7)))

#define MASK_MODE(_regs) \
	((_regs)->vf->vsr & VSR_M)

#define VECTOR_COUNT(_regs) \
	    (((_regs)->vf->vsr & VSR_VCT) >> 32)

#define VECTOR_IX(_regs) \
	    (((_regs)->vf->vsr & VSR_VIX) >> 16)

#endif /*!defined(_VFDEFS)*/

/* VST and QST formats are the same */
#undef VST
#define VST(_inst, _execflag, _regs, _vr3, _rt2, _vr1, _rs2) \
	{ \
	    (_qr3) = (_inst)[2] >> 4; \
	    (_rt2) = (_inst)[2] & 0x0F; \
	    (_vr1) = (_inst)[3] >> 4; \
	    (_rs2) = (_inst)[3] & 0x0F; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* VR, VV and QV formats are the same */
#undef VR
#define VR(_inst, _execflag, _regs, _qr3, _vr1, _vr2) \
	{ \
	    (_qr3) = (_inst)[2] >> 4; \
	    (_vr1) = (_inst)[3] >> 4; \
	    (_vr2) = (_inst)[3] & 0x0F; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

#undef VS
#define VS(_inst, _execflag, _regs, _rs2) \
	{ \
	    (_rs2) = (_inst)[3] & 0x0F; \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* The RSE vector instruction format of ESA/390 is referred to as
   VRSE to avoid conflict with the ESAME RSE instruction format */
#undef VRSE
#define VRSE(_inst, _execflag, _regs, _r3, _vr1, \
				     _b2, _effective_addr2) \
	{ \
	    (_r3) = (_inst)[2] >> 4; \
	    (_vr1) = (_inst)[3] >> 4; \
	    (_b2) = (_inst)[4] >> 4; \
	    (_effective_addr2) = (((_inst)[4] & 0x0F) << 8) | (_inst)[5]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
		(_effective_addr2) &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 6; \
		(_regs)->psw.IA += 6; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

/* S format instructions where the effective address does not wrap */
#undef S_NW
#define S_NW(_inst, _execflag, _regs, _b2, _effective_addr2) \
	{ \
	    (_b2) = (_inst)[2] >> 4; \
	    (_effective_addr2) = (((_inst)[2] & 0x0F) << 8) | (_inst)[3]; \
	    if((_b2) != 0) \
	    { \
		(_effective_addr2) += (_regs)->GR((_b2)); \
	    } \
	    if( !(_execflag) ) \
	    { \
		(_regs)->psw.ilc = 4; \
		(_regs)->psw.IA += 4; \
		(_regs)->psw.IA &= ADDRESS_MAXWRAP((_regs)); \
	    } \
	}

#endif /*defined(FEATURE_VECTOR_FACILITY)*/


#undef PERFORM_SERIALIZATION
#if MAX_CPU_ENGINES > 1 && defined(SMP_SERIALIZATION)
	/* In order to syncronize mainstorage access we need to flush
	   the cache on all processors, this needs special case when
	   running on an SMP machine, as the physical CPU's actually
	   need to perform this function.  This is accomplished by
	   obtaining and releasing a mutex lock, which is intended to
	   serialize storage access */
#define PERFORM_SERIALIZATION(_regs) \
	{ \
	    obtain_lock(&regs->serlock); \
	    release_lock(&regs->serlock); \
	}
#else  /*!SERIALIZATION*/
#define PERFORM_SERIALIZATION(_regs)
#endif /*SERIALIZATION*/


#define PERFORM_CHKPT_SYNC(_regs)


/* Functions in module channel.c */
int  ARCH_DEP(startio) (DEVBLK *dev, ORB *orb); 	       /*@IZW*/
void *s370_execute_ccw_chain (DEVBLK *dev);
void *s390_execute_ccw_chain (DEVBLK *dev);
void *z900_execute_ccw_chain (DEVBLK *dev);
int  stchan_id (REGS *regs, U16 chan);
int  testch (REGS *regs, U16 chan);
int  testio (REGS *regs, DEVBLK *dev, BYTE ibyte);
int  test_subchan (REGS *regs, DEVBLK *dev, IRB *irb);
void clear_subchan (REGS *regs, DEVBLK *dev);
int  halt_subchan (REGS *regs, DEVBLK *dev);
int  haltio (REGS *regs, DEVBLK *dev, BYTE ibyte);
int  resume_subchan (REGS *regs, DEVBLK *dev);
int  ARCH_DEP(present_io_interrupt) (REGS *regs, U32 *ioid,
        U32 *ioparm, U32 *iointid, BYTE *csw);
void io_reset (void);
int  chp_reset(BYTE chpid);
int  device_attention (DEVBLK *dev, BYTE unitstat);
int  ARCH_DEP(device_attention) (DEVBLK *dev, BYTE unitstat);


/* Functions in module cpu.c */
#if defined(_FEATURE_SIE)
void s370_store_psw (REGS *regs, BYTE *addr);
int  s370_load_psw (REGS *regs, BYTE *addr);
#endif /*defined(_FEATURE_SIE)*/
#if defined(_FEATURE_ZSIE)
int  s390_load_psw (REGS *regs, BYTE *addr);
void s390_store_psw (REGS *regs, BYTE *addr);
#endif /*defined(_FEATURE_ZSIE)*/
void ARCH_DEP(store_psw) (REGS *regs, BYTE *addr);
int  ARCH_DEP(load_psw) (REGS *regs, BYTE *addr);
#if defined(_FEATURE_SIE)
void s370_program_interrupt (REGS *regs, int code);
#endif /*!defined(_FEATURE_SIE)*/
#if defined(_FEATURE_ZSIE)
void s390_program_interrupt (REGS *regs, int code);
#endif /*!defined(_FEATURE_ZSIE)*/
void ARCH_DEP(program_interrupt) (REGS *regs, int code);
void *cpu_thread (REGS *regs);
void store_psw (REGS *regs, BYTE *addr);
void display_psw (REGS *regs);


/* Functions in module vm.c */
int  ARCH_DEP(diag_devtype) (int r1, int r2, REGS *regs);
int  ARCH_DEP(syncblk_io) (int r1, int r2, REGS *regs);
int  ARCH_DEP(syncgen_io) (int r1, int r2, REGS *regs);
void ARCH_DEP(extid_call) (int r1, int r2, REGS *regs);
int  ARCH_DEP(cpcmd_call) (int r1, int r2, REGS *regs);
void ARCH_DEP(pseudo_timer) (U32 code, int r1, int r2, REGS *regs);
void ARCH_DEP(access_reipl_data) (int r1, int r2, REGS *regs);
int  ARCH_DEP(diag_ppagerel) (int r1, int r2, REGS *regs);


/* Functions in module diagnose.c */
void ARCH_DEP(diagnose_call) (U32 effective_addr2, int r1, int r3,
	REGS *regs);


/* Functions in module diagmssf.c */
void ARCH_DEP(scpend_call) (void);
int  ARCH_DEP(mssf_call) (int r1, int r2, REGS *regs);
void ARCH_DEP(diag204_call) (int r1, int r2, REGS *regs);


/* Functions in module external.c */
void ARCH_DEP(synchronize_broadcast) (REGS *regs);
void ARCH_DEP(perform_external_interrupt) (REGS *regs);
void ARCH_DEP(store_status) (REGS *ssreg, RADR aaddr);
void store_status (REGS *ssreg, U64 aaddr);


/* Functions in module ipl.c */
int  load_ipl (U16 devnum, REGS *regs);
int  ARCH_DEP(load_ipl) (U16 devnum, REGS *regs);
void ARCH_DEP(cpu_reset) (REGS *regs);
void initial_cpu_reset (REGS *regs);
void ARCH_DEP(initial_cpu_reset) (REGS *regs);


/* Functions in module machchk.c */
int  ARCH_DEP(present_mck_interrupt) (REGS *regs, U64 *mcic, U32 *xdmg,
	RADR *fsta);
U32  channel_report (void);
void machine_check_crwpend (void);


/* Functions in module panel.c */
void ARCH_DEP(display_inst) (REGS *regs, BYTE *inst);


/* Functions in module sie.c */
#if defined(_FEATURE_SIE)
int  s370_sie_run (REGS *regs);
#endif /*!defined(_FEATURE_SIE)*/
#if defined(_FEATURE_ZSIE)
int  s390_sie_run (REGS *regs);
#endif /*!defined(_FEATURE_ZSIE)*/
int  ARCH_DEP(sie_run) (REGS *regs);
void ARCH_DEP(sie_exit) (REGS *regs, int code);


/* Functions in module stack.c */
void ARCH_DEP(trap_x) (int trap_is_trap4, int execflag, REGS *regs, U32 trap_operand);
RADR ARCH_DEP(abs_stack_addr) (VADR vaddr, REGS *regs, int acctype);
void ARCH_DEP(form_stack_entry) (BYTE etype, VADR retna, VADR calla,
	U32 csi, U32 pcnum, REGS *regs);
VADR ARCH_DEP(locate_stack_entry) (int prinst, LSED *lsedptr,
	REGS *regs);
void ARCH_DEP(stack_modify) (VADR lsea, U32 m1, U32 m2, REGS *regs);
void ARCH_DEP(stack_extract) (VADR lsea, int r1, int code, REGS *regs);
void ARCH_DEP(unstack_registers) (int gtype, VADR lsea, int r1,
	int r2, REGS *regs);
int  ARCH_DEP(program_return_unstack) (REGS *regs, RADR *lsedap);


/* Functions in module trace.c */
CREG  ARCH_DEP(trace_br) (int amode, VADR ia, REGS *regs);
CREG  ARCH_DEP(trace_bsg) (U32 alet, VADR ia, REGS *regs);
CREG  ARCH_DEP(trace_ssar) (U16 sasn, REGS *regs);
CREG  ARCH_DEP(trace_pc) (U32 pcnum, REGS *regs);
CREG  ARCH_DEP(trace_pr) (REGS *newregs, REGS *regs);
CREG  ARCH_DEP(trace_pt) (U16 pasn, GREG gpr2, REGS *regs);


/* Functions in module plo.c */
int ARCH_DEP(plo_cl) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_clg) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_clgr) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_clx) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_cs) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csg) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csgr) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csx) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_dcs) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_dcsg) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_dcsgr) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_dcsx) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csst) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csstg) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csstgr) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csstx) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csdst) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csdstg) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csdstgr) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_csdstx) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_cstst) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_cststg) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_cststgr) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);
int ARCH_DEP(plo_cststx) (int r1, int r3, VADR effective_addr2, int b2,
                            VADR effective_addr4, int b4,  REGS *regs);


/* Instruction functions in opcode.c */
DEF_INST(execute_01xx);
DEF_INST(execute_a4xx);
DEF_INST(execute_a5xx);
DEF_INST(execute_a6xx);
DEF_INST(execute_a7xx);
DEF_INST(execute_b2xx);
DEF_INST(execute_b3xx);
DEF_INST(execute_b9xx);
DEF_INST(execute_c0xx);
DEF_INST(execute_e3xx);
DEF_INST(execute_e4xx);
DEF_INST(execute_e5xx);
DEF_INST(execute_ebxx);
DEF_INST(execute_ecxx);
DEF_INST(execute_edxx);
DEF_INST(operation_exception);
DEF_INST(dummy_instruction);


/* Instructions in assist.c */
DEF_INST(svc_assist);
DEF_INST(obtain_local_lock);
DEF_INST(release_local_lock);
DEF_INST(obtain_cms_lock);
DEF_INST(release_cms_lock);
DEF_INST(trace_svc_interruption);
DEF_INST(trace_program_interruption);
DEF_INST(trace_initial_srb_dispatch);
DEF_INST(trace_io_interruption);
DEF_INST(trace_task_dispatch);
DEF_INST(trace_svc_return);


/* Instructions in cmpsc.c */
DEF_INST(compression_call);


/* Instructions in control.c */
DEF_INST(branch_and_set_authority);
DEF_INST(branch_in_subspace_group);
DEF_INST(branch_and_stack);
DEF_INST(compare_and_swap_and_purge);
DEF_INST(diagnose);
DEF_INST(extract_primary_asn);
DEF_INST(extract_secondary_asn);
DEF_INST(extract_stacked_registers);
DEF_INST(extract_stacked_state);
DEF_INST(insert_address_space_control);
DEF_INST(insert_psw_key);
DEF_INST(insert_storage_key);
DEF_INST(insert_storage_key_extended);
DEF_INST(insert_virtual_storage_key);
DEF_INST(invalidate_page_table_entry);
DEF_INST(load_address_space_parameters);
DEF_INST(load_control);
DEF_INST(load_program_status_word);
DEF_INST(load_real_address);
DEF_INST(load_using_real_address);
DEF_INST(lock_page);
DEF_INST(modify_stacked_state);
DEF_INST(move_to_primary);
DEF_INST(move_to_secondary);
DEF_INST(move_with_destination_key);
DEF_INST(move_with_key);
DEF_INST(move_with_source_key);
DEF_INST(program_call);
DEF_INST(program_return);
DEF_INST(program_transfer);
DEF_INST(purge_accesslist_lookaside_buffer);
DEF_INST(purge_translation_lookaside_buffer);
DEF_INST(reset_reference_bit);
DEF_INST(reset_reference_bit_extended);
DEF_INST(set_address_space_control_x);
DEF_INST(set_clock);
DEF_INST(set_clock_comparator);
DEF_INST(set_clock_programmable_field);
DEF_INST(set_cpu_timer);
DEF_INST(set_prefix);
DEF_INST(set_psw_key_from_address);
DEF_INST(set_secondary_asn);
DEF_INST(set_storage_key);
DEF_INST(set_storage_key_extended);
DEF_INST(set_system_mask);
DEF_INST(signal_procesor);
DEF_INST(store_clock_comparator);
DEF_INST(store_control);
DEF_INST(store_cpu_address);
DEF_INST(store_cpu_id);
DEF_INST(store_cpu_timer);
DEF_INST(store_prefix);
DEF_INST(store_system_information);
DEF_INST(store_then_and_system_mask);
DEF_INST(store_then_or_system_mask);
DEF_INST(store_using_real_address);
DEF_INST(test_access);
DEF_INST(test_block);
DEF_INST(test_protection);
DEF_INST(trace);


/* Instructions in decimal.c */
DEF_INST(add_decimal);
DEF_INST(compare_decimal);
DEF_INST(divide_decimal);
DEF_INST(edit_x_edit_and_mark);
DEF_INST(multiply_decimal);
DEF_INST(shift_and_round_decimal);
DEF_INST(subtract_decimal);
DEF_INST(zero_and_add);
DEF_INST(test_decimal);


/* Instructions in vm.c */
DEF_INST(inter_user_communication_vehicle);


/* Instructions in sie.c */
DEF_INST(start_interpretive_execution);


/* Instructions in float.c */
DEF_INST(load_positive_float_long_reg);
DEF_INST(load_negative_float_long_reg);
DEF_INST(load_and_test_float_long_reg);
DEF_INST(load_complement_float_long_reg);
DEF_INST(halve_float_long_reg);
DEF_INST(round_float_long_reg);
DEF_INST(multiply_float_ext_reg);
DEF_INST(multiply_float_long_to_ext_reg);
DEF_INST(load_float_long_reg);
DEF_INST(compare_float_long_reg);
DEF_INST(add_float_long_reg);
DEF_INST(subtract_float_long_reg);
DEF_INST(multiply_float_long_reg);
DEF_INST(divide_float_long_reg);
DEF_INST(add_unnormal_float_long_reg);
DEF_INST(subtract_unnormal_float_long_reg);
DEF_INST(load_positive_float_short_reg);
DEF_INST(load_negative_float_short_reg);
DEF_INST(load_and_test_float_short_reg);
DEF_INST(load_complement_float_short_reg);
DEF_INST(halve_float_short_reg);
DEF_INST(round_float_short_reg);
DEF_INST(add_float_ext_reg);
DEF_INST(subtract_float_ext_reg);
DEF_INST(load_float_short_reg);
DEF_INST(compare_float_short_reg);
DEF_INST(add_float_short_reg);
DEF_INST(subtract_float_short_reg);
DEF_INST(multiply_float_short_to_long_reg);
DEF_INST(divide_float_short_reg);
DEF_INST(add_unnormal_float_short_reg);
DEF_INST(subtract_unnormal_float_short_reg);
DEF_INST(store_float_long);
DEF_INST(multiply_float_long_to_ext);
DEF_INST(load_float_long);
DEF_INST(compare_float_long);
DEF_INST(add_float_long);
DEF_INST(subtract_float_long);
DEF_INST(multiply_float_long);
DEF_INST(divide_float_long);
DEF_INST(add_unnormal_float_long);
DEF_INST(subtract_unnormal_float_long);
DEF_INST(store_float_short);
DEF_INST(load_float_short);
DEF_INST(compare_float_short);
DEF_INST(add_float_short);
DEF_INST(subtract_float_short);
DEF_INST(multiply_float_short_to_long);
DEF_INST(divide_float_short);
DEF_INST(add_unnormal_float_short);
DEF_INST(subtract_unnormal_float_short);
DEF_INST(divide_float_ext_reg);
DEF_INST(squareroot_float_long_reg);
DEF_INST(squareroot_float_short_reg);
DEF_INST(loadlength_float_short_to_long_reg);
DEF_INST(loadlength_float_long_to_ext_reg);
DEF_INST(loadlength_float_short_to_ext_reg);
DEF_INST(squareroot_float_ext_reg);
DEF_INST(multiply_float_short_reg);
DEF_INST(load_positive_float_ext_reg);
DEF_INST(load_negative_float_ext_reg);
DEF_INST(load_and_test_float_ext_reg);
DEF_INST(load_complement_float_ext_reg);
DEF_INST(round_float_ext_to_short_reg);
DEF_INST(load_fp_int_float_ext_reg);
DEF_INST(compare_float_ext_reg);
DEF_INST(load_fp_int_float_short_reg);
DEF_INST(load_fp_int_float_long_reg);
DEF_INST(convert_fixed_to_float_short_reg);
DEF_INST(convert_fixed_to_float_long_reg);
DEF_INST(convert_fixed_to_float_ext_reg);
DEF_INST(convert_float_short_to_fixed_reg);
DEF_INST(convert_float_long_to_fixed_reg);
DEF_INST(convert_float_ext_to_fixed_reg);
DEF_INST(loadlength_float_short_to_long);
DEF_INST(loadlength_float_long_to_ext);
DEF_INST(loadlength_float_short_to_ext);
DEF_INST(squareroot_float_short);
DEF_INST(squareroot_float_long);
DEF_INST(multiply_float_short);


/* Instructions in general.c */
DEF_INST(add_register);
DEF_INST(add);
DEF_INST(add_halfword);
DEF_INST(add_halfword_immediate);
DEF_INST(add_logical_register);
DEF_INST(add_logical);
DEF_INST(and_register);
DEF_INST(and);
DEF_INST(and_immediate);
DEF_INST(and_character);
DEF_INST(branch_and_link_register);
DEF_INST(branch_and_link);
DEF_INST(branch_and_save_register);
DEF_INST(branch_and_save);
DEF_INST(branch_and_save_and_set_mode);
DEF_INST(branch_and_set_mode);
DEF_INST(branch_on_condition_register);
DEF_INST(branch_on_condition);
DEF_INST(branch_on_count_register);
DEF_INST(branch_on_count);
DEF_INST(branch_on_index_high);
DEF_INST(branch_on_index_low_or_equal);
DEF_INST(branch_relative_on_condition);
DEF_INST(branch_relative_and_save);
DEF_INST(branch_relative_on_count);
DEF_INST(branch_relative_on_index_high);
DEF_INST(branch_relative_on_index_low_or_equal);
DEF_INST(checksum);
DEF_INST(compare_register);
DEF_INST(compare);
DEF_INST(compare_and_form_codeword);
DEF_INST(compare_and_swap);
DEF_INST(compare_double_and_swap);
DEF_INST(compare_halfword);
DEF_INST(compare_halfword_immediate);
DEF_INST(compare_logical_register);
DEF_INST(compare_logical);
DEF_INST(compare_logical_immediate);
DEF_INST(compare_logical_character);
DEF_INST(compare_logical_characters_under_mask);
DEF_INST(compare_logical_character_long);
DEF_INST(compare_logical_long_extended);
DEF_INST(compare_logical_string);
DEF_INST(compare_until_substring_equal);
DEF_INST(convert_unicode_to_utf8);
DEF_INST(convert_utf8_to_unicode);
DEF_INST(convert_to_binary);
DEF_INST(convert_to_decimal);
DEF_INST(copy_access);
DEF_INST(divide_register);
DEF_INST(divide);
DEF_INST(exclusive_or_register);
DEF_INST(exclusive_or);
DEF_INST(exclusive_or_immediate);
DEF_INST(exclusive_or_character);
DEF_INST(execute);
DEF_INST(extract_access_register);
DEF_INST(insert_character);
DEF_INST(insert_characters_under_mask);
DEF_INST(insert_program_mask);
DEF_INST(load);
DEF_INST(load_register);
DEF_INST(load_access_multiple);
DEF_INST(load_address);
DEF_INST(load_address_extended);
DEF_INST(load_and_test_register);
DEF_INST(load_complement_register);
DEF_INST(load_halfword);
DEF_INST(load_halfword_immediate);
DEF_INST(load_multiple);
DEF_INST(load_negative_register);
DEF_INST(load_positive_register);
DEF_INST(monitor_call);
DEF_INST(move_immediate);
DEF_INST(move_character);
DEF_INST(move_inverse);
DEF_INST(move_long);
DEF_INST(move_long_extended);
DEF_INST(move_numerics);
DEF_INST(move_string);
DEF_INST(move_with_offset);
DEF_INST(move_zones);
DEF_INST(multiply_register);
DEF_INST(multiply);
DEF_INST(multiply_halfword);
DEF_INST(multiply_halfword_immediate);
DEF_INST(multiply_single_register);
DEF_INST(multiply_single);
DEF_INST(or_register);
DEF_INST(or);
DEF_INST(or_immediate);
DEF_INST(or_character);
DEF_INST(perform_locked_operation);
DEF_INST(pack);
DEF_INST(search_string);
DEF_INST(set_access_register);
DEF_INST(set_program_mask);
DEF_INST(shift_left_double);
DEF_INST(shift_left_double_logical);
DEF_INST(shift_left_single);
DEF_INST(shift_left_single_logical);
DEF_INST(shift_right_double);
DEF_INST(shift_right_double_logical);
DEF_INST(shift_right_single);
DEF_INST(shift_right_single_logical);
DEF_INST(store);
DEF_INST(store_access_multiple);
DEF_INST(store_character);
DEF_INST(store_characters_under_mask);
DEF_INST(store_clock);
DEF_INST(store_clock_extended);
DEF_INST(store_halfword);
DEF_INST(store_multiple);
DEF_INST(subtract_register);
DEF_INST(subtract);
DEF_INST(subtract_halfword);
DEF_INST(subtract_logical_register);
DEF_INST(subtract_logical);
DEF_INST(supervisor_call);
DEF_INST(test_and_set);
DEF_INST(test_under_mask);
DEF_INST(test_under_mask_high);
DEF_INST(test_under_mask_low);
DEF_INST(translate);
DEF_INST(translate_and_test);
DEF_INST(translate_extended);
DEF_INST(unpack);
DEF_INST(update_tree);


/* Instructions in io.c */
DEF_INST(clear_subchannel);
DEF_INST(halt_subchannel);
DEF_INST(modify_subchannel);
DEF_INST(resume_subchannel);
DEF_INST(set_address_limit);
DEF_INST(set_channel_monitor);
DEF_INST(reset_channel_path);
DEF_INST(start_subchannel);
DEF_INST(store_channel_path_status);
DEF_INST(store_channel_report_word);
DEF_INST(store_subchannel);
DEF_INST(test_pending_interruption);
DEF_INST(test_subchannel);
DEF_INST(start_io);
DEF_INST(test_io);
DEF_INST(halt_io);
DEF_INST(test_channel);
DEF_INST(store_channel_id);
DEF_INST(connect_channel_set);
DEF_INST(disconnect_channel_set);


/* Instructions in service.c */
DEF_INST(service_call);
DEF_INST(channel_subsystem_call);


/* Instructions in xstore.c */
DEF_INST(page_in);
DEF_INST(page_out);
DEF_INST(move_page);
DEF_INST(invalidate_expanded_storage_block_entry);


/* Instructions in vector.c */
DEF_INST(v_test_vmr);
DEF_INST(v_complement_vmr);
DEF_INST(v_count_left_zeros_in_vmr);
DEF_INST(v_count_ones_in_vmr);
DEF_INST(v_extract_vct);
DEF_INST(v_extract_vector_modes);
DEF_INST(v_restore_vr);
DEF_INST(v_save_changed_vr);
DEF_INST(v_save_vr);
DEF_INST(v_load_vmr);
DEF_INST(v_load_vmr_complement);
DEF_INST(v_store_vmr);
DEF_INST(v_and_to_vmr);
DEF_INST(v_or_to_vmr);
DEF_INST(v_exclusive_or_to_vmr);
DEF_INST(v_save_vsr);
DEF_INST(v_save_vmr);
DEF_INST(v_restore_vsr);
DEF_INST(v_restore_vmr);
DEF_INST(v_load_vct_from_address);
DEF_INST(v_clear_vr);
DEF_INST(v_set_vector_mask_mode);
DEF_INST(v_load_vix_from_address);
DEF_INST(v_store_vector_parameters);
DEF_INST(v_save_vac);
DEF_INST(v_restore_vac);


/* Instructions in ses.c */
DEF_INST(ses_opcode_0105);
DEF_INST(ses_opcode_0106);
DEF_INST(ses_opcode_0108);
DEF_INST(ses_opcode_0109);
DEF_INST(ses_opcode_B260);
DEF_INST(ses_opcode_B261);
DEF_INST(ses_opcode_B264);
DEF_INST(ses_opcode_B265);
DEF_INST(ses_opcode_B266);
DEF_INST(ses_opcode_B267);
DEF_INST(ses_opcode_B268);
DEF_INST(ses_opcode_B272);
DEF_INST(ses_opcode_B27A);
DEF_INST(ses_opcode_B27B);
DEF_INST(ses_opcode_B27C);
DEF_INST(ses_opcode_B27E);
DEF_INST(ses_opcode_B27F);
DEF_INST(ses_opcode_B2A4);
DEF_INST(ses_opcode_B2A8);
DEF_INST(ses_opcode_B2F1);
DEF_INST(ses_opcode_B2F6);


/* Instructions in crypto.c */
DEF_INST(crypto_opcode_B269);
DEF_INST(crypto_opcode_B26A);
DEF_INST(crypto_opcode_B26B);
DEF_INST(crypto_opcode_B26C);
DEF_INST(crypto_opcode_B26D);
DEF_INST(crypto_opcode_B26E);
DEF_INST(crypto_opcode_B26F);


/* Instructions in esame.c */
DEF_INST(store_fpc);
DEF_INST(load_fpc);
DEF_INST(set_fpc);
DEF_INST(extract_fpc);
DEF_INST(set_rounding_mode);
DEF_INST(trap2);
DEF_INST(trap4);
DEF_INST(resume_program);
DEF_INST(trace_long);
DEF_INST(convert_to_binary_long);
DEF_INST(convert_to_decimal_long);
DEF_INST(multiply_logical);
DEF_INST(multiply_logical_long);
DEF_INST(multiply_logical_register);
DEF_INST(multiply_logical_long_register);
DEF_INST(divide_logical);
DEF_INST(divide_logical_long);
DEF_INST(divide_logical_register);
DEF_INST(divide_logical_long_register);
DEF_INST(add_logical_carry_long_register);
DEF_INST(subtract_logical_borrow_long_register);
DEF_INST(add_logical_carry_long);
DEF_INST(subtract_logical_borrow_long);
DEF_INST(add_logical_carry_register);
DEF_INST(subtract_logical_borrow_register);
DEF_INST(add_logical_carry);
DEF_INST(subtract_logical_borrow);
DEF_INST(divide_single_long);
DEF_INST(divide_single_long_fullword);
DEF_INST(divide_single_long_register);
DEF_INST(divide_single_long_fullword_register);
DEF_INST(load_logical_character);
DEF_INST(load_logical_halfword);
DEF_INST(store_pair_to_quadword);
DEF_INST(load_pair_from_quadword);
DEF_INST(extract_stacked_registers_long);
DEF_INST(extract_psw);
DEF_INST(extract_and_set_extended_authority);
DEF_INST(load_address_relative_long);
DEF_INST(store_facilities_list);
DEF_INST(load_long_halfword_immediate);
DEF_INST(add_long_halfword_immediate);
DEF_INST(multiply_long_halfword_immediate);
DEF_INST(compare_long_halfword_immediate);
DEF_INST(and_long);
DEF_INST(or_long);
DEF_INST(exclusive_or_long);
DEF_INST(and_long_register);
DEF_INST(or_long_register);
DEF_INST(exclusive_or_long_register);
DEF_INST(load_long_register);
DEF_INST(add_logical_long_register);
DEF_INST(add_logical_long_fullword_register);
DEF_INST(subtract_logical_long_register);
DEF_INST(subtract_logical_long_fullword_register);
DEF_INST(load_control_long);
DEF_INST(store_control_long);
DEF_INST(load_multiple_disjoint);
DEF_INST(load_multiple_high);
DEF_INST(load_multiple_long);
DEF_INST(store_multiple_high);
DEF_INST(store_multiple_long);
DEF_INST(load_using_real_address_long);
DEF_INST(store_using_real_address_long);
DEF_INST(test_addressing_mode);
DEF_INST(set_addressing_mode_24);
DEF_INST(set_addressing_mode_31);
DEF_INST(set_addressing_mode_64);
DEF_INST(load_program_status_word_extended);
DEF_INST(store_long);
DEF_INST(store_real_address);
DEF_INST(load_long);
DEF_INST(multiply_single_long_register);
DEF_INST(multiply_single_long_fullword_register);
DEF_INST(multiply_single_long);
DEF_INST(multiply_single_long_fullword);
DEF_INST(rotate_left_single_logical_long);
DEF_INST(rotate_left_single_logical);
DEF_INST(shift_right_single_long);
DEF_INST(shift_left_single_long);
DEF_INST(shift_right_single_logical_long);
DEF_INST(shift_left_single_logical_long);
DEF_INST(compare_logical_long);
DEF_INST(compare_logical_long_fullword);
DEF_INST(compare_logical_long_fullword_register);
DEF_INST(load_logical_long_thirtyone_register);
DEF_INST(compare_logical_long_register);
DEF_INST(test_under_mask_high_high);
DEF_INST(test_under_mask_high_low);
DEF_INST(branch_relative_on_count_long);
DEF_INST(load_positive_long_register);
DEF_INST(load_negative_long_register);
DEF_INST(load_and_test_long_register);
DEF_INST(load_complement_long_register);
DEF_INST(load_real_address_long);
DEF_INST(load_long_fullword_register);
DEF_INST(add_long_register);
DEF_INST(add_long_fullword_register);
DEF_INST(subtract_long_register);
DEF_INST(subtract_long_fullword_register);
DEF_INST(add_logical_long);
DEF_INST(add_logical_long_fullword);
DEF_INST(add_long);
DEF_INST(add_long_fullword);
DEF_INST(subtract_logical_long);
DEF_INST(subtract_logical_long_fullword);
DEF_INST(subtract_long);
DEF_INST(subtract_long_fullword);
DEF_INST(compare_long_register);
DEF_INST(compare_long);
DEF_INST(branch_on_count_long_register);
DEF_INST(branch_on_count_long);
DEF_INST(compare_and_swap_long);
DEF_INST(compare_double_and_swap_long);
DEF_INST(branch_on_index_high_long);
DEF_INST(branch_on_index_low_or_equal_long);
DEF_INST(branch_relative_on_index_high_long);
DEF_INST(branch_relative_on_index_low_or_equal_long);
DEF_INST(compare_logical_characters_under_mask_high);
DEF_INST(store_characters_under_mask_high);
DEF_INST(insert_characters_under_mask_high);
DEF_INST(branch_relative_on_condition_long);
DEF_INST(branch_relative_and_save_long);
DEF_INST(compare_long_fullword_register);
DEF_INST(load_positive_long_fullword_register);
DEF_INST(load_negative_long_fullword_register);
DEF_INST(load_and_test_long_fullword_register);
DEF_INST(load_complement_long_fullword_register);
DEF_INST(load_long_fullword);
DEF_INST(load_long_halfword);
DEF_INST(compare_long_fullword);
DEF_INST(load_logical_long_fullword_register);
DEF_INST(load_logical_long_fullword);
DEF_INST(load_logical_long_thirtyone);
DEF_INST(insert_immediate_high_high);
DEF_INST(insert_immediate_high_low);
DEF_INST(insert_immediate_low_high);
DEF_INST(insert_immediate_low_low);
DEF_INST(and_immediate_high_high);
DEF_INST(and_immediate_high_low);
DEF_INST(and_immediate_low_high);
DEF_INST(and_immediate_low_low);
DEF_INST(or_immediate_high_high);
DEF_INST(or_immediate_high_low);
DEF_INST(or_immediate_low_high);
DEF_INST(or_immediate_low_low);
DEF_INST(load_logical_immediate_high_high);
DEF_INST(load_logical_immediate_high_low);
DEF_INST(load_logical_immediate_low_high);
DEF_INST(load_logical_immediate_low_low);
DEF_INST(load_reversed_long_register);
DEF_INST(load_reversed_register);
DEF_INST(load_reversed_long);
DEF_INST(load_reversed);
DEF_INST(load_reversed_half);
DEF_INST(store_reversed_long);
DEF_INST(store_reversed);
DEF_INST(store_reversed_half);
DEF_INST(pack_ascii);
DEF_INST(pack_unicode);
DEF_INST(unpack_ascii);
DEF_INST(unpack_unicode);


/* Instructions in ieee.c */
DEF_INST(add_bfp_ext_reg);
DEF_INST(add_bfp_long_reg);
DEF_INST(add_bfp_long);
DEF_INST(add_bfp_short_reg);
DEF_INST(add_bfp_short);
DEF_INST(compare_bfp_ext_reg);
DEF_INST(compare_bfp_long_reg);
DEF_INST(compare_bfp_long);
DEF_INST(compare_bfp_short_reg);
DEF_INST(compare_bfp_short);
DEF_INST(compare_and_signal_bfp_ext_reg);
DEF_INST(compare_and_signal_bfp_long_reg);
DEF_INST(compare_and_signal_bfp_long);
DEF_INST(compare_and_signal_bfp_short_reg);
DEF_INST(compare_and_signal_bfp_short);
DEF_INST(convert_fix32_to_bfp_long_reg);
DEF_INST(convert_fix32_to_bfp_short_reg);
DEF_INST(convert_fix64_to_bfp_long_reg);
DEF_INST(convert_fix64_to_bfp_short_reg);
DEF_INST(convert_bfp_long_to_fix32_reg);
DEF_INST(convert_bfp_short_to_fix32_reg);
DEF_INST(convert_bfp_long_to_fix64_reg);
DEF_INST(convert_bfp_short_to_fix64_reg);
DEF_INST(divide_bfp_ext_reg);
DEF_INST(divide_bfp_long_reg);
DEF_INST(divide_bfp_long);
DEF_INST(divide_bfp_short_reg);
DEF_INST(divide_bfp_short);
DEF_INST(extract_floating_point_control_register);
DEF_INST(load_and_test_bfp_ext_reg);
DEF_INST(load_and_test_bfp_long_reg);
DEF_INST(load_and_test_bfp_short_reg);
DEF_INST(loadlength_bfp_short_to_long_reg);
DEF_INST(loadlength_bfp_short_to_long);
DEF_INST(load_negative_bfp_ext_reg);
DEF_INST(load_negative_bfp_long_reg);
DEF_INST(load_negative_bfp_short_reg);
DEF_INST(load_positive_bfp_ext_reg);
DEF_INST(load_positive_bfp_long_reg);
DEF_INST(load_positive_bfp_short_reg);
DEF_INST(round_bfp_long_to_short_reg);
DEF_INST(multiply_bfp_ext_reg);
DEF_INST(multiply_bfp_long_reg);
DEF_INST(multiply_bfp_long);
DEF_INST(multiply_bfp_short_reg);
DEF_INST(multiply_bfp_short);
DEF_INST(squareroot_bfp_ext_reg);
DEF_INST(squareroot_bfp_long_reg);
DEF_INST(squareroot_bfp_long);
DEF_INST(squareroot_bfp_short_reg);
DEF_INST(squareroot_bfp_short);
DEF_INST(subtract_bfp_ext_reg);
DEF_INST(subtract_bfp_long_reg);
DEF_INST(subtract_bfp_long);
DEF_INST(subtract_bfp_short_reg);
DEF_INST(subtract_bfp_short);

/* end of OPCODE.H */
