#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A command-line argument parser.
typedef struct cxc_arg_parser
{
   // I don't usually use `int`s but that's what crt does so whatever.
   int argc;
   char **argv;
   size_t *argv_lens;

   int position;
   size_t positional_arg;
   bool parse_options;
   bool success;
} cxc_arg_parser_t;

static cxc_arg_parser_t
cxc_init_arg_parser(int argc, char *argv[])
{
   cxc_arg_parser_t ap = {
      .argc = argc,
      .argv = argv,
      .position = 1,
      .positional_arg = 0,
      .parse_options = true,
   };

   size_t *lens = calloc(ap.argc, sizeof(size_t));
   for (int i = 0; i < ap.argc; ++i) {
      lens[i] = strlen(ap.argv[i]);
   }
   ap.argv_lens = lens;

   return ap;
}

static void
cxc_free_arg_parser(cxc_arg_parser_t *ap)
{
   free(ap->argv_lens);
   ap->argv_lens = NULL;
}

// Call this as a `while` condition to parse arguments.
static bool
cxc_has_args(const cxc_arg_parser_t *ap)
{
   return ap->position < ap->argc;
}

static bool
cxc_is_short_option(const cxc_arg_parser_t *ap, size_t index)
{
   char *str = ap->argv[index];
   size_t len = ap->argv_lens[index];
   return ap->parse_options && (len == 2 && str[0] == '-');
}

static bool
cxc_is_long_option(const cxc_arg_parser_t *ap, size_t index)
{
   char *str = ap->argv[index];
   size_t len = ap->argv_lens[index];
   return ap->parse_options && (len > 2 && str[0] == '-' && str[1] == '-');
}

static bool
cxc_is_option(const cxc_arg_parser_t *ap, size_t index)
{
   return cxc_is_short_option(ap, index) || cxc_is_long_option(ap, index);
}

static void
cxc_positional_args(cxc_arg_parser_t *ap, size_t n_positional, char ***out_strings)
{
   if (ap->success)
      return;
   if (!cxc_is_option(ap, ap->position)) {
      *(out_strings[ap->positional_arg]) = ap->argv[ap->position];
      ++ap->position;
      ++ap->positional_arg;
      ap->success = true;
   }
}

typedef enum cxc_arg_type
{
   cxc_bool,
} cxc_arg_type_t;

static void
cxc_named_arg(
   cxc_arg_parser_t *ap,
   char short_name,
   const char *long_name,
   cxc_arg_type_t type,
   void *out_value)
{
   if (ap->success)
      return;
   if (
      short_name != 0 && cxc_is_short_option(ap, ap->position) &&
      ap->argv[ap->position][1] == short_name) {
      switch (type) {
         case cxc_bool:
            *((bool *)out_value) = true;
            goto success;
      }
   }
   if (
      long_name != NULL && cxc_is_long_option(ap, ap->position) &&
      strcmp(&ap->argv[ap->position][2], long_name) == 0) {
      switch (type) {
         case cxc_bool:
            *((bool *)out_value) = true;
            goto success;
      }
   }
   return;

success:
   ap->success = true;
   ++ap->position;
   return;
}

static void
cxc_finish_arg(cxc_arg_parser_t *ap)
{
   if (!ap->success) {
      fprintf(stderr, "error: unknown option %s\n", ap->argv[ap->position]);
      exit(-1);
   }
   ap->success = false;
}
