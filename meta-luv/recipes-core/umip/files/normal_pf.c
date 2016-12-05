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

static sig_atomic_t signal_code;
static sig_atomic_t got_signal;

void handler(int signum, siginfo_t *info, void *ctx_void)
{
	ucontext_t *ctx = (ucontext_t *)ctx_void;

        printf("INFO: si_signo[%d]\n", info->si_signo);
        printf("INFO: si_errno[%d]\n", info->si_errno);
        printf("INFO: si_code[%d]\n", info->si_code);
        printf("INFO: si_code[0x%p]\n", info->si_addr);
	if (signum != SIGSEGV)
		errx(1, "ERROR: Received unexpected signal");
	else
		got_signal = signum;
	if (info->si_code == SEGV_MAPERR)
		printf("INFO: Signal because of unmapped object.\n");
	else if (info->si_code == SI_KERNEL)
		printf("INFO: Signal because of #GP\n");
	else
		printf("INFO: Unknown si_code!\n");
	/* Save the signal code */
	signal_code = info->si_code;
	/*
	 * Move to the next instruction. We have a cushion of
	 * several NOPs. Thus, we can safely move 8 positions
	 */
#ifdef __x86_64__
	ctx->uc_mcontext.gregs[REG_RIP] += 8;
#else
	ctx->uc_mcontext.gregs[REG_EIP] += 8;
#endif
}

struct my_struct {
	int a;
	int b;
	int c;
};

void main (void)
{
	struct sigaction action;
	unsigned long val;
	unsigned long *val_bad = (unsigned long *)0x100000;

	memset(&action, 0, sizeof(action));
	action.sa_sigaction = &handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0)
		errx(1, "ERROR: Could not set the signal handler!");

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
		printf("FAIL: Signal not received!\n");
		return;
	}
	if (signal_code != SEGV_MAPERR)
		printf("FAIL: Signal code is not what we expect.\n");
	else
		printf("SUCCESS: A SEGV_MAPERR page fault was issued.\n");
		
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0)
		errx(1, "Error when removing signal handler!");
}
