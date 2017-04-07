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

parser = argparse.ArgumentParser()
start_string = "## Start of luv.cfg ##\n\n"
end_string = "\n## END of luv.cfg ##"

class LUVConfigParam(object):
    """ create objects to all optional, destination and help arguments """
    def __init__(self,short_option, long_option, dest_varname, help_text):
        self.short_option = short_option
        self.long_option = long_option
        self.dest_varname = dest_varname
        self.help_text =help_text

"""
list of arguments for each and every parameter in luv.cfg
file. If new parameter is added to luv.cfg, making changes only in the
luv_config_params should be sufficient.

Note: 'filename' is special as it is the only manadatory parameter
"""
luv_config_params = [LUVConfigParam("-f", "--file", 'filename',
                     'Read from the file'),
                     LUVConfigParam("-n", "--netconsole", 'luv_netconsole',
                     'The format of the netconsole 10.11.12.13,64001'),
                     LUVConfigParam("-u", "--url", 'luv_storage_url',
                     'The format of url http://ipaddress/path/to/folder'),
                     LUVConfigParam("-v", "--verbose", 'verbose',
                     'press -v for help')]

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
    """
    keywords is a collection of all the 'key' parameters that user intend to
    change using this script
    example: consider a parameter 'set Name=LUV\n', the keyword here is
    'set Name'. Such keywords are extracted from parameters using split().

    split the line at "=" just once (1) and then retrieve the first
    element [0] as in get the string just before the "="
    """
    keywords = []
    for i in luv_cfg_lines:
        if re.search('^set', i):
            keyword = i.split('=', 1)[0]
            keywords.append(keyword)

    return keywords, end_index, start_index

def replace_strings(keywords):
    """
    define a new string that will replace the concatenation of strings
    after we modify the parameters with given values
    """
    new_rep_str = start_string
    """
    Match for the substrings in luv.cfg with the parameters that are
    being passed to parser.

    lower() is used to find the match for case insensitive strings and when
    a substring is found matched it returns the offset which is always greater
    than 0
    """
    for k in keywords:
        for a in args_lists:
            if (k.lower().find(a.lower()) > 0):
                """
                check if the arguments are passed for each parameter in luv.cfg.
                If not then replace that parameter with a string "none"
                """
                if (args_lists[a]) is None:
                    replace_str = k + "=" + "none" + "\n"
                    print ("No " + a + " parameter found! Please provide one "
                           "if you intend to use the feature")
                    new_rep_str = new_rep_str + replace_str
                    print ("Modifying .EFI binary without setting the " + a +
                           " parameter!. If you may want this feature, provide "
                           "one and rerun the script")
                else:
                    replace_str = k + "=" + str(args_lists[a])+"\n"
                    print (k + " to " + str(args_lists[a]))
                    new_rep_str = new_rep_str + replace_str

    return new_rep_str

def write_to_luv_cfg(start_index, end_index):
    f = open(args_lists['filename'], "rb")
    s = str(f.read())
    new_rep_str = replace_strings(keywords)
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
                        dest=obj.dest_varname, help=obj.help_text)

args = parser.parse_args()
""" convert the namespace args in to a dictionary """
args_lists = vars(args)

""" Check if the filename is given as argument """
if not (args_lists['filename']):
    parser.error(" Please provide filename as an argument")
""" Check if the file exists and then run the script """
if os.path.exists(args_lists['filename']):
    keywords, end_index, start_index = read_luv_cfg()
    write_to_luv_cfg(start_index, end_index)
