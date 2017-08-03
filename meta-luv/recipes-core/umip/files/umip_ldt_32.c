/* Test cases for UMIP */
/* Copyright Intel Corporation 2017 */

#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <ucontext.h>
#include <signal.h>
#include "umip_test_defs.h"
#include "test_umip_ldt_32.h"
#include "test_umip_code_32.h"

extern unsigned char test_umip[], test_umip_end[];
extern unsigned char finish_testing[];
unsigned short cs_orig;

#define CODE_DESC_INDEX 1
#define DATA_DESC_INDEX 2
#define STACK_DESC_INDEX 3
#define DATA_ES_DESC_INDEX 4
#define DATA_FS_DESC_INDEX 5
#define DATA_GS_DESC_INDEX 6

#define RPL3 3
#define TI_LDT 1
#define SEGMENT_SELECTOR(index) (RPL3 | (TI_LDT << 2) | (index << 3))

static sig_atomic_t got_signal;

extern int test_passed, test_failed, test_errors;

static void print_results(void)
{
	printf("RESULTS: passed[%d], failed[%d], error[%d]\n",
	       test_passed, test_failed, test_errors);
}

void handler(int signum, siginfo_t *info, void *ctx_void)
{
        pr_info("si_signo[%d]\n", info->si_signo);
        pr_info("si_errno[%d]\n", info->si_errno);
        pr_info("si_code[%d]\n", info->si_code);
        pr_info("si_addr[0x%p]\n", info->si_addr);
	if (signum != SIGSEGV)
		pr_error(test_errors, "Received unexpected signal");
	else
		got_signal = signum;
	if (info->si_code == SEGV_MAPERR)
		pr_info("Signal because of unmapped object.\n");
	else if (info->si_code == SI_KERNEL)
		pr_info("Signal because of #GP\n");
	else
		pr_info("Unknown si_code!\n");

	pr_fail(test_failed, "Whoa! I got a SIGSEGV! Something went wrong!\n");
	print_results();
	exit(1);
}

static int setup_data_segments()
{
	int ret;
	struct user_desc desc = {
		.entry_number    = 0,
		.base_addr       = 0,
		.limit           = SEGMENT_SIZE,
		.seg_32bit       = 1,
		.contents        = 0, /* data */
		.read_exec_only  = 0,
		.limit_in_pages  = 0,
		.seg_not_present = 0,
		.useable         = 1
	};

	desc.entry_number = STACK_DESC_INDEX;
	desc.base_addr = (unsigned long)&stack;

	memset(stack, 0x88, SEGMENT_SIZE);

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		pr_error(test_errors, "Failed to install stack semgnet [%d].\n", ret);
		return ret;
	}

	desc.entry_number = DATA_DESC_INDEX;
	desc.base_addr = (unsigned long)&data;

	memset(data, 0x99, SEGMENT_SIZE);

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		pr_error(test_errors, "Failed to install data segment [%d].\n", ret);
		return ret;
	}

	desc.entry_number = DATA_ES_DESC_INDEX;
	desc.base_addr = (unsigned long)&data_es;

	memset(data_es, 0x77, SEGMENT_SIZE);

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		pr_error(test_errors, "Failed to install data segment [%d].\n", ret);
		return ret;
	}

	desc.entry_number = DATA_FS_DESC_INDEX;
	desc.base_addr = (unsigned long)&data_fs;

	memset(data_es, 0x66, SEGMENT_SIZE);

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		pr_error(test_errors, "Failed to install data segment [%d].\n", ret);
		return ret;
	}

	desc.entry_number = DATA_GS_DESC_INDEX;
	desc.base_addr = (unsigned long)&data_gs;

	memset(data_es, 0x55, SEGMENT_SIZE);

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		pr_error(test_errors, "Failed to install data segment [%d].\n", ret);
		return ret;
	}

	return 0;
}

