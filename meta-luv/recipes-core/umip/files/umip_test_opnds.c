#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include "umip_test_defs.h"

/* Register operands */

#if __x86_64__
#define PUSH(reg) "push %%r"reg"\n"
#define POP(reg) "pop %%r"reg"\n"
#else
#define PUSH(reg) "push %%e"reg"\n"
#define POP(reg) "pop %%e"reg"\n"
#endif

#define INSNreg16(insn, reg, aux) \
	PUSH(aux) \
	PUSH(reg) \
	insn" %%"reg"\n"     \
	NOP_SLED \
	"mov %%"reg", %%"aux"\n" \
	POP(reg) \
	"mov %%"aux", %0\n" \
	POP(aux)

#define INSNreg32(insn, reg, aux) \
	PUSH(aux) \
	PUSH(reg) \
	insn" %%e"reg"\n"     \
	NOP_SLED \
	"mov %%e"reg", %%e"aux"\n" \
	POP(reg) \
	"movl %%e"aux", %0\n" \
	POP(aux)

#define INSNreg64(insn, reg, aux) \
	"push %%"aux"\n" \
	"push %%"reg"\n" \
	insn" %%"reg"\n"     \
	NOP_SLED \
	"mov %%"reg", %%"aux"\n" \
	"pop %%"reg"\n" \
	"mov %%"aux", %0\n" \
	"pop %%"aux"\n"

