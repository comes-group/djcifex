import argparse

def truefalse(s):
   if s == "true": return True
   else: return False

ap = argparse.ArgumentParser(description="Generates strconsts for the DJ Cifex decoder")
ap.add_argument("in_file_name")
ap.add_argument("out_file_name")
ap.add_argument("--bytewise", type=truefalse)
ap.add_argument("--endianness", type=str, choices=["little", "big"])
args = ap.parse_args()

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
   if args.bytewise:
      n_bytes = 8
      for i in range(0, len(string), n_bytes):
         chunk = string[i : (i + n_bytes)]
         chunk_iter = chunk if args.endianness == "big" else reversed(chunk)
         hex_bytes = "".join([f"{x:x}" for x in chunk_iter])
         mask = "0x" + "FF" * len(chunk)
         if args.endianness == "big":
            zeroes = "00" * (n_bytes - len(chunk))
            mask += zeroes
            hex_bytes += zeroes
         hex_bytes = "0x" + hex_bytes
         conds.append(f"((wide_input[{i // n_bytes}] & {mask}) == {hex_bytes})")
   else:
      for i in range(0, len(string)):
         c = string[i]
         conds.append(f"(input[{i}] == {repr(c)})")
   cond = " && ".join(conds)

   result += f"""static cx_inline bool cx_sc_{ident}_match(const uint8_t *input) {{
      const uint64_t *wide_input = (const uint64_t *)input;
      return {cond};
   }}
"""
   result += f"static size_t cx_sc_{ident}_len = {len(string)};\n"
   return result

in_file_name = args.in_file_name
out_file_name = args.out_file_name

with open(in_file_name, "r") as in_file:
   for line in in_file.read().splitlines():
      pair = line.split(' ')
      generated += generate_code_for_strconst(pair[0], pair[1])
      generated += "\n"

with open(out_file_name, "w") as out_file:
   out_file.write(generated)
