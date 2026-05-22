/* ANSI-C code produced by gperf version 3.3 */
/* Command-line: gperf --struct-type --omit-struct-type --global-table --slot-name=id --compare-strncmp --multiple-iterations=100 --switch=1 --readonly-tables --enum --pic --random --includes --lookup-function-name=kw_lookup_str --output-file=./src/ast/keywords.c ./build/keywords.gperf  */
/* Computed positions: -k'1,3' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

#line 1 "./build/keywords.gperf"

	#include "ast/token.h"
	#include "core/runes.h"
	#include <assert.h>
#include <string.h>
enum
  {
    TOTAL_KEYWORDS = 38,
    MIN_WORD_LENGTH = 2,
    MAX_WORD_LENGTH = 8,
    MIN_HASH_VALUE = 2,
    MAX_HASH_VALUE = 49
  };

/* maximum key range = 48, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (register const char *str, register size_t len)
{
  static const unsigned char asso_values[] =
    {
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 33,  9,  3,
      15,  6,  0, 27, 50,  6, 24, 50,  9,  0,
      27, 21,  6, 15,  0,  0,  3, 21,  0, 12,
      50, 50, 50, 12, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
      50, 50, 50, 50, 50, 50, 50
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[2]+1];
#if (defined __cplusplus && (__cplusplus >= 201703L || (__cplusplus >= 201103L && defined __clang__ && __clang_major__ + (__clang_minor__ >= 9) > 3))) || (defined __STDC_VERSION__ && __STDC_VERSION__ >= 202000L && ((defined __GNUC__ && __GNUC__ >= 10) || (defined __clang__ && __clang_major__ >= 9)))
      [[fallthrough]];
#elif (defined __GNUC__ && __GNUC__ >= 7) || (defined __clang__ && __clang_major__ >= 10)
      __attribute__ ((__fallthrough__));
#endif
      /*FALLTHROUGH*/
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

struct stringpool_t
  {
    char stringpool_str0[sizeof("fn")];
    char stringpool_str1[sizeof("for")];
    char stringpool_str2[sizeof("self")];
    char stringpool_str3[sizeof("false")];
    char stringpool_str4[sizeof("struct")];
    char stringpool_str5[sizeof("true")];
    char stringpool_str6[sizeof("if")];
    char stringpool_str7[sizeof("mod")];
    char stringpool_str8[sizeof("enum")];
    char stringpool_str9[sizeof("error")];
    char stringpool_str10[sizeof("pub")];
    char stringpool_str11[sizeof("else")];
    char stringpool_str12[sizeof("break")];
    char stringpool_str13[sizeof("static")];
    char stringpool_str14[sizeof("when")];
    char stringpool_str15[sizeof("trait")];
    char stringpool_str16[sizeof("sizeof")];
    char stringpool_str17[sizeof("loop")];
    char stringpool_str18[sizeof("macro")];
    char stringpool_str19[sizeof("derive")];
    char stringpool_str20[sizeof("type")];
    char stringpool_str21[sizeof("or")];
    char stringpool_str22[sizeof("mut")];
    char stringpool_str23[sizeof("impl")];
    char stringpool_str24[sizeof("match")];
    char stringpool_str25[sizeof("return")];
    char stringpool_str26[sizeof("const")];
    char stringpool_str27[sizeof("ref")];
    char stringpool_str28[sizeof("continue")];
    char stringpool_str29[sizeof("let")];
    char stringpool_str30[sizeof("print")];
    char stringpool_str31[sizeof("println")];
    char stringpool_str32[sizeof("comptime")];
    char stringpool_str33[sizeof("dyn")];
    char stringpool_str34[sizeof("while")];
    char stringpool_str35[sizeof("and")];
    char stringpool_str36[sizeof("await")];
    char stringpool_str37[sizeof("default")];
  };
static const struct stringpool_t stringpool_contents =
  {
    "fn",
    "for",
    "self",
    "false",
    "struct",
    "true",
    "if",
    "mod",
    "enum",
    "error",
    "pub",
    "else",
    "break",
    "static",
    "when",
    "trait",
    "sizeof",
    "loop",
    "macro",
    "derive",
    "type",
    "or",
    "mut",
    "impl",
    "match",
    "return",
    "const",
    "ref",
    "continue",
    "let",
    "print",
    "println",
    "comptime",
    "dyn",
    "while",
    "and",
    "await",
    "default"
  };
#define stringpool ((const char *) &stringpool_contents)

