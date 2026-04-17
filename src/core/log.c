#include "core/log.h"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "memory/cstr.h"
#include "stdarg.h"

static inline void fdwrite(i32 fd, const char* src, isize len) {

  i32 nbytes = 0;

  while(nbytes < len) {
    const void* start = &src[nbytes];
    const isize remaining = len - nbytes;
    const i32 n = write(fd, start, remaining);
    if (n == -1) {
      perror("POSIX write returned -1 while attempting to write to Stdout Stream!");
      return;
    }

    nbytes += n;
  }  
}

static inline void stdout_write(const char* src, isize len) {
  fdwrite(STDOUT_FILENO, src, len);  
}

static inline void stderr_write(const char* src, isize len) {
  fdwrite(STDERR_FILENO, src, len);
}

cstr format_string(const char* fmt, ...) {
  va_list args;
  va_start(args);

  const i32 len = vsnprintf(nullptr, 0, fmt, args) + 1;
  char buf[len] = {};


  vsnprintf(buf, len, fmt, args);

  va_end(args);

  static constexpr const i32 SMALL_SIZE = cast(i32, SMALL_BUF_SIZE); 

  if (UNLIKELY(len <= SMALL_SIZE)) { 
    return cstr_small_new(buf);
  }

  return cstr_new(buf);
  
}

void sfprint(FILE* fd, sslice str) {
  
  const void* src = pcast(void, str.begin);
  const isize len = str.len;
  fwrite(src,  len, 1, fd);
  fflush(fd);
}

static constexpr const i32 NL_SIZE = sizeof("\n");
static constexpr const char NL[] = "\n";

void sfprintln(FILE* fd, sslice str) {

  const void* src = pcast(void, str.begin);
  const isize len = str.len;
  fwrite(src,  len, 1, fd);
  fwrite(NL,  NL_SIZE, 1, fd);
  fflush(fd);
}

void sprint(sslice str) {
  stdout_write(str.begin,  str.len);
}

void sprintln(sslice str) {
  stdout_write(str.begin, str.len);
  stdout_write(NL, NL_SIZE);
}

void seprint(sslice str) {
  stderr_write(str.begin, str.len);
}

void seprintln(sslice str) {
  stderr_write(str.begin, str.len);
  stderr_write(NL, NL_SIZE);
}

