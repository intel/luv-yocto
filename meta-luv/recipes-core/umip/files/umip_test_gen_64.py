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
SEGMENT_SIZE = 1048576
CODE_MEM_SIZE = 1048576

class Instruction:
	def __init__(self, name, opcode, modrm_reg, result_bytes, expected_val):
		self.name = name
		self.opcode = opcode
		self.modrm_reg = modrm_reg
		self.result_bytes = result_bytes
		self.expected_val = expected_val


class Register:
	def __init__(self, mnemonic, modrm_rm, name, rex_x, rex_b):
		self.mnemonic = mnemonic
		self.modrm_rm = modrm_rm
		self.name = name
		self.rex_x = rex_x
		self.rex_b = rex_b

class Segment:
	def __init__(self, name, prefix, array):
		self.name = name
		self.prefix = prefix
		self.array = array


RAX = Register("%rax", 0, "rax", 0, 0)
RCX = Register("%rcx", 1, "rcx", 0, 0)
RDX = Register("%rdx", 2, "rdx", 0, 0)
RBX = Register("%rbx", 3, "rbx", 0, 0)
RSP = Register("%rsp", 4, "rsp", 0, 0)
RBP = Register("%rbp", 5, "rbp", 0, 0)
RSI = Register("%rsi", 6, "rsi", 0, 0)
RDI = Register("%rdi", 7, "rdi", 0, 0)
R8  = Register("%r8", 0, "r8", 0x42, 0x41)
R9  = Register("%r9", 1, "r9", 0x42, 0x41)
R10 = Register("%r10", 2, "r10", 0x42, 0x41)
R11 = Register("%r11", 3, "r11", 0x42, 0x41)
R12 = Register("%r12", 4, "r12", 0x42, 0x41)
R13 = Register("%r13", 5, "r13", 0x42, 0x41)
R14 = Register("%r14", 6, "r14", 0x42, 0x41)
R15 = Register("%r15", 7, "r15", 0x42, 0x41)


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

DATA_SEGS = [ FS, GS ]
INSTS = [SMSW, SLDT, STR, SGDT, SIDT ]

MO0 = [ RAX, RCX, RDX, RBX, RSI, RDI, R8, R9, R10, R11, R14, R15 ]
MO1 = [ RAX, RCX, RDX, RBX, RBP, RSI, RDI, R8, R9, R10, R11, R13, R14, R15 ]
MO2 = MO1

SIB_index = [ RAX, RCX, RDX, RBX, RBP, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15 ]
SIB_base_MO0 = [ RAX, RCX, RDX, RBX, RSI, RDI, R8, R9, R10, R11, R12, R14, R15 ]
SIB_base_MO1 = [ RAX, RCX, RDX, RBX, RBP, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15 ]
SIB_base_MO2 = SIB_base_MO1

def two_comp_32(val):
	if (val >= 0):
		return val
	else:
		return ((abs(val) ^ 0xffffffff) + 1) & 0xffffffff

def two_comp_64(val):
	if (val >= 0):
		return val
	else:
		return ((abs(val) ^ 0xffffffffffffffff) + 1) & 0xffffffffffffffff

def two_comp_8(val):
	if (val >= 0):
		return val
	else:
		return ((abs(val) ^ 0xff) + 1) & 0xff

def my_hex(val):
	hxval = hex(val)
	if (hxval[-1] == 'L'):
		return hxval[:-1]
	else:
		return hxval

def find_backup_reg(do_not_use):
	regs = [ RAX, RCX, RDX, RBX, RSP, RBP, RSI, RDI, R8, R9, R10, R11, R12, R13, R14, R15]
	for u in do_not_use:
		regs.remove(u)
	return regs[0]

def get_segment_prefix(segment, register, modrm, sib=0):
	""" default segments """
	if (segment.prefix == ""):
		segment_str = ""
		if ((register == RBP) or (register == RSP)):
			# This is for a pure disp32, no register involved. Thus, default is data
			if (((modrm >> 6) == 0) and (modrm & 0x7) == 5):
				segment_chk_str = "data"
			# If using a sib byte and the base is RBP, use DS as the base is ignored
			elif (((modrm >> 6) == 0) and ((modrm & 0x7) == 4) and ((sib & 0x7) == 5)):
				segment_chk_str = "data"
			else:
				segment_chk_str = "stack"
		else:
			segment_chk_str = "data"
	else:
		segment_str = segment.prefix + ", "
		segment_chk_str = segment.array

	return segment_str, segment_chk_str

