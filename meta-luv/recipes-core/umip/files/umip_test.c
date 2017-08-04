/*
 * tests for Intel User-Mode Execution Prevention
 *
 * GPLv2
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <err.h>
#include <signal.h>
#include <string.h>
#include "umip_test_defs.h"

#ifdef __x86_64__
#define GDTR_LEN 10
#define IDTR_LEN 10
#else
#define GDTR_LEN 6
#define IDTR_LEN 6
#endif

int test_passed, test_failed, test_errors;
sig_atomic_t got_signal;
sig_atomic_t got_sigcode;

static void handler(int signum, siginfo_t *info, void *ctx_void)
{
	ucontext_t *ctx = (ucontext_t *)ctx_void;

        pr_info("si_signo[%d]\n", info->si_signo);
        pr_info("si_errno[%d]\n", info->si_errno);
        pr_info("si_code[%d]\n", info->si_code);
        pr_info("si_addr[0x%p]\n", info->si_addr);
	if (signum != SIGSEGV)
		errx(1, "ERROR: Received unexpected signal");
	else
		got_signal = signum;
	if (info->si_code == SEGV_MAPERR)
		pr_info("Signal because of unmapped object.\n");
	else if (info->si_code == SI_KERNEL)
		pr_info("Signal because of #GP\n");
	else
		pr_info("Unknown si_code!\n");

	/*
	 * Move to the next instruction; to move, increment the instruction
	 * pointer by 10 bytes. 10 bytes is the size of the instruction
	 * considering two prefix bytes, two opcode bytes, one
	 * ModRM byte, one SIB byte and 4 displacement bytes. We have
	 * a NOP sled after the instruction to ensure we continue execution
	 * safely in case we overestimate the size of the instruction.
	 */
#ifdef __x86_64__
	ctx->uc_mcontext.gregs[REG_RIP] += 10;
#else
	ctx->uc_mcontext.gregs[REG_EIP] += 10;
#endif
}

static void call_sgdt()
{
	unsigned char val[GDTR_LEN];
	unsigned long base = INIT_VAL(89898989);
	unsigned short limit = 0x3d3d;
	int i, exp_signum, exp_sigcode;

	got_signal = 0;
	got_sigcode = 0;
	INIT_EXPECTED_SIGNAL(exp_signum, 0, exp_sigcode, 0);

	for (i = 0; i < GDTR_LEN; i++)
		val[i] = 0;
	pr_info("Will issue SGDT and save at [%p]\n", val);
	asm volatile("sgdt %0\n" NOP_SLED : "=m" (val));

	if(inspect_signal(exp_signum, exp_sigcode))
		return;

	limit = val[1] << 8 | val[0];
	pr_info("GDT Limit [0x%04x]\n", limit);

	if (limit == expected_gdt.limit)
		pr_pass(test_passed, "Expected limit value\n");
	else
		pr_fail(test_failed, "Unexpected limit value\n");

#if 0
	for (i = 0; i < (GDTR_LEN -2); i++)
		base |= (unsigned long) val[i+2] << ((i * 8));
#else
	base = *(unsigned long *)(val + 2);
#endif
	pr_info("GDT Base [0x%016lx]\n", base);

	if (base == expected_gdt.base)
		pr_pass(test_passed, "Expected base value\n");
	else
		pr_fail(test_failed, "Unexpected base value\n");
}

static void call_sidt()
{
	unsigned char val[IDTR_LEN];
	unsigned long base = INIT_VAL(73737373);
	unsigned short limit = 0x9696;
	int i, exp_signum, exp_sigcode;

	got_signal = 0;
	got_sigcode = 0;
	INIT_EXPECTED_SIGNAL(exp_signum, 0, exp_sigcode, 0);

	for (i = 0; i < IDTR_LEN; i++)
		val[i] = 0;
	pr_info("Will issue SIDT and save at [%p]\n", val);
	asm volatile("sidt %0\n"  NOP_SLED : "=m" (val));

	if(inspect_signal(exp_signum, exp_sigcode))
		return;

	limit = val[1] << 8 | val[0];
	pr_info("IDT Limit [0x%04x]\n", limit);

	if (limit == expected_idt.limit)
		pr_pass(test_passed, "Expected limit value\n");
	else
		pr_fail(test_failed, "Unexpected limit value\n");

#if 0
	for (i = 0; i < (IDTR_LEN -2); i++)
		base |= (unsigned long) val[i+2] << ((i * 8));
#else
	base = *(unsigned long *)(val + 2);
#endif
	pr_info("IDT Base [0x%016lx]\n", base);

	if (base == expected_idt.base)
		pr_pass(test_passed, "Expected base value\n");
	else
		pr_fail(test_failed, "Unexpected base value\n");
}

