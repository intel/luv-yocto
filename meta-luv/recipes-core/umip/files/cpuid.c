#include <stdio.h>
#include <stdlib.h>

#define UMIP_MASK 0x4

void usage(void)
{
	printf("Usage:\n");
	printf("cpuid [eax] [ecx]\n");
	printf("Example:\n");
	printf("cpuid 0x4 0x0\n");
}

void print_umip(unsigned int ecx)
{
	printf("CPU has UMIP? %s\n", ecx & UMIP_MASK ? "yes" : "no");
}

void get_cpuid_leaf(unsigned int eax, unsigned int ecx, unsigned int *new_eax,
		    unsigned int *new_ecx, unsigned *new_edx)
{
	printf("Getting CPUID leaf for eax=0x%x ecx=0x%x\n", eax, ecx);

	asm volatile("mov %3, %%eax\n"
		     "mov %4, %%ecx\n"
		     "cpuid\n"
		     "mov %%eax, %0\n"
		     "mov %%ecx, %1\n"
		     "mov %%edx, %2\n"
		     : "=m"(*new_eax), "=m"(*new_ecx), "=m"(*new_edx)
		     : "m"(eax), "m"(ecx)
		     : "eax", "ecx", "edx");
}

int main(int argc, char **argv)
{
	unsigned int eax, ecx, edx;
	unsigned int new_eax = 0xabababab, new_ecx = 0xcbcbcbcb;
	unsigned int new_edx = 0xefefefef;

	if (argc != 3) {
		usage();
		return 1;
	}

	eax = strtoul(argv[1], NULL, 0);
	ecx = strtoul(argv[2], NULL, 0);

	get_cpuid_leaf(eax, ecx, &new_eax, &new_ecx, &new_edx);

        printf("CPUID Result:\n");
        printf("eax = 0x%x\n", new_eax);
        printf("ecx = 0x%x\n", new_ecx);
        printf("edx = 0x%x\n", new_edx);

	print_umip(new_ecx);
}
