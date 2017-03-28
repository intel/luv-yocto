/*
 * normal_pf.c - tests UMIP emulation code when a page fault should be
 * generated (i.e., the requested memory access is not mapped. No code
 * is included to test cases in which Memory Protection Keys are used.
 * Copyright (c) 2016 Intel Corporation
 *
 * GPL v2.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ucontext.h>
#include <err.h>
#include "umip_test_defs.h"

static sig_atomic_t signal_code;
static sig_atomic_t got_signal;

void handler(int signum, siginfo_t *info, void *ctx_void)
{
	ucontext_t *ctx = (ucontext_t *)ctx_void;

        pr_info("si_signo[%d]\n", info->si_signo);
        pr_info("si_errno[%d]\n", info->si_errno);
        pr_info("si_code[%d]\n", info->si_code);
        pr_info("si_code[0x%p]\n", info->si_addr);

	got_signal = signum;

	if (signum == SIGSEGV) {
		if (info->si_code == SEGV_MAPERR)
			pr_info("Signal because of unmapped object.\n");
		else if (info->si_code == SI_KERNEL)
			pr_info("Signal because of #GP\n");
		else
			pr_info("Unknown si_code!\n");
	}
	else if (signum == SIGILL) {
		if (info->si_code == SEGV_MAPERR)
			pr_info("Signal because of unmapped object.\n");
		else if (info->si_code == ILL_ILLOPN)
			pr_info("Signal because of #UD\n");
		else
			pr_info("Unknown si_code!\n");
	}
	else
		errx(1, "ERROR: Received unexpected signal");

	/* Save the signal code */
	signal_code = info->si_code;
	/*
	 * Move to the next instruction. We have a cushion of
	 * several NOPs. Thus, we can safely move 8 positions
	 */
#ifdef __x86_64__
	ctx->uc_mcontext.gregs[REG_RIP] += 8;
#else
	ctx->uc_mcontext.gregs[REG_EIP] += 4;
#endif
}

struct my_struct {
	int a;
	int b;
	int c;
};

int test_normal_pf(void)
{
	unsigned long *val_bad = (unsigned long *)0x100000;

	asm volatile ("smsw %0\n"
		      "nop\n"
		      "nop\n"
		      "nop\n"
		      "nop\n"
		      "nop\n"
		      "nop\n"
		      "nop\n"
		      "nop\n"
		      "nop\n": "=m"(*val_bad));

	if (!got_signal) {
		pr_fail("Signal not received!\n");
		return 1;
	}
#ifdef __x86_64__
	if (signal_code != SI_KERNEL) {
		pr_fail("Signal code is not what we expect.\n");
		return 1;
	}
	pr_pass("A SEGV_MAPERR page fault was issued.\n");
	return 0;
#else
	if (signal_code != SEGV_MAPERR) {
		pr_fail("Signal code is not what we expect.\n");
		return 1;
	}
	pr_pass("A SEGV_MAPERR page fault was issued.\n");
	return 0;
#endif
}

int main (void)
{
	struct sigaction action;
	int ret;

	PRINT_BITNESS;

	memset(&action, 0, sizeof(action));
	action.sa_sigaction = &handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error("Could not set the signal handler for SIGSEGV!\n");
		exit(1);
	}

	if (sigaction(SIGILL, &action, NULL) < 0) {
		pr_error("Could not set the signal handler SIGILL!\n");
		exit(1);
	}

	ret = test_normal_pf();

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error("Could not remove signal SIGSEGV handler!\n");
		return 1;
	}

	if (sigaction(SIGILL, &action, NULL) < 0) {
		pr_error("Could not remove signal SIGILL handler!\n");
		return 1;
	}

	return ret;
}