def generate_check_code(comment, segment_chk_str, address, inst):
	# TODO: Use an enum here
	if (inst.result_bytes == 2):
		checkcode = "\tgot = *(unsigned short *)(" + segment_chk_str +" + " + str(hex(address)) + ");\n"
		checkcode += "\tprintf(\"%s " + comment + ". Got:[0x%x]Exp[0x%x]\\n\",\n"
		checkcode += "\t       got==expected ? TEST_PASS : TEST_FAIL, got, expected);\n"
	elif (inst.result_bytes == 6):
		checkcode = "\tgot = (struct table_desc *)(" + segment_chk_str +" + " + str(hex(address)) + ");\n"
		checkcode += "\tprintf(\"%s " + comment + ". Got:Base[0x%lx]Limit[0x%x]ExpBase[0x%lx]Limit[0x%x]\\n\",\n"
		checkcode += "\t       ((got->base == expected->base) && (got->limit == expected->limit)) ? TEST_PASS : TEST_FAIL, got->base, got->limit, expected->base, expected->limit);\n"
	return checkcode

def generate_disp(modrm, disp, sib=0):
	modrm_mod = modrm >> 6
	if (modrm_mod == 0):

		# if a sib byte is used, disp 32 is used
		if (((sib &7) == 5) and (modrm & 7) == 4):
			disp_2comp = two_comp_32(disp)
			disp_str = ", " + str(hex(disp_2comp & 0xff)) + ", "
			disp_str += str(hex((disp_2comp >> 8)  & 0xff)) + ", "
			disp_str += str(hex((disp_2comp >> 16) & 0xff)) + ", "
			disp_str += str(hex((disp_2comp >> 24) & 0xff))
		else:
			disp_str = ""
	elif (modrm_mod == 1):
		disp_2comp = two_comp_8(disp)
		disp_str = ", " + str(hex(disp_2comp))
	elif (modrm_mod == 2):
		disp_2comp = two_comp_32(disp)
		disp_str = ", " + str(hex(disp_2comp & 0xff)) + ", "
		disp_str += str(hex((disp_2comp >> 8)  & 0xff)) + ", "
		disp_str += str(hex((disp_2comp >> 16) & 0xff)) + ", "
		disp_str += str(hex((disp_2comp >> 24) & 0xff))

	return disp_str

def generate_code(tc_nr, segment, inst, register, modrm_mod, index, disp):
	code_start = "\t\".byte "
	code_end = "\\n\\t\"\n"

	modrm = (modrm_mod << 6) | (inst.modrm_reg << 3) | register.modrm_rm
	modrm_str = ", " + str(hex(modrm))
	mov_reg_str = "\t\"mov $" + str(my_hex(two_comp_64(index))) + ", " + register.mnemonic + "\\n\\t\"\n"

	segment_str, segment_chk_str = get_segment_prefix(segment, register, modrm)

	opcode_str = inst.opcode

	if (register.rex_b != 0):
		rex_b_str = hex(register.rex_b) + ", "
	else:
		rex_b_str = ""

	disp_str = generate_disp(modrm, disp)

	comment = "Test case " + str(tc_nr) + ": "
	comment += "SEG[" + segment_chk_str + "] "
	comment += "INSN: " + inst.name + "(" + register.name
	if (modrm_mod == 0):
		comment += "). "
	elif (modrm_mod == 1):
		comment += " + disp8). "
	elif (modrm_mod == 2):
		comment += " + disp32). "
	comment += "EFF_ADDR[" + str(hex(index + disp)) + "]. "
	comment += register.name + "[" + str(hex(index)) + "] "
	if (modrm_mod == 0):
			comment += ""
	elif (modrm_mod == 1):
			comment += "disp8[" + str(hex(disp)) + "]"
	elif (modrm_mod == 2):
			comment += "disp32[" + str(hex(disp)) + "]"

	code = "\t/* " + comment + " */\n"
	code += mov_reg_str
	code += code_start + segment_str + rex_b_str + opcode_str + modrm_str + disp_str + code_end

	checkcode = generate_check_code(comment, segment_chk_str, index + disp, inst)

	return code, checkcode, inst.result_bytes

