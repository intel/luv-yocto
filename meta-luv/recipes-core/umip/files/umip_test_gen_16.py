""" Test-case generator for UMIP
Copyright Intel Corporation 2017

This file generates opcdoes for all (well, most) of the combinations
of memory operands for the UMIP-protected instructions. This includes
all the ModRM values and segments.

TODO: Add support for 16-bit address encodings
TODO: Add support for SiB address encodings"""

MODRM_MO0 = 0
MODRM_MO1 = 1
MODRM_MO2 = 2
MODRM_MO3 = 3

#TODO: need to be based on the number test cases
SEGMENT_SIZE = 32768
CODE_MEM_SIZE = 32768

TEST_PASS_CTR_VAR = "test_passed"
TEST_FAIL_CTR_VAR = "test_failed"
TEST_ERROR_CTR_VAR = "test_errors"

class Instruction:
	def __init__(self, name, opcode, modrm_reg, result_bytes, expected_val):
		self.name = name
		self.opcode = opcode
		self.modrm_reg = modrm_reg
		self.result_bytes = result_bytes
		self.expected_val = expected_val


class Register:
	def __init__(self, mnemonic, modrm_rm, name):
		self.mnemonic = mnemonic
		self.modrm_rm = modrm_rm
		self.name = name

class Segment:
	def __init__(self, name, prefix, array):
		self.name = name
		self.prefix = prefix
		self.array = array


BX_SI = Register(["%bx", "%si"], 0, ["bx", "si"])
BX_DI = Register(["%bx", "%di"], 1, ["bx", "di"])
BP_SI = Register(["%bp", "%si"], 2, ["bp", "si"])
BP_DI = Register(["%bp", "%di"], 3, ["bp", "di"])
SI = Register(["%si"], 4, ["si"])
DI = Register(["%di"], 5, ["di"])
BP = Register(["%bp"], 6, ["bp"])
BX = Register(["%bx"], 7, ["bx"])

SMSW = Instruction("smsw", "0xf, 0x1", 4, 2, "expected_msw")
SLDT = Instruction("sldt", "0xf, 0x0", 0, 2, "expected_ldt")
STR  = Instruction("str", "0xf, 0x0", 1, 2, "expected_tr")
SIDT = Instruction("sidt", "0xf, 0x1", 1, 6, "&expected_idt")
SGDT = Instruction("sgdt", "0xf, 0x1", 0, 6, "&expected_gdt")

CS = Segment("cs", "", "code")
DS = Segment("ds", "", "data")
SS = Segment("ss", "", "stack")
ES = Segment("es", "0x26", "data_es")
FS = Segment("fs", "0x64", "data_fs")
GS = Segment("gs", "0x65", "data_gs")

DATA_SEGS = [ DS, ES, FS, GS ]
INSTS = [SMSW, SLDT, STR, SGDT, SIDT ]

MO0 = [ BX_SI, BX_DI, BP_SI, BP_DI, SI, DI, BX ]
MO1 = [ BX_SI, BX_DI, BP_SI, BP_DI, SI, DI, BP, BX ]
MO2 = [ BX_SI, BX_DI, BP_SI, BP_DI, SI, DI, BP, BX ]


def two_comp_8(val):
	if (val > 0):
		return val
	else:
		return ((abs(val) ^ 0xff) + 1) & 0xff

def two_comp_16(val):
	if (val > 0):
		return val
	else:
		return ((abs(val) ^ 0xffff) + 1) & 0xffff

def my_hex(val):
	return hex(val).rstrip("L")

def get_segment_prefix(segment, register, modrm):
	""" default segments """
	if (segment.prefix == ""):
		segment_str = ""
		if (register == BX_SI):
			segment_chk_str = "data"
		elif (register == BX_DI):
			segment_chk_str = "data"
		elif (register == BP_SI):
			segment_chk_str = "stack"
		elif (register == BP_DI):
			segment_chk_str = "stack"
		elif (register == SI):
				segment_chk_str = "data"
		elif (register == DI):
				segment_chk_str = "data"
		elif (register == BP):
			# This is for a pure disp32, no register involved. Thus, default is data
			if ((modrm >> 6) == 0):
				segment_chk_str = "data"
			else:
				segment_chk_str = "stack"
		elif (register == BX):
			segment_chk_str = "data"
		else: # we should not fall here
			segment_chk_str = "data"
	else:
		segment_str = segment.prefix + ", "
		segment_chk_str = segment.array

	return segment_str, segment_chk_str

