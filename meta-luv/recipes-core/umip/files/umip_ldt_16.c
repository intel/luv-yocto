/* Test cases for UMIP */
/* Copyright Intel Corporation 2017 */

#include <stdio.h>
#include <stdlib.h>
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "umip_test_defs.h"
#include "test_umip_ldt_16.h"
#include "test_umip_code_16.h"

extern unsigned char test_umip[], test_umip_end[];
extern unsigned char interim[], interim_start[], interim_end[];
extern unsigned char finish_testing[];
extern unsigned char data[SEGMENT_SIZE];
extern unsigned char data_es[SEGMENT_SIZE];
extern unsigned char data_fs[SEGMENT_SIZE];
extern unsigned char data_gs[SEGMENT_SIZE];
extern unsigned char stack_32[SEGMENT_SIZE];
extern unsigned char stack[SEGMENT_SIZE];
extern int exit_on_signal;
unsigned short cs_orig;

#define CODE_DESC_INDEX 1
#define CODE_16_DESC_INDEX 2
#define DATA_DESC_INDEX 3
#define STACK_16_DESC_INDEX 4
#define STACK_DESC_INDEX 5
#define DATA_ES_DESC_INDEX 6
#define DATA_FS_DESC_INDEX 7
#define DATA_GS_DESC_INDEX 8

#define RPL3 3
#define TI_LDT 1
#define SEGMENT_SELECTOR(index) (RPL3 | (TI_LDT << 2) | (index << 3))

int test_passed, test_failed, test_errors;

asm(".pushsection .rodata\n\t"
	"interim:\n\t"
	/* this is the return point */
	"interim_return:\n\t"
	/* restore our own stack */
	"mov %ebx, %ss\n\t"
	"mov %eax, %esp\n\t"
	/* restore ss of caller */
	"pop %ebx\n\t"
	/* restore esp of caller*/
	"pop %eax\n\t"
	/* prepare to return, set IP, CS is already in stack */
	"push $finish_testing\n\t"
	"retf\n\t"
	/* this is the interim start */
	"interim_start:\n\t"
	/* setup stack */
	"mov $4096, %esp\n\t"
	"mov %ecx, %ss\n\t"
	/* save old cs */
	"push %edx\n\t"
	/* save old esp */
	"push %eax\n\t"
	/* save old ss */
	"push %ebx\n\t"
	/* prepare to jump */
	/* pass current stack pointer, ss and cs */
	/* this is to know where we need to return to */
	"mov %esp, %eax\n\t"
	"mov %ss, %ebx\n\t"
	"mov %cs, %edx\n\t"
	"push %edi\n\t"
	"push $0\n\t"
	"retf\n\t"
	"interim_end:\n\t"
	".popsection\n\t"
	);

