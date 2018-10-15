#!/usr/bin/env python

import sys
import os
import re
import traceback
import subprocess

DAT_PATH = "../Release/dat/dat.exe"
DATA_PATH = "../data"
SOURCE_PATH = "../src/datafiles/"

def run_cmd(argv, contents=None):
    errContents = ""
    try:
        process = subprocess.Popen(argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output, errContents = process.communicate(contents)
        exit_code = process.wait()
        resultunicode = output.decode("utf-8", "replace")
        if not resultunicode:
            return None
        return resultunicode
    except:
        raise IOError("Error using dat\n%s\n'%s'"%(traceback.format_exc(),errContents))

def generate_datafile(datafile):
    datafile_path = os.path.join(DATA_PATH, datafile)
    header_path = os.path.join(SOURCE_PATH, "gen_" + os.path.splitext(datafile)[0] + ".h")
    source_path = os.path.join(SOURCE_PATH, "gen_" + os.path.splitext(datafile)[0] + ".c")

    pragma_once = os.path.splitext(datafile)[0].upper() + "_H"
    index_function_def = "int datafile_" + os.path.splitext(datafile)[0] + "_index(const char* str)"
    
    if os.path.exists(header_path):
        os.unlink(header_path)
    run_cmd([DAT_PATH, "-h", header_path, datafile_path])
    data = open(header_path, "rb").read()
    items = re.findall("#define\s+(\S+)\s+(\S+)", data)
    data = data.replace("/* Do not hand edit! */", "/* Do not hand edit! */\r\n\r\n#ifndef " + pragma_once + "\r\n#define " + pragma_once)
    data = data + "#ifdef __cplusplus\r\nextern \"C\" {\r\n#endif\r\n\r\n"
    data = data + index_function_def + ";\r\n\r\n"
    data = data + "#ifdef __cplusplus\r\n}\r\n#endif\r\n\r\n"
    data = data  + "#endif\r\n"
    open(header_path, "wb").write(data.replace("\r", ""))

    data = """ /* This is generated file, do not edit, you changes will be lost 

Run: %s script to update it
*/

#include <stdio.h>

""" % sys.argv[0]
    data = data + index_function_def + " {\r\n"
    for define, value in items:
        data = data + "\tif (strcmp(str, \"" + define + "\") == 0)\r\n\t\treturn " + value + ";\r\n"
    data = data + "\treturn -1;\r\n}\r\n"
    open(source_path, "wb").write(data.replace("\r", ""))

generate_datafile("victoryditty.dat")
