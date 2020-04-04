#ifndef STRINGDECODER_H
#define STRINGDECODER_H

#define _CRT_SECURE_NO_WARNINGS
#ifdef _WIN64
#define INT __int64
#else
#define INT int
#endif

#define BUFSIZE 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <iconv.h>

class StringDecoder
{
public:
    StringDecoder();
    static char *DecodeString(char *);

protected:
    static char*   DecodePart(char *);
    static INT Base64decode(char *, const char *);
};

#endif // STRINGDECODER_H
