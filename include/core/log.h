#pragma once

#include <stdio.h>

#include "attributes.h"
#include "memory/cstr.h"

// #define format(lit, ...)

// FORMAT_FUNC(1,2)
// void println(const char* fmt, ...);

/// Formats arguments int a new [cstr] using
/// a [printf] style format string
FORMAT_FUNC(1, 2)
[[nodiscard("There is a good chance that the returned formatted string needs to be freed! Possible Memory Leak!")]]
cstr format_string(const char* fmt, ...);

#define fdprint(fd, fmt, ...) (fprintf(fd, fmt __VA_OPT__(, ) __VA_ARGS__))

#define print(fmt, ...) (fprintf(stdout, fmt __VA_OPT__(,) __VA_ARGS__)
#define eprint(fmt, ...) (fprintf(stderr, fmt __VA_OPT__(, ) __VA_ARGS__))
#define fprintln(fd, fmt, ...) (fdprint(fd, fmt "\n" __VA_OPT__(, ) __VA_ARGS__))

// #define fprintln(fd, fmt, ...) (fprintf(fd, fmt "\n" __VA_OPT__(, ) __VA_ARGS__))

#define println(fmt, ...) (fprintln(stdout, fmt, __VA_ARGS__))
// (fprintf(stdout, fmt "\n" __VA_OPT__(, ) __VA_ARGS__))

#define eprintln(fmt, ...) (fprintln(stdout, fmt, __VA_ARGS__))

/// Prints a given string [sslice] to
/// a file. This is a verstion of [fdprint] that does not require
/// null-terminated strings. However this function does not
/// do any formatting. If you need to print a formatted string. see [fdprint] and others
///
PARAMS_NONNULL(1)
void sfprint(FILE* fd, sslice str);

/// Same as [sfprint], but appends a newline character to the end of
/// give string [sslice]
/// 
/// Prints a given string [sslice] to
/// a file. This is a verstion of [fdprint] that does not require
/// null-terminated strings. However this function does not
/// do any formatting. If you need to print a formatted string. see [fdprint] and others
///
PARAMS_NONNULL(1)
void sfprintln(FILE* fd, sslice str);

void sprint(sslice str);

void sprintln(sslice str);

void seprint(sslice str);

void seprintln(sslice str);

// #define sprint(slice) (sfprint(stdout, (slice)))

// #define sprintln(slice) (sfprintln(stdout, (slice)))

// #define seprint(slice) (sfprint(stderr, (slice)))
// #define seprintln(slice) (sfprintln(stderr, (slice)))

