#!/usr/bin/env python

import sys
import os
import re

s = """ /* This is generated file, do not edit, you changes will be lost 

Run: %s script to update it
*/

#include "other/gameconf.h"
#include "ship.h"

void register_ships()
{
""" % sys.argv[0]

files = [ os.path.join("../src/ships", x) for x in os.listdir("../src/ships") if x[-3:] == "cpp" and x[:3] == 'shp']


for name in files:
	f = open(name, "r")
	for line in f.readlines():
		m = re.match(r'REGISTER_SHIP\((.*)\)', line)
		if m:
			s = s + "\t{\n\t\tvoid __register_shipclass_%s();\n\t\t__register_shipclass_%s();\n\t}\n" % (m.group(1), m.group(1))
	f.close()

s = s + "\n\n"

for name in files:
	s = s + "\tregister_shiptype(data_full_path(\"%s\").c_str());\n" % os.path.basename(name).replace(".cpp", ".ini")


s = s + "}\n"



f = open("../src/ships/gen_ships.cpp", "w+b")
f.write(s)
f.close()