static void call_sldt()
{
	unsigned long val = INIT_VAL(a1a1a1a1);
	unsigned long init_val = INIT_VAL(a1a1a1a1);
	/* if operand is memory, result is 16-bit */
	unsigned short mask = 0xffff;
	int exp_signum, exp_sigcode;

	got_signal = 0;
	got_sigcode = 0;
	INIT_EXPECTED_SIGNAL_STR_SLDT(exp_signum, 0, exp_sigcode, 0);

	pr_info("Will issue SLDT and save at [%p]\n", &val);
	asm volatile("sldt %0\n" NOP_SLED : "=m" (val));

	if(inspect_signal(exp_signum, exp_sigcode))
		return;

	pr_info("SS for LDT[0x%08lx]\n", val);

	/*
	 * Check that the bits that are supposed to change does so
	 * as well as that the bits that are not supposed to change
	 * does not change.
	 */ \
	if ((val & mask) == expected_ldt &&
	    (val & ~mask) == (init_val & ~mask))
		pr_pass(test_passed, "Obtained expected value\n");
	else
		pr_fail(test_failed, "Obtained unexpected value\n");
}

static void call_smsw()
{
	unsigned long val = INIT_VAL(a2a2a2a2);
	unsigned long init_val = INIT_VAL(a2a2a2a2);
	unsigned short mask = 0xffff;
	int exp_signum, exp_sigcode;

	got_signal = 0;
	got_sigcode = 0;
	INIT_EXPECTED_SIGNAL(exp_signum, 0, exp_sigcode, 0);


	pr_info("Will issue SMSW and save at [%p]\n", &val);
	asm volatile("smsw %0\n" NOP_SLED : "=m" (val));

	if(inspect_signal(exp_signum, exp_sigcode))
		return;

	pr_info("CR0[0x%08lx]\n", val);

	/*
	 * Check that the bits that are supposed to change does so
	 * as well as that the bits that are not supposed to change
	 * does not change.
	 */ \
	if ((val & mask) == expected_msw &&
	    (val & ~mask) == (init_val & ~mask))
		pr_pass(test_passed, "Obtained expected value\n");
	else
		pr_fail(test_failed, "Obtained unexpected value\n");
}

static void call_str()
{
	unsigned int val32 = 0xa4a4a4a4;
	unsigned int init_val32 = 0xa4a4a4a4;
	unsigned short val16 = 0xa5a5;
	unsigned short init_val16 = 0xa5a5;
	int exp_signum, exp_sigcode;

	got_signal = 0;
	got_sigcode = 0;
	INIT_EXPECTED_SIGNAL_STR_SLDT(exp_signum, 0, exp_sigcode, 0);

#if __x86_64__
	unsigned long val64 = 0xa3a3a3a3a3a3a3a3;

	pr_info("Will issue STR and save at m64[0x%p]\n", &val64);
	asm volatile("str %0\n" NOP_SLED : "=m" (val64));

	if(inspect_signal(exp_signum, exp_sigcode))
		goto test_m32;

	pr_info("SS for TSS[0x%016lx]\n", val64);

	/* All 64 bits are written */
	if (val64 == expected_tr)
		pr_pass(test_passed, "Obtained 64-bit expected value\n");
	else
		pr_fail(test_failed, "Obtained 64-bit unexpected value\n");

test_m32:
#endif

	got_signal = 0;
	got_sigcode = 0;

	pr_info("Will issue STR and save at m32[0x%p]\n", &val32);
	asm volatile("str %0\n" NOP_SLED : "=m" (val32));

	if(inspect_signal(exp_signum, exp_sigcode))
		goto test_m16;

	pr_info("SS for TSS[0x%08x]\n", val32);

	/*
	 * Check that the bits that are supposed to change does so
	 * as well as that the bits that are not supposed to change
	 * does not change. Since operand is memory, value will be
	 * 16-bit.
	 */
	if ((val32 & 0xffff) == expected_tr &&
	    (val32 & ~0xffff) == (init_val32 & ~0xffff))
		pr_pass(test_passed, "Obtained 32-bit expected value\n");
	else
		pr_fail(test_failed, "Obtained 32-bit unexpected value\n");

test_m16:
	got_signal = 0;
	got_sigcode = 0;

	pr_info("Will issue STR and save at m16[0x%p]\n", &val16);
	asm volatile("str %0\n" NOP_SLED : "=m" (val16));

	if(inspect_signal(exp_signum, exp_sigcode))
		return;

	pr_info("SS for TSS[0x%04x]\n", val16);

	/*
	 * Check that the bits that are supposed to change does so
	 * as well as that the bits that are not supposed to change
	 * does not change. Since operand is memory, value will be
	 * 16-bit.
	 */
	if ((val16 & 0xffff) == expected_tr &&
	    (val16 & ~0xffff) == (init_val16 & ~0xffff))
		pr_pass(test_passed, "Obtained 16-bit expected value\n");
	else
		pr_fail(test_failed, "Obtained 16-bit unexpected value\n");

}

int main(void)
{
	struct sigaction action;

	PRINT_BITNESS;

	memset(&action, 0, sizeof(action));
	action.sa_sigaction = &handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error(test_errors, "Could not set the signal handler!");
		print_results();
		exit(1);
	}

	call_sgdt();
	call_sidt();
	call_sldt();
	call_smsw();
	call_str();

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error(test_errors, "Could not remove signal handler!");
		print_results();
		exit(1);
	}

	print_results();
	return 0;
}