def generate_check_code(comment, segment_chk_str, address, inst, pass_ctr, fail_ctr):
	# TODO: Use an enum here
	comment += ". "
	if (inst.result_bytes == 2):
		checkcode = "\tgot = *(unsigned short *)(" + segment_chk_str +" + " + str(my_hex(address)) + ");\n"
		checkcode += "\tpr_result(got, expected, \"" + comment + "\", " + pass_ctr + ", " + fail_ctr + ");\n"
	elif (inst.result_bytes == 6):
		checkcode = "\tgot = (struct table_desc *)(" + segment_chk_str +" + " + str(my_hex(address)) + ");\n"
		checkcode += "\tpr_result_table(got, expected, \"" + comment + "\", " + pass_ctr + ", " + fail_ctr + ");\n"
	return checkcode

def generate_disp(modrm, disp):
	modrm_mod = modrm >> 6
	modrm_rm = modrm & 0x7
	if (modrm_mod == 0):
		# if r/m is 6 (same as BP), this is a disp16
		if (modrm_rm == 6):
			disp_2comp = two_comp_16(disp)
			disp_str = ", " + str(my_hex(disp_2comp & 0xff)) + ", "
			disp_str += str(my_hex((disp_2comp >> 8)  & 0xff))
		else:
			disp_str = ""
	elif (modrm_mod == 1):
		disp_2comp = two_comp_8(disp)
		disp_str = ", " + str(my_hex(disp))
	elif (modrm_mod == 2):
		disp_2comp = two_comp_16(disp)
		disp_str = ", " + str(my_hex(disp_2comp  & 0xff)) + ", "
		disp_str += str(my_hex((disp_2comp >> 8)  & 0xff))

	return disp_str

def generate_code(tc_nr, segment, inst, register, modrm_mod, index, disp):
	code_start = "\t\".byte "
	code_end = "\\n\\t\"\n"

	modrm = (modrm_mod << 6) | (inst.modrm_reg << 3) | register.modrm_rm
	modrm_str = ", " + str(my_hex(modrm))

	mov_reg_str = ""
	for m in register.mnemonic:
		val = two_comp_16(index/len(register.mnemonic))
		# we are just splitting the index between the number of registers indicated in modrm (max is 2)
		# this should not be a problem as we always expect the index to be an even number
		mov_reg_str += "\t\"mov $" + str(my_hex(val)) + ", " + m + "\\n\\t\"\n"

	segment_str, segment_chk_str = get_segment_prefix(segment, register, modrm)

	opcode_str = inst.opcode

	disp_str = generate_disp(modrm, disp)

	comment = "Test case " + str(tc_nr) + ": "
	comment += "SEG[" + segment_chk_str + "] "
	comment += "INSN: " + inst.name + "("
	if (len(register.name) == 2):
		comment += register.name[0] + " + " + register.name[1]
	else:
		comment += register.name[0]
	if (modrm_mod == 0):
		if ((modrm & 0x7) == 6):
			comment += " + disp32). "
		else:
			comment += "). "
	elif (modrm_mod == 1):
		comment += " + disp8). "
	elif (modrm_mod == 2):
		comment += " + disp16). "
	comment += "EFF_ADDR[" + str(my_hex(index + disp)) + "]. "
	for n in register.name:
		comment += n + "[" + str(my_hex(index/len(register.name))) + "] "
	if (modrm_mod == 0):
			comment += ""
	elif ((modrm_mod == 0) and ((modrm & 0x7) == 6)):
		comment += "disp16[" + str(my_hex(disp)) + "]"
	elif (modrm_mod == 1):
			comment += "disp8[" + str(my_hex(disp)) + "]"
	elif (modrm_mod == 2):
			comment += "disp16[" + str(my_hex(disp)) + "]"

	code = "\t/* " + comment + " */\n"
	code += mov_reg_str
	code += code_start + segment_str + opcode_str + modrm_str + disp_str + code_end

	checkcode = generate_check_code(comment, segment_chk_str, index + disp, inst, TEST_PASS_CTR_VAR, TEST_FAIL_CTR_VAR)

	return code, checkcode, inst.result_bytes