static int setup_data_segments()
{
	int ret;
	struct user_desc desc = {
		.entry_number    = 0,
		.base_addr       = 0,
		.limit           = SEGMENT_SIZE,
		.seg_32bit       = 0,
		.contents        = 0, /* data */
		.read_exec_only  = 0,
		.limit_in_pages  = 0,
		.seg_not_present = 0,
		.useable         = 1
	};

	desc.entry_number = STACK_DESC_INDEX;
	desc.base_addr = (unsigned long)&stack_32;

	memset(stack_32, 0x88, SEGMENT_SIZE);

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		printf("Failed to install stack semgnet [%d].\n", ret);
		return ret;
	}

	desc.entry_number = STACK_16_DESC_INDEX;
	desc.base_addr = (unsigned long)&stack;

	memset(stack, 0x44, SEGMENT_SIZE);

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

	memset(data_fs, 0x66, SEGMENT_SIZE);

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		pr_error(test_errors, "Failed to install data segment [%d].\n", ret);
		return ret;
	}

	desc.entry_number = DATA_GS_DESC_INDEX;
	desc.base_addr = (unsigned long)&data_gs;

	memset(data_gs, 0x55, SEGMENT_SIZE);

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
	unsigned short interim_cs, interim_ss;
	unsigned short test_cs_16, test_ds_16, test_ss_16;
	unsigned short test_es_16, test_fs_16, test_gs_16;
	unsigned long interim_start_addr;
	unsigned char *code_interim, *code_16;
	struct sigaction action;

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
	action.sa_sigaction = &signal_handler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);

	exit_on_signal = 1;

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error(test_errors, "Could not set the signal handler!");
		goto err_out;
	}

	code_interim = mmap(NULL, 4096, PROT_WRITE | PROT_READ | PROT_EXEC,
		    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (!code_interim) {
		pr_error(test_errors, "Failed to allocate memory for interim code segment!\n");
		goto err_out;
	}

	memcpy(code_interim, interim, interim_end - interim);

	code_16 = mmap(NULL, CODE_MEM_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC,
		    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (!code_16) {
		pr_error(test_errors, "Failed to allocate memory for code segment!\n");
		goto err_out;
	}

	memcpy(code_16, test_umip, test_umip_end - test_umip);

	/* install our 32-bit intermediate code segment */
	code_desc.base_addr = (unsigned long)code_interim;
	code_desc.limit = interim_end - interim + 100;

	ret = syscall(SYS_modify_ldt, 1, &code_desc, sizeof(code_desc));
	if (ret) {
		pr_error(test_errors, "Failed to install interim code segment [%d].\n", ret);
		goto err_out;
	}

	/* install our 16-bit code segment */
	code_desc.entry_number = CODE_16_DESC_INDEX,
	code_desc.base_addr = (unsigned long)code_16;
	code_desc.seg_32bit = 0,
	code_desc.limit = test_umip_end - test_umip + 100;

	ret = syscall(SYS_modify_ldt, 1, &code_desc, sizeof(code_desc));
	if (ret) {
		pr_error(test_errors, "Failed to install 16-bit code segment [%d].\n", ret);
		goto err_out;
	}

	if (setup_data_segments()) {
		pr_error(test_errors, "Failed to setup segments [%d].\n", ret);
		goto err_out;
	}

	interim_cs = SEGMENT_SELECTOR(CODE_DESC_INDEX);
	interim_ss = SEGMENT_SELECTOR(STACK_DESC_INDEX);
	test_cs_16 = SEGMENT_SELECTOR(CODE_16_DESC_INDEX);
	test_ss_16 = SEGMENT_SELECTOR(STACK_16_DESC_INDEX);
	test_ds_16 = SEGMENT_SELECTOR(DATA_DESC_INDEX);
	test_es_16 = SEGMENT_SELECTOR(DATA_ES_DESC_INDEX);
	test_fs_16 = SEGMENT_SELECTOR(DATA_FS_DESC_INDEX);
	test_gs_16 = SEGMENT_SELECTOR(DATA_GS_DESC_INDEX);

	/*
	 * We cannot use the object's interim_start label as it we
	 * have copied our code to mmap'ed memory. Thus, we need
	 * calculate it as an offset since the beginning of the
	 * section
	 */
	 interim_start_addr = interim_start - interim;

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
	    "mov %[test_ds_16], %%ds\n\t"
	    "mov %[test_es_16], %%es\n\t"
	    "mov %[test_fs_16], %%fs\n\t"
	    "mov %[test_gs_16], %%gs\n\t"
	    /*
	     * Save current stack settings and pass them to the new code segment
	     * via registers. We will save these as soon as we setup a new stack
	     * segment.
	     */
	    "mov %%esp, %%eax\n\t"
	    "mov %%ss, %%ebx\n\t"
	    /*
	     * Give interim code the new stack segment. We cannot set it here as
	     * we need it to jump to the test code via retf
	     */
	    "mov %[interim_ss], %%ecx\n\t"
	    /*
	     * Pass the code segment selector to the interim code so that it knows
	     * where to return
	     */
	    "mov %%cs, %%edx\n\t"
	    /*
	     * Pass the code and stacks segment selectors of the 16-bit code
	     * as we only know them from this context.
	     */
	    "mov %[test_ss_16], %%esi\n\t"
	    "mov %[test_cs_16], %%edi\n\t"
	    /*
	     *ljmp only takes constants. Instead use retf, which takes
	     * instruction pointer and code segment selector from the stack
	     */
	    "push %[interim_cs]\n\t"
	    "push %[interim_start_addr]\n\t"
	    "retf \n\t"
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
	    : [test_ds_16]"m"(test_ds_16), [test_es_16]"m"(test_es_16),
	      [test_fs_16]"m"(test_fs_16), [test_gs_16]"m"(test_gs_16),
	      [interim_ss]"m"(interim_ss), [test_ss_16]"m"(test_ss_16),
	      [test_cs_16]"m"(test_cs_16), [interim_cs]"m"(interim_cs),
	      [interim_start_addr]"m"(interim_start_addr)
	);

	pr_info("===Test results===\n");

	check_results();

	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	sigemptyset(&action.sa_mask);

	if (sigaction(SIGSEGV, &action, NULL) < 0) {
		pr_error(test_errors, "Could not remove signal handler!");
		goto err_out;
	}

	printf("Exiting...\n");
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
