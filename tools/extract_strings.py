#!/usr/bin/env python3
# SuperTux
# Copyright (C) 2018 Tobias Markus <tobbi.bugs@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys
import re

pattern = re.compile(r"(translate|_)\((.*?)\)")

lisp_template_begin = """(msgids
""";
lisp_template_end = ")"

lisp_content = lisp_template_begin

if(len(sys.argv) < 3):
	print("Usage: " + sys.argv[0] + " <filename> <output file>")
else:
	filename = sys.argv[1]
	out_file = sys.argv[2]
	strings_found = False
	with open(filename, 'r') as f:
		s = f.read()
	matches = pattern.findall(s)
	if len(matches) > 0:
		strings_found = True

	for match in matches:
		lines = match[1].split("\\n")
		num_lines = len(lines)
		if num_lines == 1:
			lisp_content += "  (msgid "
			lisp_content += "(_ " + match[1] + ")"
			lisp_content += ")\r\n"
		else:
			lisp_content += "  (msgid (_ "
			for i, line in enumerate(lines):
				lisp_content += line
				if i < num_lines:
					if i == num_lines - 1:
						lisp_content += "))"
					lisp_content += "\n"

	lisp_content += lisp_template_end

	if strings_found:
		output_file = open(out_file, "w")
		output_file.write(lisp_content)
		output_file.close()
