#include <Regex.h>
#include <regex.h>
#include <iostream>

using namespace std;

int
regex_match(string text, string pattern)
{
    int    status;
    regex_t    re;

    if (regcomp(&re, pattern.c_str(), REG_EXTENDED|REG_NOSUB) != 0) {
        return(0);      
    }
    status = regexec(&re, text.c_str(), (size_t) 0, NULL, 0);
    regfree(&re);
    return status;
}

