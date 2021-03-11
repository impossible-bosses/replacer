#include "replacer.h"

bool Replace(const struct String filePath, const struct String replaceSrc, const struct String replaceDst)
{
    const struct String file = ReadEntireFile(filePath);
    if (file.str == NULL) {
        LOG_ERROR("Failed to read input file %.*s\n", (int)filePath.size, filePath.str);
        return false;
    }

    const size_t findIndex = StringFindFirstSubstring(file, replaceSrc);
    if (findIndex == file.size) {
        LOG_ERROR("replace-src not found in input file\n");
        return false;
    }

    if (file.size < replaceSrc.size) {
        LOG_ERROR("Unexpected error: file size %zu smaller than replace-src %zu\n", file.size, replaceSrc.size);
        return false;
    }
    const size_t dstSize = file.size - replaceSrc.size + replaceDst.size;
    struct String dst = StringAlloc(dstSize);
    MemCopy(dst.str, file.str, findIndex);
    MemCopy(dst.str + findIndex, replaceDst.str, replaceDst.size);
    MemCopy(dst.str + findIndex + replaceDst.size, file.str + findIndex + replaceSrc.size, file.size - findIndex - replaceSrc.size);

    if (!WriteEntireFile(filePath, dst.size, dst.str, false)) {
        LOG_ERROR("Failed to write input file %.*s\n", (int)filePath.size, filePath.str);
        return false;
    }

    return true;
}

bool UnescapeChar(char c, char* outC)
{
    switch (c) {
    case '\\': {
        *outC = '\\';
    } break;
    case '0': {
        *outC = 0;
    } break;
    case 'n': {
        *outC = '\n';
    } break;
    case 'r': {
        *outC = '\r';
    } break;
    case 't': {
        *outC = '\t';
    } break;
    default: {
        return false;
    } break;
    }

    return true;
}

size_t UnescapeSequence(const struct String sequence, char* outC)
{
    if (sequence.size == 0) {
        return 0;
    }

    size_t size = 1;
    switch (sequence.str[0]) {
    case '\\': {
        *outC = '\\';
    } break;
    case '0': {
        *outC = 0;
    } break;
    case 'n': {
        *outC = '\n';
    } break;
    case 'r': {
        *outC = '\r';
    } break;
    case 't': {
        *outC = '\t';
    } break;
    case 'x': {
        if (sequence.size < 3) {
            size = 0;
        }
        else {
            size = 3;
            const int digit1 = CharHexDigitToInt(sequence.str[1]);
            const int digit2 = CharHexDigitToInt(sequence.str[2]);
            if (digit1 == -1 || digit2 == -1) {
                return 0;
            }
            const unsigned char c = digit1 * 16 + digit2;
            *outC = c;
        }
    } break;
    default: {
        size = 0;
    } break;
    }

    return size;
}

bool UnescapeString(const struct String str, struct String* outStr, bool sizeOnly)
{
    size_t ind = 0;
    for (size_t i = 0; i < str.size; i++) {
        if (str.str[i] == '\\') {
            const struct String sequence = StringSlice(str, i + 1, str.size);
            char escC;
            const size_t escResult = UnescapeSequence(sequence, &escC);
            if (escResult == 0) {
                return false;
            }
            if (!sizeOnly) {
                if (ind >= outStr->size) {
                    return false;
                }
                outStr->str[ind] = escC;
            }
            ind++;
            i += escResult;
        }
        else {
            if (!sizeOnly) {
                if (ind >= outStr->size) {
                    return false;
                }
                outStr->str[ind] = str.str[i];
            }
            ind++;
        }
    }

    if (sizeOnly) {
        outStr->size = ind;
    }
    else if (ind != outStr->size) {
        return false;
    }
    return true;
}

struct String Unescape(const struct String str)
{
    const struct String nullString = {0};
    struct String result;
    if (!UnescapeString(str, &result, true)) {
        return nullString;
    }

    result = StringAlloc(result.size);
    if (!UnescapeString(str, &result, false)) {
        return nullString;
    }

    return result;
}