def generate_special_unit_tests(start_tc_nr, segment, inst, start_idx):
	code = ""
	checkcode = ""
	index = start_idx
	tc_nr = start_tc_nr
	code_start = "\t\".byte "
	code_end = "\\n\\t\"\n"
	opcode_str = inst.opcode

	# MOD = 0, r/m = 6 (BP), no index is used, only a disp16
	modrm = (MODRM_MO0 << 6) | (inst.modrm_reg << 3) | BP.modrm_rm
	modrm_str = ", " + str(my_hex(modrm))
	
	segment_str, segment_chk_str = get_segment_prefix(segment, BP, modrm)

	disp_str = generate_disp(modrm, index)
	
	comment = "Special Test case " + str(tc_nr) + ": "
	comment += "SEG[" + segment.name + "] "
	comment += "INSN: " + inst.name + " (disp32)."
	comment += "EFF_ADDR[" + str(my_hex(index)) + "]."
	comment += " disp32[" + str(my_hex(index)) + "]"

	code = "\t/* " + comment + " */\n"
	code += code_start + segment_str + opcode_str + modrm_str + disp_str + code_end

	checkcode += generate_check_code(comment, segment_chk_str, index, inst, TEST_PASS_CTR_VAR, TEST_FAIL_CTR_VAR)

	index += inst.result_bytes
	tc_nr += 1

	return code, checkcode, index, tc_nr

def generate_unit_tests(segment, inst, start_idx, start_tc_nr):
	code = ""
	check_code = ""
	index = start_idx
	testcase_nr = start_tc_nr

	code += "\t /* ==================== Test code for " + inst.name + " ==================== */\n"
	code += "\t\"test_umip_" + inst.name + "_" + segment.name + ":\\t\\n\"\n"

	check_code += "\n/* AUTOGENERATED CODE */\n"
	if (inst.result_bytes == 2):
		check_code += "static void check_tests_" + inst.name + "_" + segment.name + "(const unsigned short expected)\n"
		check_code += "{\n"
		check_code += "\tunsigned short got;\n\n"
	elif (inst.result_bytes == 6):
		check_code += "static void check_tests_" + inst.name + "_" + segment.name + "(const struct table_desc *expected)\n"
		check_code += "{\n"
		check_code += "\tstruct table_desc *got;\n\n"

	run_check_code = "\tcheck_tests_" + inst.name + "_" + segment.name + "(" + inst.expected_val + ");\n"
	check_code += "\tprintf(\"=======Results for " + inst.name + " in segment " + segment.name + "=============\\n\");\n"

	for reg in MO0:
		c, chk, idx = generate_code(testcase_nr, segment, inst, reg, MODRM_MO0, index, 0)
		code += c
		check_code += chk
		index += idx
		testcase_nr += 1

	# force a negative displacement
	start_addr = -100
	index += 100

	for reg in MO1:
		c, chk, idx = generate_code(testcase_nr, segment, inst, reg, MODRM_MO1, index, start_addr)
		code += c
		check_code += chk
		index += idx
		testcase_nr += 1

	# force a negative index
	start_addr += index + 100
	index = -100

	for reg in MO2:
		c, chk, idx = generate_code(testcase_nr, segment, inst, reg, MODRM_MO2, index, start_addr)
		code += c
		check_code += chk
		index += idx
		testcase_nr += 1

	start_addr += index
	
	c, chk, idx, tc_nr = generate_special_unit_tests(testcase_nr, segment, inst, start_addr)
	code += c
	check_code += chk
	testcase_nr = tc_nr
	start_addr = idx

	code += "\t\"test_umip_" + inst.name + "_" + segment.name + "_end:\\t\\n\"\n"
	check_code += "}\n\n"

	return code, check_code, run_check_code, start_addr, testcase_nr

def generate_tests_all_insts(seg, start_index, start_test_nr):
	run_check_code = ""
	test_code = ""
	check_code = ""
	index = start_index
	test_nr = start_test_nr

	for inst in INSTS:
		tc, chkc, hdr, index, test_nr = generate_unit_tests(seg, inst, index, test_nr)
		run_check_code += hdr
		test_code += tc
		check_code += chkc

	return test_code, check_code, run_check_code, index, test_nr

