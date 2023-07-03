#! /bin/bash
set -eu

# This script outputs two graphviz files, one describing the raw Barretenberg module dependencies, and one 
# giving a minimal dependency graph. The minimal dependency graph is derived using transitive reduction,
# which is implemented by the program called tred.
# A graph file can be rendered into a pdf as in
#   `dot -Tpdf foo.graph > foo.pdf`

find ../src/barretenberg -type f -iname "CMakeLists.txt" -exec grep "barretenberg_module" {} + > bb_modules

python << END
import re


with open('bb_modules_unreduced.graph', 'w') as outfile:
    outfile.write("digraph G {\n")
    lines = []
    with open('bb_modules', 'r') as infile:
        for line in infile:
            line = re.findall(r'barretenberg_module\(([^]]*)\)', line)[0]
            line = line.replace("-", "_")
            lines.append(line)

        lines.sort()
        for line in lines:
            deps = line.split(" ")
            module = deps[0]
            deps = deps[1:]
            deps.sort()
            for dep in deps:
                outfile.write("        " + module + " -> " + dep + ";\n")
    outfile.write("}\n")
END

rm bb_modules
tred bb_modules_unreduced.graph > bb_modules_reduced_not_sorted.graph

python << END
with open('bb_modules_reduced.graph', 'w') as outfile:
    outfile.write("digraph G {\n")
    sorted_lines = []
    with open('bb_modules_reduced_not_sorted.graph', 'rw') as infile:
        for line in infile:
            if line[0] == '\t':
                sorted_lines.append(line)
        sorted_lines.sort()
    for line in sorted_lines:
        outfile.write(line)
    outfile.write("}\n")
END

rm bb_modules_reduced_not_sorted.graph

diff -c bb_modules_unreduced.graph bb_modules_reduced.graph