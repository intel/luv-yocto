/*
 * tests for Intel User-Mode Execution Prevention
 *
 * GPLv2
 */
#include <stdio.h>

extern int test_passed, test_failed, test_errors;

void print_results(void)
{
	printf("RESULTS: passed[%d], failed[%d], error[%d]\n",
	       test_passed, test_failed, test_errors);
}
