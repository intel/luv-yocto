#ifndef _UMIP_TEST_DEFS_H
#define _UMIP_TEST_DEFS_H
#include <stdio.h>

#define TEST_PASS "\x1b[32m[pass]\x1b[0m "
#define TEST_FAIL "\x1b[31m[FAIL]\x1b[0m "
#define TEST_INFO "\x1b[34m[info]\x1b[0m "
#define TEST_ERROR "\x1b[33m[ERROR]\x1b[0m "

#define pr_pass(...) printf(TEST_PASS __VA_ARGS__)
#define pr_fail(...) printf(TEST_FAIL __VA_ARGS__)
#define pr_info(...) printf(TEST_INFO __VA_ARGS__)
#define pr_error(...) printf(TEST_ERROR __VA_ARGS__)

#ifdef __x86_64__
#define PRINT_BITNESS pr_info("This binary uses 64-bit code\n")
#define INIT_VAL(val) (val)
#else
#define PRINT_BITNESS pr_info("This binary uses 32-bit code\n")
#define INIT_VAL(val) (val & 0xffffffff)
#endif

#define EXPECTED_SMSW 0x33
#define EXPECTED_SLDT 0x0
#define EXPECTED_STR 0x0
#define EXPECTED_GDT_BASE 0xfffe0000
#define EXPECTED_GDT_LIMIT 0x0
#define EXPECTED_IDT_BASE 0xffff0000
#define EXPECTED_IDT_LIMIT 0x0

struct table_desc {
	unsigned short limit;
	unsigned long base;
} __attribute__((packed));

static const unsigned short expected_msw = EXPECTED_SMSW;
static const unsigned short expected_ldt = EXPECTED_SLDT;
static const unsigned short expected_tr = EXPECTED_STR;

static const struct table_desc expected_gdt = {
	.limit = EXPECTED_GDT_LIMIT,
	.base = EXPECTED_GDT_BASE
};

static const struct table_desc expected_idt = {
	.limit = EXPECTED_IDT_LIMIT,
	.base = EXPECTED_IDT_BASE
};

#endif /* _UMIP_TEST_DEFS_H */