int run_umip_ldt_test(void)
{
	int ret;
	unsigned short test_cs, test_ds, test_ss;
	unsigned short test_es, test_fs, test_gs;
	struct sigaction action;
	unsigned char *code;

	struct user_desc code_desc = {
	.entry_number    = CODE_DESC_INDEX,
	.seg_32bit       = 1,
	.contents        = 2, /* non-conforming */
	.read_exec_only  = 1,
	.limit_in_pages  = 0,
	.seg_not_present = 0,
	.useable         = 1
	};

	PRINT_BITNESS;

	memset(&action, 0, sizeof(action));
	action.sa_sigaction = &handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error(test_errors, "Could not set the signal handler!");
		goto err_out;
	}

	code = mmap(NULL, CODE_MEM_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC,
		    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (!code) {
		pr_error(test_errors, "Failed to allocate memory for code segment!\n");
		goto err_out;
	}

	memcpy(code, test_umip, test_umip_end - test_umip);

	code_desc.base_addr = (unsigned long)code;
	code_desc.limit = test_umip_end - test_umip + 100;

	ret = syscall(SYS_modify_ldt, 1, &code_desc, sizeof(code_desc));
	if (ret) {
		pr_error(test_errors, "Failed to install code segment [%d].\n", ret);
		goto err_out;
	}

	if (setup_data_segments()) {
		pr_error(test_errors, "Failed to setup segments [%d].\n", ret);
		goto err_out;
	}

	test_cs = SEGMENT_SELECTOR(CODE_DESC_INDEX);
	test_ds = SEGMENT_SELECTOR(DATA_DESC_INDEX);
	test_ss = SEGMENT_SELECTOR(STACK_DESC_INDEX);
	test_es = SEGMENT_SELECTOR(DATA_ES_DESC_INDEX);
	test_fs = SEGMENT_SELECTOR(DATA_FS_DESC_INDEX);
	test_gs = SEGMENT_SELECTOR(DATA_GS_DESC_INDEX);

	asm(/* make a backup of everything */
	    "push %%ds\n\t"
	    "push %%es\n\t"
	    "push %%fs\n\t"
	    "push %%gs\n\t"
	    "push %%eax\n\t"
	    "push %%ebx\n\t"
	    "push %%ecx\n\t"
	    "push %%edx\n\t"
	    "push %%edi\n\t"
	    "push %%esi\n\t"
	    "push %%ebp\n\t"
	    /* set new data segment */
            "mov %0, %%ds\n\t"
            "mov %1, %%es\n\t"
            "mov %2, %%fs\n\t"
            "mov %3, %%gs\n\t"
	    /*
	     * Save current stack settings and pass them to the new code segment
	     * via registers. We will save these as soon as we setup a new stack
	     * segment.
	     */
	    "mov %%esp, %%eax\n\t"
	    "mov %%ss, %%ebx\n\t"
	    /*
	     * Give test code the new stack segment. We cannot set it here as
	     * we need it to jump to the test code via retf
	     */
	    "mov %4, %%ecx\n\t"
	    /*
	     * Pass the code segment selector to the new code so that it knows
	     * where to return
	     */
	    "mov %%cs, %%edx\n\t"
	    /*
	     *  ljmp only takes constants. Instead use retf, which pops
	     * instruction pointer and code segment selector from the stack
	     */
	    "push %5\n\t"
	    /* jump to the beginning of the new segment */
	    "push $0\n\t"
	    /* Everything is set. Make the jump */
	    "retf \n\t"
	    /* After running tests, we return here */
            "finish_testing:\n\t"
	    /* restore our stack */
	    "mov %%ebx, %%ss\n\t"
	    "mov %%eax, %%esp\n\t"
	    /* restore everything */
	    "pop %%ebp\n\t"
	    "pop %%esi\n\t"
	    "pop %%edi\n\t"
	    "pop %%edx\n\t"
	    "pop %%ecx\n\t"
	    "pop %%ebx\n\t"
	    "pop %%eax\n\t"
	    "pop %%gs\n\t"
	    "pop %%fs\n\t"
	    "pop %%es\n\t"
	    "pop %%ds\n\t"
	    :
	    :"m"(test_ds), "m"(test_es), "m"(test_fs), "m"(test_gs),
	     "m"(test_ss), "m"(test_cs)
	   );

	pr_info("===Test results===\n");
	check_results();

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error(test_errors, "Could not remove signal handler!");
		print_results();
		exit(1);
	}

	pr_info("Exiting...\n");

	print_results();
	return 0;
err_out:
	pr_error(test_errors, "Could not run tests\n");
	print_results();
	return 1;

};

int main(void)
{
	return run_umip_ldt_test();
}