#define CHECK_INSN(op_size, insn, reg, val, aux, init, exp) \
	do { \
		val = init; \
		mask = get_mask(op_size); \
		if (!mask) \
			return -1; \
		\
		got_signal = 0; \
		got_sigcode = 0; \
		\
		asm volatile(INSNreg##op_size(insn, reg, aux) : "=m" (val)); \
		\
		if(inspect_signal(exp_signum, exp_sigcode)) \
			break; \
		/* \
		 * Check that the bits that are supposed to change does so \
		 * as well as that the bits that are not supposed to change \
		 * does not change. \
		 */ \
		if (((val & mask) == (exp & mask)) && ((exp & ~mask) == (exp & ~mask))) \
			pr_pass(test_passed, " On %s-bit '%s %s'! Got [0x%016lx] Exp[0x%016lx]\n", \
			        #op_size, insn, reg, val, (val&mask) | (init&~mask)); \
		else { \
			pr_fail(test_failed, "On %s-bit '%s %s'! Got[0x%016lx] Exp[0x%016lx]\n", \
			       #op_size, insn, reg, val, (val&mask) | (init&~mask)); \
		} \
	} while(0);

#define CHECK_ALLreg32(insn, val, init, exp) \
	CHECK_INSN(16, insn,  "ax", val, "bx", init, exp); \
	CHECK_INSN(16, insn,  "cx", val, "ax", init, exp); \
	CHECK_INSN(16, insn,  "dx", val, "ax", init, exp); \
	CHECK_INSN(16, insn,  "bx", val, "ax", init, exp); \
	CHECK_INSN(16, insn, "bp", val, "ax", init, exp); \
	CHECK_INSN(16, insn,  "si", val, "ax", init, exp); \
	CHECK_INSN(16, insn,  "di", val, "ax", init, exp); \
	CHECK_INSN(32, insn,  "ax", val, "bx", init, exp); \
	CHECK_INSN(32, insn,  "cx", val, "ax", init, exp); \
	CHECK_INSN(32, insn,  "dx", val, "ax", init ,exp); \
	CHECK_INSN(32, insn,  "bx", val, "ax", init ,exp); \
	CHECK_INSN(32, insn,  "bp", val, "ax", init, exp);\
	CHECK_INSN(32, insn,  "si", val, "ax", init, exp); \
	CHECK_INSN(32, insn,  "di", val, "ax", init, exp); \

#define CHECK_ALLreg64(insn, val, init, exp) \
	CHECK_INSN(64, insn, "rax", val, "rbx", init, exp); \
	CHECK_INSN(64, insn, "rcx", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "rdx", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "rbx", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "rbp", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "rsi", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "rdi", val, "rax", init, exp); \
	CHECK_INSN(64, insn,  "r8", val, "rax", init, exp); \
	CHECK_INSN(64, insn,  "r9", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "r10", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "r11", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "r12", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "r13", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "r14", val, "rax", init, exp); \
	CHECK_INSN(64, insn, "r15", val, "rax", init, exp);

#if __x86_64__
#define CHECK_ALLreg(insn, val, init, exp) \
	CHECK_ALLreg32(insn, val, init, exp) \
	CHECK_ALLreg64(insn, val, init, exp)
#else
#define CHECK_ALLreg(insn, val, init, exp) \
	CHECK_ALLreg32(insn, val, init, exp)
#endif


/* Memory operands */
#if __x86_64__
#define INSNmem(insn, reg, disp) \
	/*
	 * Move initialization value to %eax. Do it before changing %esp as some\
	 * compilers refer local variables as an offset from %esp. This code	\
	 * causes %eax to be clobbered. This is OK as long it is not used in	\
	 * the caller function.							\
	 */									\
	"mov %0, %%rax\n"							\
	/*									\
	 * Make a backup of our scratch memory. Adjust wrt %rsp according to the\
	 * two stack pushes below.						\
	 */									\
	"push ("disp"-0x8-0x8)(%%rsp)\n"					\
	/* Make a backup of contents of test register */			\
	"push %%"reg"\n"							\
	/* Write our initialization value in scratch memory. */			\
	"mov %%rax, "disp"(%%rsp)\n"						\
	/* Make test register point to our scratch memory. */			\
	"mov %%rsp, %%"reg"\n"							\
	/* Run our test: register-indirect addressing with an offset. */	\
	insn" "disp"(%%"reg")\n"						\
	NOP_SLED								\
	/* Save result to %rax */						\
	"mov "disp"(%%rsp), %%rax\n"						\
	/* Restore test register */						\
	"pop %%"reg"\n"								\
	/*									\
	 * Restore scratch memory. Adjust offset wrt %rsp according to the	\
	 * two stack pops above							\
	 */									\
	"pop ("disp"-0x8-0x8)(%%rsp)\n"						\
	/*									\
	 * Since some compilers refer local variables as an offset from %esp,	\
	 * the test result can only be saved to a local variable only once %esp	\
	 * has been restored by an equal number of stack pops and pushes.	\
	 */									\
	"mov %%rax, %0\n"
#else
#define INSNmem(insn, reg, disp) \
	/*
	 * Move initialization value to %eax. Do it before changing %esp as some\
	 * compilers refer local variables as an offset from %esp. This code	\
	 * causes %eax to be clobbered. This is OK as long it is not used in	\
	 * the caller function.							\
	 */									\
	"mov %0, %%eax\n"							\
	/*									\
	 * Make a backup of our scratch memory. Adjust wrt %rsp according to the\
	 * two stack pushes below.						\
	 */									\
	"push ("disp"-0x4-0x4)(%%esp)\n"					\
	/* Make a backup of contents of test register */			\
	"push %%"reg"\n"							\
	/*									\
	 * Write our initialization value in scratch memory. Adjust offset	\
	 * according to	the number of previous stack pushes.			\
	 */									\
	"mov %%eax, "disp"(%%esp)\n"						\
	/* Make test register point to our scratch memory. */			\
	"mov %%esp, %%"reg"\n"							\
	/* Run our test: register-indirect addressing with an offset. */	\
	insn" "disp"(%%"reg")\n"						\
	NOP_SLED								\
	/* Save result to %eax */						\
	"mov "disp"(%%esp), %%eax\n"						\
	/* Restore test register */						\
	"pop %%"reg"\n"								\
	/*									\
	 * Restore scratch memory. Adjust offset wrt %rsp according to the	\
	 * two stack pops above							\
	 */									\
	"pop ("disp"-0x4-0x4)(%%esp)\n"						\
	/*									\
	 * Since some compilers refer local variables as an offset from %esp,	\
	 * the test result can only be saved to a local variable only once %esp	\
	 * has been restored by an equal number of stack pops and pushes.	\
	 */									\
	"mov %%eax, %0\n"
#endif

/*
 * TODO: test no displacement. However, gcc refuses to not use displacement.
 * Instead, it uses -0x0(%%reg). No displacement is OK unless the SIB byte
 * is used with RBP.
 */
#define INSNmemdisp8(insn, reg) INSNmem(insn, reg, "-0x80")
#define INSNmemdisp32(insn, reg) INSNmem(insn, reg, "-0x1000")

#define CHECK_INSNmemdisp(INSNmacro, insn, reg, val, init, exp) \
	val = init; \
	/* Memory operands are always treated as 16-bit locations */ \
	mask = get_mask(16); \
	if (!mask) \
		return -1; \
		\
		got_signal = 0; \
		got_sigcode = 0; \
	\
	asm volatile(INSNmacro(insn, reg) : "=m" (val): "m"(val) : "%rax"); \
	\
	if(!inspect_signal(exp_signum, exp_sigcode)) { \
		/* \
		* Check that the bits that are supposed to change does so \
		* as well as that the bits that are not supposed to change \
		* does not change. \
		*/ \
		if (((val & mask) == (exp & mask)) && ((exp & ~mask) == (exp & ~mask))) \
			pr_pass(test_passed, "On '%s %s(%s)'! Got [0x%016lx] Exp[0x%016lx]\n", \
			insn, #INSNmacro, reg, val, (exp & mask) | (init & ~mask)); \
		else { \
			pr_fail(test_failed, "On '%s %s(%s)'! Got[0x%016lx] Exp[0x%016lx]\n", \
				insn, #INSNmacro, reg, val, (exp & mask) | (init & ~mask)); \
		}\
	}

#define CHECK_INSNmem(insn, reg, val, init, exp) \
	CHECK_INSNmemdisp(INSNmemdisp8, insn, reg, val, init, exp) \
	CHECK_INSNmemdisp(INSNmemdisp32, insn, reg, val, init, exp)

#if __x86_64__
#define CHECK_ALLmem(insn, val, init, exp) \
	CHECK_INSNmem(insn, "rax", val, init, exp) \
	CHECK_INSNmem(insn, "rcx", val, init, exp) \
	CHECK_INSNmem(insn, "rdx", val, init, exp) \
	CHECK_INSNmem(insn, "rbp", val, init, exp) \
	CHECK_INSNmem(insn, "rsi", val, init, exp) \
	CHECK_INSNmem(insn, "rdi", val, init, exp) \
	CHECK_INSNmem(insn, "r8", val, init, exp) \
	CHECK_INSNmem(insn, "r9", val, init, exp) \
	CHECK_INSNmem(insn, "r10", val, init, exp) \
	CHECK_INSNmem(insn, "r11", val, init, exp) \
	CHECK_INSNmem(insn, "r12", val, init, exp) \
	CHECK_INSNmem(insn, "r13", val, init, exp) \
	CHECK_INSNmem(insn, "r14", val, init, exp) \
	CHECK_INSNmem(insn, "r15", val, init, exp)
#else
#define CHECK_ALLmem(insn, val, init, exp) \
	CHECK_INSNmem(insn, "eax", val, init, exp) \
	CHECK_INSNmem(insn, "ecx", val, init, exp) \
	CHECK_INSNmem(insn, "edx", val, init, exp) \
	CHECK_INSNmem(insn, "ebp", val, init, exp) \
	CHECK_INSNmem(insn, "esi", val, init, exp) \
	CHECK_INSNmem(insn, "edi", val, init, exp)
#endif

#define INIT_SS   INIT_VAL(13131313)
#define INIT_MSW  INIT_VAL(14141414)
#define INIT_LDTS INIT_VAL(15151515)

extern sig_atomic_t got_signal, got_sigcode;
int test_passed, test_failed, test_errors;

static unsigned long get_mask(int op_size) {
	switch (op_size) {
	case 16:
		return 0xffff;
	case 32:
		return 0xffffffff;
#if __x86_64__
	case 64:
		return 0xffffffffffffffff;
#endif
	default:
		pr_error(test_errors, "Invalid operand size!\n");
		/*
		 * We can't return -1 as it would be equal to the
		 * 32 or 64-bit mask
		 */
		return 0;
	}
}

static int test_str(void)
{
	unsigned long val;
	unsigned long mask = 0xffff;
	int exp_signum, exp_sigcode;

	INIT_EXPECTED_SIGNAL_STR_SLDT(exp_signum, 0, exp_sigcode, 0);

	pr_info("====Checking STR. Expected value: [0x%x]====\n", expected_tr);
	pr_info("==Tests for register operands==\n");
	pr_info("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLreg("str", val, INIT_SS, (unsigned long)expected_tr);
	pr_info("==Tests for memory operands==\n");
	pr_info("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLmem("str", val, INIT_SS, (unsigned long)expected_tr);
	/* TODO: check with addressing using SIB byte */
	return 0;

}

static int test_smsw(void)
{
	unsigned long val;
	unsigned long mask = 0xffff;
	int exp_signum, exp_sigcode;

	INIT_EXPECTED_SIGNAL(exp_signum, 0, exp_sigcode, 0);

	pr_info("====Checking SMSW. Expected value: [0x%x]====\n", expected_msw);
	pr_info("==Tests for register operands==\n");
	pr_info("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLreg("smsw", val, INIT_MSW, (unsigned long)expected_msw);
	pr_info("==Tests for memory operands==\n");
	pr_info("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLmem("smsw", val, INIT_MSW, (unsigned long)expected_msw);
	/* TODO: check with addressing using SIB byte */
	return 0;
}

static int test_sldt(void)
{
	unsigned long val;
	unsigned long mask = 0xffff;
	int exp_signum, exp_sigcode;

	INIT_EXPECTED_SIGNAL(exp_signum, 0, exp_sigcode, 0);

	pr_info("====Checking SLDT. Expected value: [0x%x]====\n", expected_ldt);
	pr_info("==Tests for register operands==\n");
	pr_info("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLreg("sldt", val, INIT_LDTS, (unsigned long)expected_ldt);
	pr_info("==Tests for memory operands==\n");
	pr_info("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLmem("sldt", val, INIT_LDTS, (unsigned long)expected_ldt);
	/* TODO: check with addressing using SIB byte */
	return 0;
}

int main (void)
{
	int ret_str, ret_smsw, ret_sldt;
	struct sigaction action;

	PRINT_BITNESS;

	memset(&action, 0, sizeof(action));
	action.sa_sigaction = &signal_handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error(test_errors, "Could not set the signal handler!");
		exit(1);
	}

	pr_info("***Starting tests***\n");
	ret_str = test_str();
	ret_smsw = test_smsw();
	ret_sldt = test_sldt();

	if (ret_str || ret_smsw || ret_sldt)
		pr_info("***Test completed with errors str[%d] smsw[%d] sldt[%d]\n",
		       ret_str, ret_smsw, ret_sldt);
	else
		pr_info("***All tests completed successfully.***\n");

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error(test_errors, "Could not remove signal handler!");
		return 1;
	}

	print_results();
	return 0;
}