def generate_test_cases(test_code, check_code):
	index = 0

	header_info = "/* ******************** AUTOGENERATED CODE ******************** */\n"
	header_info += "#define SEGMENT_SIZE " + str(SEGMENT_SIZE) + "\n"
	header_info += "#define CODE_MEM_SIZE " + str(CODE_MEM_SIZE) + "\n"
	header_info += "\n"
	header_info += "unsigned char data[SEGMENT_SIZE];\n"
	header_info += "unsigned char data_es[SEGMENT_SIZE];\n"
	header_info += "unsigned char data_fs[SEGMENT_SIZE];\n"
	header_info += "unsigned char data_gs[SEGMENT_SIZE];\n"
	header_info += "unsigned char stack_32[SEGMENT_SIZE];\n"
	header_info += "unsigned char stack[SEGMENT_SIZE];\n"
	header_info += "\n"
	header_info += "void check_results(void);\n"
	header_info += "\n"

	check_code += "/* ******************** AUTOGENERATED CODE ******************** */\n"
	check_code += "#include <stdio.h>\n"
	check_code += "#include \"test_umip_ldt_16.h\"\n\n"
	check_code += "#include \"umip_test_defs.h\"\n\n"
	check_code += "\n"
	check_code +="int " + TEST_PASS_CTR_VAR + ";\n"
	check_code +="int " + TEST_FAIL_CTR_VAR + ";\n"
	check_code +="int " + TEST_ERROR_CTR_VAR + ";\n"
	check_code += "\n"

	run_check_code = ""

	test_code += "\tasm(\n"
	test_code += "\t /* ******************** AUTOGENERATED CODE ******************** */\n"
	test_code += "\t\".pushsection .rodata\\n\\t\"\n"
	test_code += "\t\"test_umip:\\n\\t\"\n"
	test_code += "\t\".code16\\n\\t\"\n"
	test_code += "\t/* setup stack */\n"
	test_code += "\t\"mov $" + str(SEGMENT_SIZE) + ", %sp\\n\\t\"\n"
	test_code += "\t\"mov %si, %ss\\n\\t\"\n"
	test_code += "\t/* save caller's cs */\n"
	test_code += "\t\"push %dx\\n\\t\"\n"
	test_code += "\t/* save caller's sp */\n"
	test_code += "\t\"push %ax\\n\\t\"\n"
	test_code += "\t/* save caller's ss */\n"
	test_code += "\t\"push %bx\\n\\t\"\n"

	test_nr = 0

	for seg in DATA_SEGS:
		index = 0
		tc, chkc, rchk, index, test_nr = generate_tests_all_insts(seg, index, test_nr)
		run_check_code += rchk
		test_code += tc
		check_code += chkc

	test_code += "\t/* preparing to return */\n"
	test_code += "\t/* restore caller's ss */\n"
	test_code += "\t\"pop %bx\\n\\t\"\n"
	test_code += "\t/* restore caller's sp */\n"
	test_code += "\t\"pop %ax\\n\\t\"\n"
	test_code += "\t/*\n"
	test_code += "\t * We only need the return IP, CS is already in stack\n"
	test_code += "\t * jump to interim_start, which is at the beginning of\n"
	test_code += "\t * this chunk of code\n"
	test_code += "\t */\n"
	test_code += "\t\"push $0\\n\\t\"\n"
	test_code += "\t\"retf\\n\\t\"\n"
	test_code += "\t\"test_umip_end:\\t\\n\"\n"
	test_code += "\t\".code32\\n\\t\"\n"
	test_code += "\t\".popsection\\n\\t\"\n"
	test_code += "\t);\n"

	check_code += "\n"
	check_code += "void check_results(void)\n"
	check_code += "{\n"
	check_code += run_check_code
	check_code += "}\n"
	check_code += "\n"

	return test_code, check_code, header_info, index

def write_test_files():
	check_code = ""
	test_code = "/* This is an autogenerated file. If you intend to debug, better to debug the generating script. */\n\n"
	test_code += "\n"
	test_code, check_code, check_code_hdr, global_index = generate_test_cases(test_code, check_code)

	fcheck_code = open("test_umip_ldt_16.c", "w")
	fheader = open("test_umip_ldt_16.h", "w")
	ftest_code = open("test_umip_code_16.h", "w")
	ftest_code.writelines(test_code)
	fheader.writelines(check_code_hdr)
	fcheck_code.writelines(check_code)
	ftest_code.close()
	fcheck_code.close()
	fheader.close()

write_test_files()
