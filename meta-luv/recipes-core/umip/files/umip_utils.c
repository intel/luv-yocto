/*
 * tests for Intel User-Mode Execution Prevention
 *
 * GPLv2
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "umip_test_defs.h"

extern int test_passed, test_failed, test_errors;
sig_atomic_t got_signal, got_sigcode;
int exit_on_signal;

void print_results(void)
{
	printf("RESULTS: passed[%d], failed[%d], errors[%d].\n",
	       test_passed, test_failed, test_errors);
}

#if 0
int inspect_signal(int exp_signum, int exp_sigcode)
{
	if (got_signal == exp_signum && got_sigcode == exp_sigcode) {
		if (exp_signum && exp_sigcode)
			pr_pass(test_passed, "Received expected signal.\n");
		return 0;

	} else {
		if (got_signal && exp_sigcode)
			pr_fail(test_failed, "Received unexpected signal.\n");
		else
			pr_fail(test_failed, "Did not receive signal. A signal [%d] was expected.\n", exp_signum);
		return 1;
	}
}
#else

/**
 * Inspect the contents of exp_signum and exp_sigcode to determine if they match
 * the received got_signal signal and signal code, if any. A return value of
 * 1 means that the test case is complete and no further action is needed (e.g.,
 * examine values returned by instructions. A return value of 0 means that signal
 * processing is not relevant for the caller can proceed with further test case
 * validation.
 */
int inspect_signal(exp_signum, exp_sigcode)
{
	/* If we expect signal, make sure it is the one we expect. */
	if (exp_signum) {
		/* A signal was received, examine it */
		if (got_signal == exp_signum) {
			if (got_sigcode == exp_sigcode) {
				/* All is good. Test case is complete. */
				pr_pass(test_passed, "Received expected signal and code.\n");
				return 1;
			} else {
				pr_fail(test_failed, "Received wrong signal code. Expected si_code[%d].\n", exp_sigcode);
				return 1;
			}
		} else {
			if (got_signal) {
				/* Wrong signal */
				pr_fail(test_failed, "Received wrong signal. Expected [%d]\n", exp_signum);
				return 1;
			} else {
				/* signal did not come */
				pr_fail(test_failed, "A signal [%d] was expected. None was not received.\n", exp_signum);
				return 1;
			}
		}
	} else { /* If no signal is expected, make sure we did not receive one */
		if (got_signal) {
			pr_fail(test_failed, "Received unexpected signal.\n");
			return 1;
		} else { /* Signal is not relevant.*/
			return 0;
		}
	}
}
#endif

void signal_handler(int signum, siginfo_t *info, void *ctx_void)
{
	ucontext_t *ctx = (ucontext_t *)ctx_void;

	pr_info("si_signo[%d]\n", info->si_signo);
	pr_info("si_errno[%d]\n", info->si_errno);
	pr_info("si_code[%d]\n", info->si_code);
	pr_info("si_addr[0x%p]\n", info->si_addr);

	got_signal = signum;

	if (signum == SIGSEGV) {
		if (info->si_code == SEGV_MAPERR)
			pr_info("Signal because of unmapped object.\n");
		else if (info->si_code == SI_KERNEL)
			pr_info("Signal because of #GP\n");
		else
			pr_info("Unknown si_code!\n");
	} else if (signum == SIGILL) {
		if (info->si_code == SEGV_MAPERR)
			pr_info("Signal because of unmapped object.\n");
		else if (info->si_code == ILL_ILLOPN)
			pr_info("Signal because of #UD\n");
		else
			pr_info("Unknown si_code!\n");
	} else {
		pr_error(test_errors, "Received signal that I cannot handle!\n");
		exit(1);
	}

	/* Save the signal code */
	got_sigcode = info->si_code;

	/*
	 * Move to the next instruction; to move, increment the instruction
	 * pointer by 10 bytes. 10 bytes is the size of the instruction
	 * considering two prefix bytes, two opcode bytes, one
	 * ModRM byte, one SIB byte and 4 displacement bytes. We have
	 * a NOP sled after the instruction to ensure we continue execution
	 * safely in case we overestimate the size of the instruction.
	 */

	if (exit_on_signal) {
		pr_fail(test_failed, "Whoa! I got a signal! Something went wrong!\n");
		exit(1);
	}
#ifdef __x86_64__
	ctx->uc_mcontext.gregs[REG_RIP] += 10;
#else
	ctx->uc_mcontext.gregs[REG_EIP] += 10;
#endif
}
