#include <stdio.h>
#include "log.h"
#include "memory/cstr.h"

#define PROJECT_NAME "zeal"

int main(int argc, char **argv) {

    UNUSED(argc);
    UNUSED(argv);

    // const i32 len = stringlen("ayooooooo");

    // println("len: %d", len);
    
    const char* s = "ayyyo this is a string that we going to slice";
    const sslice sl = sslice_from_range(s, 0, 5);

    const cstr fstr = format_string("formatting into a cstr is: %s heres a number: %d", "working!", 69);

    sprintln(cstr_as_slice(&fstr));

    sprintln(sl);
    println("slice len: %d", sl.len);

    println("AYooo we printing! %s %d", "and wqe formatting!", 6969);

}
