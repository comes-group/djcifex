import sys

generated = """
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "cxcompilers.h"

"""

def generate_code_for_strconst(ident, string):
   result = f"/* ident={ident} string='{string}' */\n"

   string = bytes(string, "UTF-8")
   conds = []
   for i in range(0, len(string)):
      c = string[i]
      conds.append(f"(input[{i}] == {repr(c)})")
   cond = ' && '.join(conds)

   result += f"static cx_inline bool cx_sc_{ident}_match(const uint8_t *input) {{ return {cond}; }}\n"
   result += f"static size_t cx_sc_{ident}_len = {len(string)};\n"
   return result

in_file_name = sys.argv[1]
out_file_name = sys.argv[2]

with open(in_file_name, "r") as in_file:
   for line in in_file.read().splitlines():
      pair = line.split(' ')
      generated += generate_code_for_strconst(pair[0], pair[1])
      generated += "\n"

with open(out_file_name, "w") as out_file:
   out_file.write(generated)
