#ifndef _UMIP_TEST_DEFS_H
#define _UMIP_TEST_DEFS_H
#include <stdio.h>

#define TEST_PASS "\x1b[32m[pass]\x1b[0m "
#define TEST_FAIL "\x1b[31m[FAIL]\x1b[0m "
#define TEST_INFO "\x1b[34m[info]\x1b[0m "
#define TEST_ERROR "\x1b[33m[ERROR]\x1b[0m "

#define pr_pass(pass_ctr, ...) do{ printf(TEST_PASS __VA_ARGS__); pass_ctr++; } while(0)
#define pr_fail(fail_ctr, ...) do{ printf(TEST_FAIL __VA_ARGS__); fail_ctr++; } while(0)
#define pr_info(...) printf(TEST_INFO __VA_ARGS__)
#define pr_error(error_ctr, ...) do{ printf(TEST_ERROR __VA_ARGS__); error_ctr++; } while(0)

/*
 * Use this definiton to check for results that fit in a single variable
 * (e.g., char, short, int, long, double)
 */
#define pr_result(result, expected, text, pass_ctr, fail_ctr) \
	do{								\
		if (result == expected)					\
			pr_pass(pass_ctr, text);			\
		else							\
			pr_fail(fail_ctr, text);			\
		printf("Got:[0x%x]Exp[0x%x]\n", result, expected);	\
	} while(0)

/*
 * Use this definiton to check for results that in struct table_desc
 * (e.g., char, short, int, long, double)
 */
#define pr_result_table(result, expected, text, pass_ctr, fail_ctr) \
	do{								\
		if ((result->base == expected->base) &&			\
		    (result->limit == expected->limit))			\
			pr_pass(pass_ctr, text);			\
		else							\
			pr_fail(fail_ctr, text);			\
		printf("Got:Base[0x%lx]Limit[0x%x]ExpBase[0x%lx]Limit[0x%x]\n", \
			got->base, got->limit,				\
			expected->base, expected->limit);		\
	} while(0)

#ifdef __x86_64__
#define PRINT_BITNESS pr_info("This binary uses 64-bit code\n")
#define INIT_VAL(val) (0x##val##val)
#else
#define PRINT_BITNESS pr_info("This binary uses 32-bit code\n")
#define INIT_VAL(val) (0x##val)
#endif

#define EXPECTED_SMSW 0x33
#define EXPECTED_SLDT 0x0
#define EXPECTED_STR 0x0
#define EXPECTED_GDT_BASE 0xfffe0000
#define EXPECTED_GDT_LIMIT 0x0
#define EXPECTED_IDT_BASE 0xffff0000
#define EXPECTED_IDT_LIMIT 0x0

/*
 * EMULATE_ALL implies that all the UMIP-protected instructions are emulated.
 * If not defined, the following rules apply:
 *  + UMIP-protected instructions are not emulated for 64-bit processes. This
 *    means that we always should get a SIGSEGV signal with code SI_KERNEL.
 *  + In 32-bit processes, only SGDT, SIDT and SMSW are emulated, STR and
 *    SLDT should cause a SIGSEGV signal with code SI_CODE.
 */
#ifdef EMULATE_ALL
#define INIT_EXPECTED_SIGNAL(signum, exp_signum, sigcode, exp_sigcode)	\
	do{								\
		signum = exp_signum;					\
		sigcode = exp_sigcode;					\
	} while (0)
#else /* EMULATE_ALL */
#ifdef __x86_64__
#define INIT_EXPECTED_SIGNAL(signum, exp_signum, sigcode, exp_sigcode)	\
	do{								\
		signum = SIGSEGV;					\
		sigcode = SI_KERNEL;					\
	} while(0)
#else /* __x86_64__ */
#define INIT_EXPECTED_SIGNAL(signum, exp_signum, sigcode, exp_sigcode)	\
	do{								\
		signum = exp_signum;					\
		sigcode = exp_sigcode;					\
	} while(0)
#endif /* __x86_64__ */
#endif /* EMULATE_ALL */

#ifdef EMULATE_ALL
#define INIT_EXPECTED_SIGNAL_STR_SLDT(signum, exp_signum, sigcode, exp_sigcode)	\
	INIT_EXPECTED_SIGNAL(signum, exp_signum, sigcode, exp_sigcode)
#else
#define INIT_EXPECTED_SIGNAL_STR_SLDT(signum, exp_signum, sigcode, exp_sigcode)	\
	INIT_EXPECTED_SIGNAL(signum, SIGSEGV, sigcode, SI_KERNEL)
#endif

struct table_desc {
	unsigned short limit;
	unsigned long base;
} __attribute__((packed));

static const unsigned short expected_msw = EXPECTED_SMSW;
static const unsigned short expected_ldt = EXPECTED_SLDT;
static const unsigned short expected_tr = EXPECTED_STR;

static const struct table_desc expected_gdt = {
	.limit = EXPECTED_GDT_LIMIT,
	.base = EXPECTED_GDT_BASE
};

static const struct table_desc expected_idt = {
	.limit = EXPECTED_IDT_LIMIT,
	.base = EXPECTED_IDT_BASE
};

void print_results(void);

#endif /* _UMIP_TEST_DEFS_H */
