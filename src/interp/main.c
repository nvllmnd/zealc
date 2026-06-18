#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef NDEBUG
#define LIBNV_DEBUG 0
#ifndef ZEAL_DEBUG
#define ZEAL_DEBUG 0
#endif
#else
#define LIBNV_DEBUG 1
#ifndef ZEAL_DEBUG
#define ZEAL_DEBUG 1
#endif
#endif

#include "nv/core/constants.h"
#include "nv/core/debug.h"
#include "nv/core/log.h"

typedef struct option Opt;

static Opt options[] = {
    {.name = "repl", .has_arg = no_argument, .flag = nullptr, .val = 'r'},
    {.name = "compile", .has_arg = required_argument, .flag = nullptr, .val = 'c'},
    {.name = "eval", .has_arg = required_argument, .flag = nullptr, .val = 'e'},
    {.name = "output", .has_arg = required_argument, .flag = nullptr, .val = 'o'},
    {.name = "load", .has_arg = required_argument, .flag = nullptr, .val = 'l'},
    {.name = "interp", .has_arg = required_argument, .flag = nullptr, .val = 'i'},
    {.name = "version", .has_arg = no_argument, .flag = nullptr, .val = 'v'},
    {},
};

i32 main(i32 argc, char* argv[argc]) {
  char c = 0;
  i32 opt_index = 0;

  while ((c = getopt_long(argc, argv, "vrc:e:o:l:i:", options, &opt_index)) != -1) {
    const i32 curr_optid = optind ? optind : 1;
    switch (c) {
      case 0: {
        print("Option: %s ", options[opt_index].name);
        if (optarg) {
          println("With arg: %s", optarg);
        }
      } break;
      case 'r': {
        println("option repl");
      } break;
      case 'c': {
        println("option compile with value: %s", optarg);
      } break;
      case 'e': {
        println("option eval with value: %s", optarg);
      } break;
      case 'o': {
        println("option out with value: %s", optarg);
      } break;
      case 'l': {
        println("option load with value: %s", optarg);
      } break;
      case 'i': {
        println("option interp with value: %s", optarg);
      } break;
      case 'v': {
        println("option version");
      } break;
      case '?': {
          LOG_ERROR("Unknown option %d", curr_optid);
      } break;
      default: {
        LOG_ERROR("?? getopt returned character code; 0%o ??", c);
      } break;
    }
  }
}
