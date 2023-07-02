#! /bin/bash
set -eu

# find ../src/barretenberg -type f -iname "CMakeLists.txt" -exec grep "barretenberg_module" {} + | awk -F " " '{print $2}' >> bb_modules.graph
find ../src/barretenberg -type f -iname "CMakeLists.txt" -exec grep "barretenberg_module" {} + > bb_modules

python << END
import re


with open('bb_modules_unreduced.graph', 'w') as outfile:
    outfile.write("digraph G {\n")
    sorted_lines = []
    with open('bb_modules', 'r') as infile:
        for line in infile:
            line = re.findall(r'barretenberg_module\(([^]]*)\)', line)[0]
            line = line.replace("-", "_")
            sorted_lines.append(line)

        sorted_lines.sort()
        for line in sorted_lines:
            deps = line.split(" ")
            module = deps[0]
            deps = deps[1:]
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