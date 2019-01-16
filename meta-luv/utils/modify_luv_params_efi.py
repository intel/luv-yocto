#!/usr/bin/python
#
# Copyright 2016 Intel Corporation; author Gayatri Kammela
#
# This script changes the parameters such as luv_netconsole and luv_storage
# for network debugging(netconsole) and sending the results to the
# webserver respectively, when booting LUV via netboot(luv-netboot-image.efi).
# To know the usage of this file do $ ./modify_luv_netboot_efi.py --help
#
#################################################################

import string
import argparse
import re
import os

start_string = "## Start of luv.cfg ##\n\n"
end_string = "\n## END of luv.cfg ##"
# List of parameters which cannot be modified by user #
do_not_modify = ["LUV_BITS_CAN_RUN"]

parser = argparse.ArgumentParser()

class LUVConfigParam(object):
    """ create objects to all optional, destination and help arguments """
    def __init__(self, short_option, long_option, dest_varname, num_args, choices, help_text):
        self.short_option = short_option
        self.long_option = long_option
        self.dest_varname = dest_varname
        self.num_args = num_args
        self.choices = choices
        self.help_text = help_text

"""
list of arguments for each and every parameter in luv.cfg file. If new parameter
is added to luv.cfg, making changes only in the luv_config_params should be sufficient.

Note: 'filename' is special as it is the only mandatory parameter
"""
luv_config_params = [LUVConfigParam("-f", "--filename", 'filename', None, None,
                     'Read from the file'),
                     LUVConfigParam("-n", "--netconsole", 'luv_netconsole', None, None,
                     'The format of the netconsole 10.11.12.13,64001'),
                     LUVConfigParam("-u", "--url", 'luv_storage_url', None, None,
                     'The format of url http://ipaddress/path/to/folder'),
                     LUVConfigParam("-d", "--disable_tests", 'luv_tests', '*',
                     ['bits', 'chipsec', 'fwts', 'ndctl', 'efivarfs-test', 'pstore-tests', 'kernel-efi-warnings'],
                     "exclude specific test suites while running LUV"),
                     LUVConfigParam("-r", "--reboot-tests", 'luv_reboot_tests', '*',
                     ['luv.pstore-tests'], "Reboot tests to run in LUV")]

def read_luv_cfg():
    """ Read the luv.cfg file from .EFI binary and store in a list/array"""
    f = open(args_lists['filename'], "rb")
    s = str(f.read())
    try:
        """ get the offset of the start and end lines of luv.cfg """
        start_index = s.index(start_string)
        end_index = s.index(end_string)
    except ValueError:
        print ("Oops! something went wrong, please use a fresh binary file")
        parser.exit()

    p = end_index - start_index
    f.close()
    f = open(args_lists['filename'], "rb")
    f.seek(start_index, 0)
    luv_cfg = f.read(p)
    luv_cfg_lines = luv_cfg.split('\n')

    return luv_cfg_lines, end_index, start_index

def replace_strings():
    new_rep_str = start_string
    for i in luv_cfg_lines:
        if re.search('^set', i):
            """
            keywords is a collection of all the 'key' parameters that user intends to
            change using this script
            example: consider a parameter 'set Name=LUV\n', the keyword here is
            'set Name'. Such keywords are extracted from parameters using split().

            split the line at "=" just once (1) and then retrieve the first
            element [0] as in get the string just before the "=" as the keyword and
            the second element[1] as in the string after the "=" as the keyval
            """
            keyword= i.split('=', 1)[0]
            keyval = i.split('=', 1)[1]
            for a in args_lists:
                if keyword[4:] == a.upper():
                    if (str(args_lists[a]).lower() != 'none'):
                        if keyword[4:] == "LUV_TESTS":
                            luv_tests = keyval
                            for d, p in enumerate(args.luv_tests):
                                if p not in luv_tests:
                                    print ("Test suite " + p + " is already disabled\n")
                                luv_tests = luv_tests.replace(args.luv_tests[d], '')
                            replace_str = keyword + "=" + luv_tests + "\n"
                        else:
                            replace_str = keyword + "=" + str(args_lists[a]).lower() + "\n"
                    else:
                        replace_str = keyword + "=" + keyval.lower() + "\n"
                    new_rep_str = new_rep_str + replace_str
            if keyword[4:] in do_not_modify:
                new_rep_str = new_rep_str + i + "\n"

    return new_rep_str

def write_to_luv_cfg(start_index, end_index):
    f = open(args_lists['filename'], "rb")
    s = str(f.read())
    new_rep_str = replace_strings()

    """ check if the length of the both lines is greater than the file size """
    if len(new_rep_str) > end_index - start_index:
        parser.error("Sorry can't replace the variables as they are too large!")
        print (".EFI binary can not be modified! Please checkout the "
               "documentation in meta-luv/README and provide valid parameters!")
    else:
        """
        if not, pad the file with x number of white spaces that are equal to the
        difference of both lines and the length of the file itself.
        """
        spaces = end_index - len(new_rep_str) - start_index
        x = ' '
        """
        Padding the file with white spaces is required as we are modifying a
        .EFI binary file. Failing to do so will result in unreadable binary.
        """
        white_spaces = x*spaces
        s = s[:start_index] +  new_rep_str + white_spaces + s[end_index:]
        f.close()
        f = open(args_lists['filename'], "wb")
        f.write(s)
        f.close()

""" Parse the arguments to the parser to populate all the options """
for obj in luv_config_params:
    parser.add_argument(obj.short_option, obj.long_option,
                        dest=obj.dest_varname, nargs=obj.num_args,
                        choices=obj.choices, help=obj.help_text)

args = parser.parse_args()
""" convert the namespace args in to a dictionary """
args_lists = vars(args)

""" Check if the filename is given as argument """
if not (args_lists['filename']):
    parser.error(" Please provide filename as an argument")
""" Check if the file exists and then run the script """
if os.path.exists(args_lists['filename']):
    luv_cfg_lines, end_index, start_index = read_luv_cfg()
    write_to_luv_cfg(start_index, end_index)