def generate_code_sib(tc_nr, segment, inst, reg_base, reg_index, modrm_mod, sib_index, sib_base, sib_scale, disp, special=None):
	code_start = "\t\".byte "
	code_end = "\\n\\t\"\n"

	modrm = (modrm_mod << 6) | (inst.modrm_reg << 3) | RSP.modrm_rm
	sib = (sib_scale << 6) | (reg_index.modrm_rm << 3) | reg_base.modrm_rm
	modrm_str = ", " + str(hex(modrm))
	sib_str = ", " + str(hex(sib))
	mov_reg_str = "\t\"mov $" + str(my_hex(two_comp_64(sib_base))) + ", " + reg_base.mnemonic + "\\n\\t\"\n"
	mov_reg_str += "\t\"mov $" + str(my_hex(two_comp_64(sib_index))) + ", " + reg_index.mnemonic + "\\n\\t\"\n"

	if ((reg_index == RSP) or (reg_base == RSP)):
		backup_reg = find_backup_reg([reg_base, reg_index])
		backup_str = "\t\"mov " + RSP.mnemonic + ", " + backup_reg.mnemonic + "\\n\\t\"\n"
		restore_str = "\t\"mov " + backup_reg.mnemonic + ", " + RSP.mnemonic + "\\n\\t\"\n"
	else:
		backup_str = ""
		restore_str = ""

	segment_str, segment_chk_str = get_segment_prefix(segment, reg_base, modrm, sib)

	opcode_str = inst.opcode

	if ((reg_index.rex_x != 0) or (reg_base.rex_b != 0)):
		rex_b_str = hex(reg_index.rex_x | reg_base.rex_b) + ", "
	else:
		rex_b_str = ""

	disp_str = generate_disp(modrm, disp, sib)

	if (special != None):
		comment = special + " Test case " + str(tc_nr) + ": "
	else:
		comment = "Test case " + str(tc_nr) + ": "
	comment += "SEG[" + segment_chk_str + "] "
	comment += "INSN: " + inst.name + " SIB(b:" + reg_base.name + " i:" + reg_index.name + " s:" + str(sib_scale)
	if (modrm_mod == 0):
		comment += "). "
	elif (modrm_mod == 1):
		comment += " + disp8). "
	elif (modrm_mod == 2):
		comment += " + disp32). "

	# ignore index value when computing effective address
	calc_index = sib_index
	calc_base = sib_base
	if (reg_index == RSP):
		calc_index = 0
	# ignore base
	if ((reg_base == RBP) and (modrm_mod == 0)):
		calc_base = 0

	eff_addr = calc_base + calc_index*(1<<sib_scale) + disp

	comment += "EFF_ADDR[" + str(hex(eff_addr)) + "]. "
	comment += 	"b[" + str(hex(sib_base)) + "] i[" + str(hex(sib_index)) + "] "
	if (modrm_mod == 0):
		if(reg_base == RBP):
			comment += "disp32[" + str(hex(disp)) + "]"
		else:
			comment += ""
	elif (modrm_mod == 1):
			comment += "disp8[" + str(hex(disp)) + "]"
	elif (modrm_mod == 2):
			comment += "disp32[" + str(hex(disp)) + "]"

	code = "\t/* " + comment + " */\n"
	code += backup_str
	code += mov_reg_str
	code += code_start + segment_str + rex_b_str + opcode_str + modrm_str + sib_str + disp_str + code_end
	code += restore_str

	checkcode = generate_check_code(comment, segment_chk_str, eff_addr, inst)

	return code, checkcode, inst.result_bytes

