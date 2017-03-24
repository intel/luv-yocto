/* Test cases for UMIP */
/* Copyright Intel Corporation 2017 */

#include <stdio.h>
#include <stdlib.h>
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>
#include <asm/prctl.h>
#include <sys/prctl.h>
#include <string.h>
#include <ucontext.h>
#include <signal.h>
#include "umip_test_defs.h"
#include "test_umip_ldt_64.h"
#include "test_umip_code_64.h"

extern unsigned char test_umip[], test_umip_end[];
extern unsigned char finish_testing[];
unsigned long old_fsbase, old_gsbase;
unsigned short old_fs, old_gs;

#define CODE_DESC_INDEX 1
#define DATA_FS_DESC_INDEX 2
#define DATA_GS_DESC_INDEX 3

#define RPL3 3
#define TI_LDT 1
#define SEGMENT_SELECTOR(index) (RPL3 | (TI_LDT << 2) | (index << 3))

static sig_atomic_t got_signal;

void handler(int signum, siginfo_t *info, void *ctx_void)
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

	pr_fail("Whoa! I got a SIGSEGV! Something went wrong!\n");
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

	desc.entry_number = DATA_FS_DESC_INDEX;
	desc.base_addr = (unsigned long)&data_fs;

	memset(data_fs, 0x66, SEGMENT_SIZE);

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		printf("Failed to install data segment [%d].\n", ret);
		return ret;
	}

	desc.entry_number = DATA_GS_DESC_INDEX;
	desc.base_addr = (unsigned long)&data_gs;

	memset(data_gs, 0x55, SEGMENT_SIZE);

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		printf("Failed to install data segment [%d].\n", ret);
		return ret;
	}

	return 0;
}

int main(void)
{
	int ret;
	unsigned short test_fs, test_gs;
	unsigned char *code;
	struct sigaction action;

	PRINT_BITNESS;

	memset(&action, 0, sizeof(action));
	action.sa_sigaction = &handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error("Could not set the signal handler!");
		goto err_out;
	}

	code = mmap(NULL, CODE_MEM_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC,
		    MAP_PRIVATE | MAP_32BIT | MAP_ANONYMOUS, -1, 0);
	if (!code) {
		printf("Failed to allocate memory for code segment!\n");
		goto err_out;
	}

	memcpy(code, test_umip, test_umip_end - test_umip);

	test_fs = SEGMENT_SELECTOR(DATA_FS_DESC_INDEX);
	test_gs = SEGMENT_SELECTOR(DATA_GS_DESC_INDEX);

	ret = setup_data_segments();
	if (ret) {
		pr_error("Failed to setup segments [%d].\n", ret);
		goto err_out;
	}

	syscall(SYS_arch_prctl, ARCH_GET_FS, &old_fsbase);
	syscall(SYS_arch_prctl, ARCH_GET_GS, &old_gsbase);

	asm volatile("movw %%fs, %0" : "=m" (old_fs));
	asm volatile("movw %%gs, %0" : "=m" (old_gs));

	syscall(SYS_arch_prctl, ARCH_SET_FS, (unsigned long)&data_fs);
	syscall(SYS_arch_prctl, ARCH_SET_GS, (unsigned long)&data_gs);

	asm(/* make a backup of everything */
	    "push %%rax\n\t"
	    "push %%rbx\n\t"
	    "push %%rcx\n\t"
	    "push %%rdx\n\t"
	    "push %%rdi\n\t"
	    "push %%rsi\n\t"
	    "push %%rbp\n\t"
	    /* set new data segment */
            /* jump to test code */
	    "call *%2\n\t"
	    /* After running tests, we return here */
            "finish_testing:\n\t"
	    /* restore everything */
	    "pop %%rbp\n\t"
	    "pop %%rsi\n\t"
	    "pop %%rdi\n\t"
	    "pop %%rdx\n\t"
	    "pop %%rcx\n\t"
	    "pop %%rbx\n\t"
	    "pop %%rax\n\t"
	    :
	    :"m"(test_fs), "m"(test_gs), "m"(code)
	   );

	asm volatile("movw %0,%%fs" : :"m" (old_fs));
	asm volatile("movw %0, %%gs" : : "m" (old_gs));
	syscall(SYS_arch_prctl, ARCH_SET_FS, &old_fsbase);
	syscall(SYS_arch_prctl, ARCH_SET_GS, &old_gsbase);

	printf("===Test results===\n");
	check_results();

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error("Could not remove signal handler!");
		exit(1);
	}
	printf("Exiting...\n");

	return 0;
err_out:
	pr_error("Could not run tests\n");
	return 1;
};

