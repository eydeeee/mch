#include "stringdecoder.h"

StringDecoder::StringDecoder()
{
}

char* StringDecoder::DecodeString(char *in)
{
    char *ret, *p1, *p2, *tmp, sub[BUFSIZE];

    if(!in) return 0;
    if(in[0] != '=' || in[1] != '?')
    {
        ret = (char *)malloc((1+strlen(in))*sizeof(char));
        if(ret) strcpy(ret, in);
        return ret;
    }

    p1 = in;
    p2 = strchr(in, ' ');
    if(!p2) p2 = strchr(in, '\t');

    memset(sub, 0, BUFSIZE);
    if(p2) strncpy(sub, p1, p2-p1);
    else strcpy(sub, p1);

    ret = DecodePart(sub);
    if(!ret) return 0;

    if(p2)
    {
        while(1)
        {
            p1 = p2+1;
            p2 = strchr(p1, ' ');
            if(!p2) p2 = strchr(p1, '\t');

            memset(sub, 0, BUFSIZE);
            if(p2) strncpy(sub, p1, p2-p1);
            else strcpy(sub, p1);
            tmp = DecodePart(sub);
            if(!tmp) break;
            strcat(ret, tmp);
            free(tmp);
            if(!p2) break;
        }
    }

    return ret;
}

char* StringDecoder::DecodePart(char *in)
{
    char enc[64], *i, *is, *o, *os, *p1, *p2;
    iconv_t handle;
    size_t ileft, oleft, ret;
    int type, index;

    is = i = (char *)malloc(BUFSIZE * sizeof(char));
    os = o = (char *)malloc(BUFSIZE * sizeof(char));
    memset(i, 0, BUFSIZE);
    memset(o, 0, BUFSIZE);

    p1 = in+2; p1 = strchr(p1, '?');
    memset(enc, 0, 64);
    strncat(enc, in+2, p1-in-2);

    if(*(p1+1) == 'q' || *(p1+1) == 'Q') type = 0;
    else type = 1;

    memset(i, 0, BUFSIZE);

    p2 = strchr(p1+1, '?'); p2++;
    p1 = strstr(p2, "?=");

    strncat(i, p2, p1-p2);

    index = 0;
    if(!type)
    {
        for(size_t x = 0; x<strlen(i); x++)
            if(i[x] == '=')
            {
                char str1[64];
                memset(str1, 0, 64);
                sprintf(str1, "0x%c%c", i[x+1], i[x+2]);
                o[index++] = (int)strtol(str1, 0, 0);
                x+=2;
            }
            else if(i[x] == '_')
            {
                o[index++] = 0x20;
            }
            else
            {
                o[index++] = i[x];
            }
    }
    else Base64decode(o, i);

    handle = iconv_open("UTF-8", enc);
    if(handle==(iconv_t)-1) return 0;

    oleft = strlen(o);
    ileft = BUFSIZE;
    memset(i, 0, BUFSIZE);

    ret = iconv(handle, (char **)&o, &oleft, &i, &ileft);
    if(ret==(size_t)-1) return 0;

    iconv_close(handle);

    if(os) free(os);
    return is;
}

INT StringDecoder::Base64decode(char *bufplain, const char *bufcoded)
{
    static const unsigned char pr2six[256] =
    {
        /* ASCII table */
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    INT nbytesdecoded;
    register const unsigned char *bufin;
    register unsigned char *bufout;
    register INT nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);
    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *) bufplain;
    bufin = (const unsigned char *) bufcoded;

    while (nprbytes > 4) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    bufin += 4;
    nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
    *(bufout++) =
        (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3) {
    *(bufout++) =
        (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    *(bufout++) = '\0';
    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}
