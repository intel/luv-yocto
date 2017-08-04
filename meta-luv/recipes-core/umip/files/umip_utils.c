/*
 * tests for Intel User-Mode Execution Prevention
 *
 * GPLv2
 */
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "umip_test_defs.h"

extern int test_passed, test_failed, test_errors;
extern sig_atomic_t got_signal, got_sigcode;

void print_results(void)
{
	printf("RESULTS: passed[%d], failed[%d], error[%d]\n",
	       test_passed, test_failed, test_errors);
}

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
