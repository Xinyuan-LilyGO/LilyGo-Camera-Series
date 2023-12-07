#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

#define NON_NUM '0'

inline int hex2num(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;

    // printf("unexpected char: %c", c);
    return NON_NUM;
}


/**
 * @brief Decode URL string
 *
 * @param[in]  str        input string
 * @param[in]  strSize    input string size (excluding final \0)
 * @param[out] result     output string buffer
 * @param[in]  resultSize output buffer size (including the final \0)
 *
 * @return: >0 Effective length of the output buffer
 *          0  Decoding failed
 */
inline int urlDecode(const char *str, const int strSize, char *result, const int resultSize)
{
    char ch, ch1, ch2;
    int i;
    int j = 0; // record result index

    if (str == NULL || result == NULL || strSize <= 0 || resultSize <= 0)
    {
        return 0;
    }

    for (i = 0; (i < strSize) && (j < resultSize); ++i)
    {
        ch = str[i];
        switch (ch)
        {
            case '+':
                result[j++] = ' ';
                break;
            case '%':
                if (i + 2 < strSize)
                {
                    ch1 = hex2num(str[i + 1]); //高4位
                    ch2 = hex2num(str[i + 2]); //低4位
                    if ((ch1 != NON_NUM) && (ch2 != NON_NUM))
                        result[j++] = (char)((ch1 << 4) | ch2);
                    i += 2;
                }
                break;
            default:
                result[j++] = ch;
                break;
        }
    }

    result[j] = 0;
    return j;
}


/**
 * @brief Encode URL string
 *
 * @param[in]  str        input string
 * @param[in]  strSize    input string size (excluding final \0)
 * @param[out] result     output string buffer
 * @param[in]  resultSize output buffer size (including the final \0)
 *
 * @return: >0 Effective length of the output buffer
 *          0  encoding failed
 */
inline int urlEncode(const char *str, const int strSize, char *result, const int resultSize)
{
    int i;
    int j = 0; // for result index
    char ch;

    if (str == NULL || result == NULL || strSize <= 0 || resultSize <= 0)
    {
        return 0;
    }

    for (i = 0; (i < strSize) && (j < resultSize); ++i)
    {
        ch = str[i];
        if (((ch >= 'A') && (ch < 'Z')) ||
            ((ch >= 'a') && (ch < 'z')) ||
            ((ch >= '0') && (ch < '9')))
        {
            result[j++] = ch;
        }
        else if (ch == ' ')
        {
            result[j++] = '+';
        }
        else if (ch == '.' || ch == '-' || ch == '_' || ch == '*')
        {
            result[j++] = ch;
        }
        else
        {
            if (j + 3 < resultSize)
            {
                sprintf(result + j, "%%%02X", (unsigned char)ch);
                j += 3;
            }
            else
            {
                return 0;
            }
        }
    }

    result[j] = '\0';
    return j;
}

#ifdef __cplusplus
}
#endif

#endif