#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
static const struct Keyword wordlist[] =
  {
#line 15 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str0, Keyword__Fn,},
#line 25 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str1, Keyword__For,},
#line 22 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str2, Keyword__Self,},
#line 9 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str3, Keyword__False,},
#line 16 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str4, Keyword__Struct,},
#line 8 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str5, Keyword__True,},
#line 11 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str6, Keyword__If,},
#line 38 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str7, Keyword__Mod,},
#line 33 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str8, Keyword__Enum,},
#line 32 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str9, Keyword__Error,},
#line 30 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str10, Keyword__Pub,},
#line 12 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str11, Keyword__Else,},
#line 28 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str12, Keyword__Break,},
#line 37 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str13, Keyword__Static,},
#line 14 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str14, Keyword__When,},
#line 17 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str15, Keyword__Trait,},
#line 43 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str16, Keyword__Sizeof,},
#line 24 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str17, Keyword__Loop,},
#line 39 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str18, Keyword__Macro,},
#line 40 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str19, Keyword__Derive,},
#line 34 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str20, Keyword__Type,},
#line 19 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str21, Keyword__Or,},
#line 13 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str22, Keyword__Mut,},
#line 20 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str23, Keyword__Impl,},
#line 29 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str24, Keyword__Match,},
#line 21 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str25, Keyword__Return,},
#line 23 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str26, Keyword__Const,},
#line 31 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str27, Keyword__Ref,},
#line 27 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str28, Keyword__Continue,},
#line 10 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str29, Keyword__Let,},
#line 44 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str30, Keyword__Print,},
#line 45 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str31, Keyword__Println,},
#line 36 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str32, Keyword__Comptime,},
#line 41 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str33, Keyword__Dyn,},
#line 26 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str34, Keyword__While,},
#line 18 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str35, Keyword__And,},
#line 35 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str36, Keyword__Await,},
#line 42 "./build/keywords.gperf"
    {(int)(size_t)&((struct stringpool_t *)0)->stringpool_str37, Keyword__Default,}
  };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

const struct Keyword *
kw_lookup_str (register const char *str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= MIN_HASH_VALUE)
        {
          register const struct Keyword *resword;

          switch (key - 2)
            {
              case 0:
                resword = &wordlist[0];
                goto compare;
              case 1:
                resword = &wordlist[1];
                goto compare;
              case 2:
                resword = &wordlist[2];
                goto compare;
              case 3:
                resword = &wordlist[3];
                goto compare;
              case 4:
                resword = &wordlist[4];
                goto compare;
              case 5:
                resword = &wordlist[5];
                goto compare;
              case 6:
                resword = &wordlist[6];
                goto compare;
              case 7:
                resword = &wordlist[7];
                goto compare;
              case 8:
                resword = &wordlist[8];
                goto compare;
              case 9:
                resword = &wordlist[9];
                goto compare;
              case 10:
                resword = &wordlist[10];
                goto compare;
              case 11:
                resword = &wordlist[11];
                goto compare;
              case 12:
                resword = &wordlist[12];
                goto compare;
              case 13:
                resword = &wordlist[13];
                goto compare;
              case 14:
                resword = &wordlist[14];
                goto compare;
              case 15:
                resword = &wordlist[15];
                goto compare;
              case 16:
                resword = &wordlist[16];
                goto compare;
              case 17:
                resword = &wordlist[17];
                goto compare;
              case 18:
                resword = &wordlist[18];
                goto compare;
              case 19:
                resword = &wordlist[19];
                goto compare;
              case 20:
                resword = &wordlist[20];
                goto compare;
              case 21:
                resword = &wordlist[21];
                goto compare;
              case 22:
                resword = &wordlist[22];
                goto compare;
              case 23:
                resword = &wordlist[23];
                goto compare;
              case 24:
                resword = &wordlist[24];
                goto compare;
              case 25:
                resword = &wordlist[25];
                goto compare;
              case 27:
                resword = &wordlist[26];
                goto compare;
              case 28:
                resword = &wordlist[27];
                goto compare;
              case 30:
                resword = &wordlist[28];
                goto compare;
              case 31:
                resword = &wordlist[29];
                goto compare;
              case 33:
                resword = &wordlist[30];
                goto compare;
              case 35:
                resword = &wordlist[31];
                goto compare;
              case 36:
                resword = &wordlist[32];
                goto compare;
              case 37:
                resword = &wordlist[33];
                goto compare;
              case 39:
                resword = &wordlist[34];
                goto compare;
              case 40:
                resword = &wordlist[35];
                goto compare;
              case 45:
                resword = &wordlist[36];
                goto compare;
              case 47:
                resword = &wordlist[37];
                goto compare;
            }
          return (struct Keyword *) 0;
        compare:
          {
            register const char *s = resword->id + stringpool;

            if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
              return resword;
          }
        }
    }
  return (struct Keyword *) 0;
}
#line 46 "./build/keywords.gperf"


const char* kw_string(const Keyword* self) {
	const i32 i = self->id;
	static constexpr const i32 POOLSIZE = (i32)sizeof(struct stringpool_t);
	assert(i >= 0 && i <= POOLSIZE);
	#if defined(NDEBUG)
	(void)POOLSIZE;
	#endif

	return (stringpool + i);
}
