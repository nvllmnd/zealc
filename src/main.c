#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ast/lex.h"
#include "constants.h"
#include "log.h"
#include "memory/cstr.h"

static constexpr const isize MAX_ARGV = 24;

struct CmdArgs {
  i32 argc;
  const char* exe_name;

  const char* argv[MAX_ARGV];
};
alias(CmdArgs);

CmdArgs parse_cmd_args(i32 argc, char** argv) {
  argc--;

  auto args = make(CmdArgs, .argc = argc, argv[0], .argv = {});

  argv++;

  if (argc > MAX_ARGV) {
    argc = MAX_ARGV;
  }

  memcpy(args.argv, argv, argc);
  return args;
}

static i32 repl(void);

i32 eval_string(const char* s) {
  LOG_DBG("eval_string(\"%s\")", s);
  return 0;  
}

i32 eval_file(const char* filepath) {

  LOG_DBG("eval_file(\"%s\")", filepath);
  return 0;    
}

int main(i32 argc, char** argv) {
  const CmdArgs args = parse_cmd_args(argc, argv);

  switch (args.argc) {
    case 0: {
      return repl();
    } break;
    case 1: {
      return eval_file(args.argv[0]);
    } break;
    case 2: {
      const char* a0 = args.argv[0];
      const char* a1 = args.argv[1];
      switch (stringlen(a0)) {
        case 1: {
          if (stringeq(a0, "e", 1)) {
            return eval_string(a1);
          } else {
            eprintln("Unknown argument: %s", a0);
            return 1;
          }
        } break;
        case 2: {
          if (stringeq(a0, "-e", 2)) {
            return eval_string(a1);
          } else {
            eprintln("Unknown argument: %s", a0);
            return 1;
          }
        } break;
        case 4: {
          if (stringeq(a0, "eval", 4)) {
            return eval_string(a1);
          }

          eprintln("Unknown argument: %s", a0);
          return 1;
        } break;
        default: {
          eprintln("Unknown argument: %s", a0);
          return 1;
        } break;
      }
    } break;
    default: {
    } break;
  }

  static constexpr const char LANG_VALID_INPUT[] = "let x = 155;\nfn do_thing(n: i32) i32 {\n    return n;\n}\n   ";
  static const sslice LANG_VALID_INPTU_SLICE = sslice_static_new(LANG_VALID_INPUT);

  LexState lex = {};
  lexer_init_source(&lex, sslice_static_new(LANG_VALID_INPUT));

  Token t = {};
  LexError err = LexError__Ok;

  SLOG_DBG(LANG_VALID_INPTU_SLICE);

  while (t.type != Token__Eof && err == LexError__Ok) {
    err = lexer_next(&lex, &t);
    SLOG_DBG(t.lexeme);
  }

  if (err != LexError__Ok) {
    LOG_DBG("LEX ERROR: %s", lex_error_string(err));
  }
}

i32 repl(void) {
  bool running = true;
  i32 hist_size = 0;

  char buf[KB1] = {};
  char hist[KB1][KB1] = {};

  while (running) {
    fflush(stdin);
    println();

    print("zeal> ");

    const i32 nbytes = read(STDIN_FILENO, buf, KB1);
    if (nbytes < 0) {
      eprintln("Failed to read from STDIN. ERRNO: %d", nbytes);
      return 1;
    } else if (nbytes == 0) {
      println("EOF read. Exiting zeal repl!");
      return 0;
    }

    if (stringeq(buf, "up", 2)) {
      print("%s", hist[--hist_size]); 
      continue;
    }

    if (stringeq(buf, ".exit", 5) || stringeq(buf, ".quit", 5)) {
      println("Exiting zeal repl!");
      return 0;
    }
    
    const i32 err = eval_string(buf);
    memcpy(hist[hist_size++],  buf, nbytes);
    memset(buf, 0, nbytes);
    if (hist_size >= KB1) {
      hist_size = 0;
    }
    if (err != 0) {
      eprintln("Eval Error! ERRNO: %d", err);
      continue;
    }

  }
  return 0;
}
