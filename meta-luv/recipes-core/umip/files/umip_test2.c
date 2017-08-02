#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ucontext.h>
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
	"mov %%"reg", %%"aux"\n" \
	POP(reg) \
	"mov %%"aux", %0\n" \
	POP(aux)

#define INSNreg32(insn, reg, aux) \
	PUSH(aux) \
	PUSH(reg) \
	insn" %%e"reg"\n"     \
	"mov %%e"reg", %%e"aux"\n" \
	POP(reg) \
	"movl %%e"aux", %0\n" \
	POP(aux)

#define INSNreg64(insn, reg, aux) \
	"push %%"aux"\n" \
	"push %%"reg"\n" \
	insn" %%"reg"\n"     \
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
		asm volatile(INSNreg##op_size(insn, reg, aux) : "=m" (val)); \
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
	"push %%rax\n"               /* rax used to copy values around. Make a copy */ \
	"mov %0, %%rax\n"            /* Init our variable. Split into two instructions */ \
	"mov %%rax, "disp"(%%rsp)\n" \
	"mov %%"reg", %%rax\n"       /* make a backup of register under test */ \
	"mov %%rsp, %%"reg"\n"       /* move rsp to our register under test */ \
	insn" "disp"(%%"reg")\n"     /* execute our instruction */ \
	"push "disp"(%%"reg")\n"     /* save result to stack */ \
	"mov %%rax, %%"reg"\n"       /* restore register under test */ \
	"pop %1\n"                   /* copy result to our variable */ \
	"pop %%rax\n"                /* restore rax */
#else
#define INSNmem(insn, reg, disp) \
	"push %%eax\n"               /* eax used to copy values around. Make a copy */ \
	"mov %0, %%eax\n"            /* Init our variable. Split into two instructions */ \
	"mov %%eax, "disp"(%%esp)\n" \
	"mov %%"reg", %%eax\n"       /* make a backup of register under test */ \
	"mov %%esp, %%"reg"\n"       /* move esp to our register under test */ \
	insn" "disp"(%%"reg")\n"     /* execute our instruction */ \
	"push "disp"(%%"reg")\n"     /* save result to stack */ \
	"mov %%eax, %%"reg"\n"       /* restore register under test */ \
	"pop %1\n"                   /* copy result to our variable */ \
	"pop %%eax\n"                /* restore eax */
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
	asm volatile(INSNmacro(insn, reg) : "=m" (val): "m"(val) : "%rax"); \
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

static sig_atomic_t got_signal;

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
		pr_error("Invalid operand size!\n");
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

static void handler(int signum, siginfo_t *info, void *ctx_void)
{
        pr_info("si_signo[%d]\n", info->si_signo);
        pr_info("si_errno[%d]\n", info->si_errno);
        pr_info("si_code[%d]\n", info->si_code);
        pr_info("si_code[0x%p]\n", info->si_addr);
	if (signum != SIGSEGV)
		pr_error("Received unexpected signal");
	else
		got_signal = signum;
	if (info->si_code == SEGV_MAPERR)
		pr_info("Signal because of unmapped object.\n");
	else if (info->si_code == SI_KERNEL)
		pr_info("Signal because of #GP\n");
	else
		pr_info("Unknown si_code!\n");

#ifdef __x86_64__
	pr_pass("I got a SIGSEGV. UMIP not emulated in 64-bit\n");
#else
	pr_fail("FAIL: Whoa! I got a SIGSEGV. This is an error!\n");
#endif
	exit(1);
}

int main (void)
{
	int ret_str, ret_smsw, ret_sldt;
	struct sigaction action;

	PRINT_BITNESS;

	memset(&action, 0, sizeof(action));
	action.sa_sigaction = &handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error("Could not set the signal handler!");
		exit(1);
	}

	pr_info("***Starting tests***\n");
	ret_str = test_str();
	ret_smsw = test_smsw();
	ret_sldt = test_sldt();
	if (ret_str || ret_smsw || ret_sldt)
		pr_fail("***Test completed with errors str[%d] smsw[%d] sldt[%d]\n",
		       ret_str, ret_smsw, ret_sldt);
	else
		pr_pass("***All tests completed successfully.***\n");

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error("Could not remove signal handler!");
		return 1;
	}

	return 0;
}