def generate_unit_tests_sib(segment, inst, start_idx, start_tc_nr):
	index = start_idx
	testcase_nr = start_tc_nr
	code = ""
	check_code = ""

	for scale in range(0,4):
		for idx_reg in SIB_index:
			sib_base = index - 3*(1<<scale)
			sib_index = 3
			for base_reg in SIB_base_MO0:
				# while using the same register as base and index is not forbidden, it complicates our address calculation very greatly
				# hence, just skip this case
				if (base_reg == idx_reg):
					continue
				c, chk , i = generate_code_sib(testcase_nr, segment, inst, base_reg, idx_reg, MODRM_MO0, sib_index, sib_base, scale, 0)
				sib_base += i
				code += c
				check_code += chk
				testcase_nr += 1
			index = sib_base + sib_index*(1<<scale)

	for scale in range(0,4):
		for idx_reg in SIB_index:
			if (index > 127):
				disp = 0
			else:
				disp = index
				index = 0
			sib_base = index - 3*(1<<scale)
			sib_index = 3
			for base_reg in SIB_base_MO1:
				# while using the same register as base and index is not forbidden, it complicates our address calculation very greatly
				# hence, just skip this case
				if (base_reg == idx_reg):
					continue
				c, chk, i = generate_code_sib(testcase_nr, segment, inst, base_reg, idx_reg, MODRM_MO1, sib_index, sib_base, scale, disp)
				disp += i
				code += c
				check_code += chk
				testcase_nr += 1
			index = sib_base + sib_index*(1<<scale) + disp


	# the four scale values for MODRM_MOD=2 are split in several loops to
	# tests for various negative values
	# start here with negative indexes
	scale = 0
	disp = 0
	for idx_reg in SIB_index:
		sib_base = index - 3*(1<<scale)
		sib_index = 3

		# force a negative index
		sib_base += 100*(1<<scale)
		sib_index -= 100*(1<<scale)
		for base_reg in SIB_base_MO2:
			# while using the same register as base and index is not forbidden, it complicates our address calculation very greatly
			# hence, just skip this case
			if (base_reg == idx_reg):
				continue
			c, chk, i = generate_code_sib(testcase_nr, segment, inst, base_reg, idx_reg, MODRM_MO2, sib_index, sib_base, scale, disp)
			disp += i
			code += c
			check_code += chk
			testcase_nr += 1
		index = sib_base + sib_index*(1<<scale) + disp

	for scale in range(0,2):
		# use a negative displacement
		disp = -200
		for idx_reg in SIB_index:
			# offset here for the negative value of disp
			sib_base = index - 3*(1<<scale) + 200
			sib_index = 3
			for base_reg in SIB_base_MO2:
				# while using the same register as base and index is not forbidden, it complicates our address calculation very greatly
				# hence, just skip this case
				if (base_reg == idx_reg):
					continue
				c, chk, i = generate_code_sib(testcase_nr, segment, inst, base_reg, idx_reg, MODRM_MO2, sib_index, sib_base, scale, disp)
				disp += i
				code += c
				check_code += chk
				testcase_nr += 1
			index = sib_base + sib_index*(3<<scale) + disp

	for scale in range(3,4):
		# prepare use a negative base
		disp = index
		for idx_reg in SIB_index:
			sib_base = -3*(1<<scale)
			sib_index = 3
			for base_reg in SIB_base_MO2:
				# while using the same register as base and index is not forbidden, it complicates our address calculation very greatly
				# hence, just skip this case
				if (base_reg == idx_reg):
					continue
				c, chk, i = generate_code_sib(testcase_nr, segment, inst, base_reg, idx_reg, MODRM_MO2, sib_index, sib_base, scale, disp)
				disp += i
				code += c
				check_code += chk
				testcase_nr += 1
			index = sib_base + sib_index*(3<<scale) + disp

	return code, check_code, index, testcase_nr

