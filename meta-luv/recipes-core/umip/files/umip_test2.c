#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

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
			printf("Success on %s-bit '%s %s'! Got [0x%lx] Exp[0x%lx]\n", \
			        #op_size, insn, reg, val, exp); \
		else { \
			printf("Error on %s-bit '%s %s'! Got[0x%lx] Exp[0x%lx]\n", \
			       #op_size, insn, reg, val, exp); \
			return -1; \
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
		printf("Success on '%s %s(%s)'! Got [0x%lx] Exp[0x%lx]\n", \
		       insn, #INSNmacro, reg, val, exp); \
	else { \
		printf("Error on '%s %s(%s)'! Got[0x%lx] Exp[0x%lx]\n", \
		       insn, #INSNmacro, reg, val, exp); \
		return -1; \
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

const unsigned long correct_ss = 0x40;
#if __x86_64__
const unsigned long correct_msw = 0x80050033;
#else
const unsigned long correct_msw = 0x50033;
#endif
const unsigned long correct_ldtss = 0x0;

#if __x86_64__
#define INIT_SS   0x1313131313131313
#define INIT_MSW  0x1414141414141414
#define INIT_LDTS 0x1515151515151515
#else
#define INIT_SS   0x13131313
#define INIT_MSW  0x14141414
#define INIT_LDTS 0x15151515
#endif

void sig_handler(int signum)
{
        printf("Whoa! I got a SIGSEGV. UMIP should not cause it!\n");
        exit(1);
}

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
		printf("Invalid operand size!\n");
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
	unsigned short mask = 0xffff;

	signal(SIGSEGV, sig_handler);

	printf("====Checking STR. Expected value: [0x%lx]====\n", correct_ss);
	printf("==Tests for register operands==\n");
	printf("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLreg("str", val, INIT_SS, correct_ss);
	printf("==Tests for memory operands==\n");
	printf("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLmem("str", val, INIT_SS, correct_ss);
	/* TODO: check with addressing using SIB byte */
	return 0;

}

static int test_smsw(void)
{
	unsigned long val;
	unsigned short mask = 0xffff;

	printf("====Checking SMSW. Expected value: [0x%lx]====\n", correct_msw);
	printf("==Tests for register operands==\n");
	printf("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLreg("smsw", val, INIT_MSW, correct_msw);
	printf("==Tests for memory operands==\n");
	printf("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLmem("smsw", val, INIT_MSW, correct_msw);
	/* TODO: check with addressing using SIB byte */
	return 0;
}

static int test_sldt(void)
{
	unsigned long val;
	unsigned short mask = 0xffff;

	printf("====Checking SLDT. Expected value: [0x%lx]====\n", correct_ldtss);
	printf("==Tests for register operands==\n");
	printf("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLreg("sldt", val, INIT_LDTS, correct_ldtss);
	printf("==Tests for memory operands==\n");
	printf("Value should be saved at [0x%p]\n", &val);
	CHECK_ALLmem("sldt", val, INIT_LDTS, correct_ldtss);
	/* TODO: check with addressing using SIB byte */
	return 0;
}

void main (void)
{
	int ret_str, ret_smsw, ret_sldt;

	printf("***Starting tests***\n");
	ret_str = test_str();
	ret_smsw = test_smsw();
	ret_sldt = test_sldt();
	if (ret_str || ret_smsw || ret_sldt)
		printf("***Test completed with errors str[%d] smsw[%d] sldt[%d]\n",
		       ret_str, ret_smsw, ret_sldt);
	else
		printf("***All tests completed successfully.***\n");
}
