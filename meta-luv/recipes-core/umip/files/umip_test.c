/*
 * tests for Intel User-Mode Execution Prevention
 *
 * GPLv2
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#ifdef __x86_64__
#define GDTR_LEN 10
#define IDTR_LEN 10
#else
#define GDTR_LEN 6
#define IDTR_LEN 6
#endif


void sig_handler(int signum)
{
	printf("Whoa! I got a SIGSEGV. UMIP should not cause it!?\n");
	exit(1);
}

void call_sgdt()
{
	unsigned char val[GDTR_LEN];
	unsigned long base = 0;
	int i;

	for (i = 0; i < GDTR_LEN; i++)
		val[i] = 0;
	printf("Will issue SGDT and save at [0x%lx]\n", val);
	asm volatile("sgdt %0" : "=m" (val));

	printf("GDT Limit [0x%04x]\n", val[1] << 8 | val[0]);

	for (i = 0; i < (GDTR_LEN -2); i++)
		base |= (unsigned long) val[i+2] << ((i * 8));

	printf("GDT Base [0x%016lx]\n", base);
}

void call_sidt()
{
	unsigned char val[IDTR_LEN];
	unsigned long base = 0;
	int i;

	for (i = 0; i < IDTR_LEN; i++)
		val[i] = 0;
	printf("Will issue SIDT and save at [0x%lx]\n", val);
	asm volatile("sidt %0" : "=m" (val));

	printf("IDT Limit [0x%04x]\n", val[1] << 8 | val[0]);

	for (i = 0; i < (IDTR_LEN -2); i++)
		base |= (unsigned long) val[i+2] << ((i * 8));
	printf("IDT Base [0x%016lx]\n", base);
}

void call_sldt()
{
	unsigned long val = 0xa1a1a1a1a1a1a1a1;

	printf("Will issue SLDT and save at [0x%lx]\n", &val);
	asm volatile("sldt %0" : "=m" (val));

	printf("SS for LDT[0x%08lx]\n", val);
}

void call_smsw()
{
	unsigned long val = 0xa2a2a2a2a2a2a2a2;

	printf("Will issue SMSW and save at [0x%lx]\n", &val);
	asm volatile("smsw %0" : "=m" (val));

	printf("CR0[0x%08lx]\n", val);
}

void call_str()
{
	unsigned long val64 = 0xa3a3a3a3a3a3a3a3;
	unsigned int val32 = 0xa4a4a4a4;
	unsigned short val16 = 0xa5a5;

	printf("Will issue STR and save at m64[0x%p]\n", &val64);
	asm volatile("str %0" : "=m" (val64));
	printf("SS for TSS[0x%016lx]\n", val64);

	printf("Will issue STR and save at m32[0x%p]\n", &val32);
	asm volatile("str %0" : "=m" (val32));
	printf("SS for TSS[0x%08x]\n", val32);

	printf("Will issue STR and save at m16[0x%p]\n", &val16);
	asm volatile("str %0" : "=m" (val16));
	printf("SS for TSS[0x%04x]\n", val16);
}

void main(void)
{
	signal(SIGSEGV, sig_handler);
#ifdef __x86_64__
	printf("64 bits\n");
#else
	printf("32 bits\n");
#endif
	call_sgdt();
	call_sidt();
	call_sldt();
	call_smsw();
	call_str();
}
