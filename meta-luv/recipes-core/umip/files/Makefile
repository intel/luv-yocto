x86_64:
	$(CC) $(CFLAGS) -c umip_utils.c -o umip_utils_64.o

	$(CC) $(CFLAGS) -o umip_test2_64 umip_utils_64.o umip_test2.c
	$(CC) $(CFLAGS) -o umip_test_64 umip_utils_64.o umip_test.c
	$(CC) $(CFLAGS) -o umip_exceptions_64 umip_utils_64.o umip_exceptions.c

	python umip_test_gen_64.py
	$(CC) $(CFLAGS) -c test_umip_ldt_64.c
	$(CC) $(CFLAGS) -c umip_ldt_64.c
	$(CC) $(CFLAGS) -o umip_ldt_64 test_umip_ldt_64.o umip_ldt_64.o umip_utils_64.o

i586: i686

i686:
	# fo 32-bit builds, use -m32 if building independently
	$(CC) $(CFLAGS) -c umip_utils.c -o umip_utils_32.o

	$(CC) $(CFLAGS) -o umip_test2_32 umip_utils_32.o umip_test2.c
	$(CC) $(CFLAGS) -o umip_test_32 umip_utils_32.o umip_test.c
	$(CC) $(CFLAGS) -o umip_exceptions_32 umip_utils_32.o umip_exceptions.c

	python umip_test_gen_32.py
	$(CC) $(CFLAGS) -c test_umip_ldt_32.c
	$(CC) $(CFLAGS) -c umip_ldt_32.c
	$(CC) $(CFLAGS) -o umip_ldt_32 test_umip_ldt_32.o umip_ldt_32.o umip_utils_32.o

	python umip_test_gen_16.py
	$(CC) $(CFLAGS) -c test_umip_ldt_16.c
	$(CC) $(CFLAGS) -c umip_ldt_16.c
	$(CC) $(CFLAGS) -o umip_ldt_16 test_umip_ldt_16.o umip_ldt_16.o umip_utils_32.o


.PHONY: clean
clean:
	rm -f *.o
	rm -f umip_test
	rm -f umip_test2_64
	rm -f umip_test2_32
	rm -f umip_pf
	rm -f umip_ldt_64
	rm -f umip_ldt_32
	rm -f umip_ldt_16
