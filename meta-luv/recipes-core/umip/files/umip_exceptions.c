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
#include <asm/ldt.h>
#include <sys/syscall.h>
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

#define gen_test_lock_prefix_inst(name, inst)				\
static int __test_lock_prefix_##name(void)				\
{									\
	pr_info("Test %s with lock prefix\n", #name);			\
	/* name (%eax) with the LOCK prefix */				\
	asm volatile(inst						\
		      "nop\n"						\
		      "nop\n"						\
		      "nop\n"						\
		      "nop\n");						\
									\
	if (signal_code != ILL_ILLOPN) {				\
		pr_fail("Signal code is not what we expect.\n");	\
		signal_code = 0;					\
		return 1;						\
	}								\
	pr_pass("An ILL_ILLOPN exception was issued.\n");		\
	signal_code = 0;						\
									\
	return 0;							\
}

gen_test_lock_prefix_inst(SMSW, ".byte 0xf0, 0xf, 0x1, 0x20\n")
gen_test_lock_prefix_inst(SIDT, ".byte 0xf0, 0xf, 0x1, 0x8\n")
gen_test_lock_prefix_inst(SGDT, ".byte 0xf0, 0xf, 0x1, 0x0\n")
gen_test_lock_prefix_inst(STR,  ".byte 0xf0, 0xf, 0x0, 0x8\n")
gen_test_lock_prefix_inst(SLDT, ".byte 0xf0, 0xf, 0x0, 0x0\n")

static int test_lock_prefix(void)
{
	int ret = 0;
	ret = __test_lock_prefix_SMSW();
	if (ret)
		return ret;

	ret = __test_lock_prefix_SIDT();
	if (ret)
		return ret;

	ret = __test_lock_prefix_SGDT();
	if (ret)
		return ret;

	ret = __test_lock_prefix_STR();
	if (ret)
		return ret;

	ret = __test_lock_prefix_SLDT();

	return ret;
}

#define gen_test_register_operand_inst(name, inst)			\
static int __test_register_operand_##name(void)				\
{									\
	pr_info("Test %s with register operand\n", #name);		\
	/* name (%eax) with the LOCK prefix */				\
	asm volatile(inst						\
		      "nop\n"						\
		      "nop\n"						\
		      "nop\n"						\
		      "nop\n"						\
		      "nop\n");						\
									\
	if (signal_code != ILL_ILLOPN) {				\
		pr_fail("Signal code is not what we expect.\n");	\
		signal_code = 0;					\
		return 1;						\
	}								\
	pr_pass("An ILL_ILLOPN exception was issued.\n");		\
	signal_code = 0;						\
									\
	return 0;							\
}

gen_test_register_operand_inst(SIDT, ".byte 0xf, 0x1, 0xc8\n")
gen_test_register_operand_inst(SGDT, ".byte 0xf, 0x1, 0xc0\n")

int test_register_operand(void)
{
	int ret;

	signal_code = 0;
	ret = __test_register_operand_SGDT();
	if (ret)
		return 1;

	ret = __test_register_operand_SIDT();
}

#ifdef __x86_64__
int test_null_segment_selectors(void)
{
}
#else
#define gen_test_null_segment_selector(inst, reg)				\
static int __test_null_segment_selector_##inst##_##reg(void)			\
{										\
	pr_info("Test using null seg sel for " #inst " with " #reg "\n");	\
	asm volatile("push %" #reg "\n"						\
		     "push %eax\n"						\
		     "push %ebx\n"						\
		     "mov $0x1000, %eax\n"					\
		     "mov $0, %ebx\n"						\
		     "mov %bx, %" #reg "\n"					\
		     "smsw %" #reg ":(%eax)\n"					\
		     "nop\n"							\
		     "nop\n"							\
		     "nop\n"							\
		     "nop\n"							\
		     "nop\n"							\
		     "pop %ebx\n"						\
		     "pop %eax\n"						\
		     "pop %" #reg "\n");					\
	if (signal_code != SI_KERNEL) {						\
		pr_fail("Signal code is not what we expect.\n");		\
		signal_code = 0;						\
	return 1;								\
	}									\
	pr_pass("An ILL_ILLOPN exception was issued.\n");			\
	signal_code = 0;							\
										\
	return 0;								\
}

gen_test_null_segment_selector(smsw, ds)
gen_test_null_segment_selector(smsw, es)
gen_test_null_segment_selector(smsw, fs)
gen_test_null_segment_selector(smsw, gs)
gen_test_null_segment_selector(sidt, ds)
gen_test_null_segment_selector(sidt, es)
gen_test_null_segment_selector(sidt, fs)
gen_test_null_segment_selector(sidt, gs)
gen_test_null_segment_selector(sgdt, ds)
gen_test_null_segment_selector(sgdt, es)
gen_test_null_segment_selector(sgdt, fs)
gen_test_null_segment_selector(sgdt, gs)
gen_test_null_segment_selector(str, ds)
gen_test_null_segment_selector(str, es)
gen_test_null_segment_selector(str, fs)
gen_test_null_segment_selector(str, gs)
gen_test_null_segment_selector(sldt, ds)
gen_test_null_segment_selector(sldt, es)
gen_test_null_segment_selector(sldt, fs)
gen_test_null_segment_selector(sldt, gs)

int test_null_segment_selectors(void)
{
	int ret;

	ret = __test_null_segment_selector_smsw_ds();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_smsw_es();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_smsw_fs();
	if (ret)
		return 1;
#ifdef TEST_GS /* TODO: Meddling with gs breaks libc */
	ret = __test_null_segment_selector_smsw_gs();
	if (ret)
		return 1;
#endif

	ret = __test_null_segment_selector_sidt_ds();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_sidt_es();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_sidt_fs();
	if (ret)
		return 1;
#ifdef TEST_GS /* TODO: Meddling with gs breaks libc */
	ret = __test_null_segment_selector_sidt_gs();
	if (ret)
		return 1;
#endif

	ret = __test_null_segment_selector_sgdt_ds();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_sgdt_es();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_sgdt_fs();
	if (ret)
		return 1;
#ifdef TEST_GS /* TODO: Meddling with gs breaks libc */
	ret = __test_null_segment_selector_sgdt_gs();
	if (ret)
		return 1;
#endif

	ret = __test_null_segment_selector_sldt_ds();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_sldt_es();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_sldt_fs();
	if (ret)
		return 1;
#ifdef TEST_GS /* TODO: Meddling with gs breaks libc */
	ret = __test_null_segment_selector_sldt_gs();
	if (ret)
		return 1;
#endif

	ret = __test_null_segment_selector_str_ds();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_str_es();
	if (ret)
		return 1;
	ret = __test_null_segment_selector_str_fs();
	if (ret)
		return 1;
#ifdef TEST_GS /* TODO: Meddling with gs breaks libc */
	ret = __test_null_segment_selector_str_gs();
	if (ret)
		return 1;
#endif
	return 0;
}
#endif

#ifdef __x86_64__
int test_addresses_outside_segment(void)
{
}
#else

#define SEGMENT_SIZE 0x1000
#define CODE_DESC_INDEX 1
#define DATA_DESC_INDEX 2

#define RPL3 3
#define TI_LDT 1

#define SEGMENT_SELECTOR(index) (RPL3 | (TI_LDT << 2) | (index << 3))

unsigned char custom_segment[SEGMENT_SIZE];

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

	desc.entry_number = DATA_DESC_INDEX;
	desc.base_addr = (unsigned long)&custom_segment;

	ret = syscall(SYS_modify_ldt, 1, &desc, sizeof(desc));
	if (ret) {
		pr_error("Failed to install stack semgnet [%d].\n", ret);
		return ret;
	}

	return 0;
}

#define gen_test_addresses_outside_segment(inst, sel)			\
int __test_addresses_outside_segment_##inst##_##sel(void)		\
{									\
	int ret;							\
	unsigned short seg_sel;						\
									\
	seg_sel = SEGMENT_SELECTOR(DATA_DESC_INDEX);			\
									\
	pr_info("Test address outside of segment limit for " #inst " with " #sel"\n");\
	asm volatile("push %%" #sel"\n"					\
		     "push %%eax\n"					\
		     "push %%ebx\n"					\
		     "mov $0x2000, %%eax\n"				\
		     "mov %0, %%" #sel "\n"				\
		     #inst " %%" #sel ":(%%eax)\n"			\
		     "nop\n"						\
		     "nop\n"						\
		     "nop\n"						\
		     "nop\n"						\
		     "nop\n"						\
		     "pop %%ebx\n"					\
		     "pop %%eax\n"					\
		     "pop %%" #sel "\n"					\
		     :							\
		     :"m" (seg_sel));					\
									\
	if (signal_code != SI_KERNEL) {					\
		pr_fail("Signal code is not what we expect.\n");	\
		signal_code = 0;					\
	return 1;							\
	}								\
	pr_pass("An ILL_ILLOPN exception was issued.\n");		\
	signal_code = 0;						\
									\
	return 0;							\
}

gen_test_addresses_outside_segment(smsw, ds)
gen_test_addresses_outside_segment(str, ds)
gen_test_addresses_outside_segment(sldt, ds)
gen_test_addresses_outside_segment(sgdt, ds)
gen_test_addresses_outside_segment(sidt, ds)
gen_test_addresses_outside_segment(smsw, es)
gen_test_addresses_outside_segment(str, es)
gen_test_addresses_outside_segment(sldt, es)
gen_test_addresses_outside_segment(sgdt, es)
gen_test_addresses_outside_segment(sidt, es)
gen_test_addresses_outside_segment(smsw, fs)
gen_test_addresses_outside_segment(str, fs)
gen_test_addresses_outside_segment(sldt, fs)
gen_test_addresses_outside_segment(sgdt, fs)
gen_test_addresses_outside_segment(sidt, fs)
gen_test_addresses_outside_segment(smsw, gs)
gen_test_addresses_outside_segment(str, gs)
gen_test_addresses_outside_segment(sldt, gs)
gen_test_addresses_outside_segment(sgdt, gs)
gen_test_addresses_outside_segment(sidt, gs)

int test_addresses_outside_segment(void)
{
	int ret;

	ret = setup_data_segments();
	if (ret)
		return 1;

	ret = __test_addresses_outside_segment_smsw_ds();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_str_ds();
	if (ret)
			return 1;

		ret = __test_addresses_outside_segment_sgdt_ds();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_sidt_ds();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_sldt_ds();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_smsw_es();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_str_es();
	if (ret)
			return 1;

		ret = __test_addresses_outside_segment_sgdt_es();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_sidt_es();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_sldt_es();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_smsw_fs();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_str_fs();
	if (ret)
			return 1;

		ret = __test_addresses_outside_segment_sgdt_fs();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_sidt_fs();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_sldt_fs();
	if (ret)
			return 1;

#ifdef TEST_GS
	ret = __test_addresses_outside_segment_smsw_gs();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_str_gs();
	if (ret)
			return 1;

		ret = __test_addresses_outside_segment_sgdt_gs();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_sidt_gs();
	if (ret)
			return 1;

	ret = __test_addresses_outside_segment_sldt_gs();
	if (ret)
			return 1;
#endif
}
#endif

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
	if (ret)
		return 1;

	ret = test_lock_prefix();
	if (ret)
		return 1;

	ret = test_register_operand();
	if (ret)
		return 1;

	ret = test_null_segment_selectors();
	if (ret)
		return 1;

	test_addresses_outside_segment();

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