def generate_special_unit_tests(start_tc_nr, segment, inst, start_idx):
	code = ""
	checkcode = ""
	index = start_idx
	tc_nr = start_tc_nr
	code_start = "\t\".byte "
	code_end = "\\n\\t\"\n"
	opcode_str = inst.opcode

	if(0):
		# MOD = 0, r/m = 5 (RBP), no index is used, only a disp32
		modrm = (MODRM_MO0 << 6) | (inst.modrm_reg << 3) | RBP.modrm_rm
		modrm_str = ", " + str(hex(modrm))

		segment_str, segment_chk_str = get_segment_prefix(segment, RBP, modrm)

		disp_str = ", " + str(hex(index & 0xff)) + ", "
		disp_str += str(hex((index >> 8)  & 0xff)) + ", "
		disp_str += str(hex((index >> 16) & 0xff)) + ", "
		disp_str += str(hex((index >> 24) & 0xff))

		comment = "Ricardin Special Test case " + str(tc_nr) + ": "
		comment += "SEG[" + segment.name + "] "
		comment += "INSN: " + inst.name + " (disp32)."
		comment += "EFF_ADDR[" + str(hex(index)) + "]."
		comment += " disp32[" + str(hex(index)) + "]"

		code = "\t/* " + comment + " */\n"
		code += "\t\"jmp 1f:\\n\\t\"\n"
		code += "\t\"1f:\\n\\t\"\n"
		code += code_start + segment_str + opcode_str + modrm_str + disp_str +code_end

		checkcode += generate_check_code(comment, segment_chk_str, index, inst)

		index += inst.result_bytes
		tc_nr += 1

	# With SIB, index is ignored if such index is RSP. This also means
	# that the scale is ignored
	for reg_base in SIB_base_MO0:
		c, chk , i = generate_code_sib(tc_nr, segment, inst, reg_base, RSP, MODRM_MO0, 0xffff, index, 3, 0, "Special")
		index += i
		code += c
		checkcode += chk
		tc_nr += 1

	disp = 0
	for reg_base in SIB_base_MO1:
		c, chk , i = generate_code_sib(tc_nr, segment, inst, reg_base, RSP, MODRM_MO1, 0xffff, index, 3, disp, "Special")
		disp += i
		code += c
		checkcode += chk
		tc_nr += 1

	index = index + disp
	disp = 0

	for reg_base in SIB_base_MO2:
		c, chk , i = generate_code_sib(tc_nr, segment, inst, reg_base, RSP, MODRM_MO2, 0xfff, index, 3, disp, "Special")
		disp += i
		code += c
		checkcode += chk
		tc_nr += 1

	index += disp

	# with SIB and base register as RBP and mod = 0, the base register is ignored and a disp32 is used. The default register is DS
	disp = 0
	for sib_scale in range (0,4):
		new_index = (index >> sib_scale) << sib_scale
		disp = index - new_index
		new_index >>= sib_scale
		for sib_index in SIB_base_MO0:
			c, chk, i = generate_code_sib(tc_nr, segment, inst, RBP, sib_index, MODRM_MO0, new_index, 0xeeee, sib_scale, disp, "Special")
			code += c
			disp += i
			checkcode += chk
			tc_nr += 1

	index += disp

	# with SIB, base as RBP and base as RSP, only disp32 is considered in the calculation
	# in 64-bit, the disp is added to RIP
	c, chk, i = generate_code_sib(tc_nr, segment, inst, RBP, RSP, MODRM_MO0, 0xbbbb, 0xcccc, 3, index, "Special")
	code += c
	disp += i
	checkcode += chk
	tc_nr += 1

	index += i

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

	# in signed 8-bit displacements, the max value is 255. Thus, correct, by adding
	# the remainder to index
	start_addr = index
	if (start_addr > 127):
		remainder = start_addr -127
		index = remainder
		start_addr = 127
	else:
		index = 0
 
	for reg in MO1:
		c, chk, idx = generate_code(testcase_nr, segment, inst, reg, MODRM_MO1, index, start_addr)
		code += c
		check_code += chk
		index += idx
		testcase_nr += 1
 
	start_addr += index
	index = 0
 
	# Force some indexes to be negative
	start_addr += 127
	index -=127
 
	for reg in MO2:
		c, chk, idx = generate_code(testcase_nr, segment, inst, reg, MODRM_MO2, index, start_addr)
		code += c
		check_code += chk
		index += idx
		testcase_nr += 1

	start_addr += index
 
	c, chk, start_addr, testcase_nr = generate_unit_tests_sib(segment, inst, start_addr, testcase_nr)
	code += c
	check_code += chk
 
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
	header_info = "/* ******************** AUTOGENERATED CODE ******************** */\n"
	header_info += "#define SEGMENT_SIZE " + str(SEGMENT_SIZE) + "\n"
	header_info += "#define CODE_MEM_SIZE " + str(CODE_MEM_SIZE) + "\n"
	header_info += "\n"
	header_info += "unsigned char data_fs[SEGMENT_SIZE];\n"
	header_info += "unsigned char data_gs[SEGMENT_SIZE];\n"
	header_info += "\n"
	header_info += "void check_results(void);\n"
	header_info += "\n"

	check_code += "/* ******************** AUTOGENERATED CODE ******************** */\n"
	check_code += "#include <stdio.h>\n"
	check_code += "#include \"test_umip_ldt_64.h\"\n\n"
	check_code += "#include \"umip_test_defs.h\"\n\n"
	check_code += "\n"

	run_check_code = ""

	test_code += "\tasm(\n"
	test_code += "\t /* ******************** AUTOGENERATED CODE ******************** */\n"
	test_code += "\t\".pushsection .rodata\\n\\t\"\n"
	test_code += "\t\"test_umip:\\t\\n\"\n"


	test_nr = 0

	index = 0
	for seg in DATA_SEGS:
		index = 0
		tc, chkc, rchk, index, test_nr = generate_tests_all_insts(seg, index, test_nr)
		run_check_code += rchk
		test_code += tc
		check_code += chkc

	#test_code += "\t\"jmp $finish_testing\\n\\t\"\n"
	test_code += "\t\"ret\\n\\t\"\n"
	test_code += "\t\"test_umip_end:\\t\\n\"\n"
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

	fcheck_code = open("test_umip_ldt_64.c", "w")
	fheader = open("test_umip_ldt_64.h", "w")
	ftest_code = open("test_umip_code_64.h", "w")
	ftest_code.writelines(test_code)
	fheader.writelines(check_code_hdr)
	fcheck_code.writelines(check_code)
	ftest_code.close()
	fcheck_code.close()
	fheader.close()

write_test_files()
