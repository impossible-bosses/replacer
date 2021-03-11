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

struct String Unescape(const struct String str)
{
    const struct String nullString = {0};
    size_t size = 0;

    for (size_t i = 0; i < str.size; i++) {
        if (str.str[i] == '\\') {
            if (i == str.size - 1) {
                // Error: escape at the end of the string
                return nullString;
            }

            i++;
            char c;
            if (!UnescapeChar(str.str[i], &c)) {
                LOG_ERROR("Unrecognized escape char \"%c\"\n", str.str[i]);
                return nullString;
            }
            size++;
        }
        else {
            size++;
        }
    }

    struct String result = StringAlloc(size);
    size = 0;
    for (size_t i = 0; i < str.size; i++) {
        if (str.str[i] == '\\') {
            if (i == str.size - 1) {
                // Error: escape at the end of the string
                StringFree(result);
                return nullString;
            }

            i++;
            char c;
            if (!UnescapeChar(str.str[i], &c)) {
                LOG_ERROR("Unrecognized escape char \"%c\"\n", str.str[i]);
                StringFree(result);
                return nullString;
            }
            result.str[size++] = c;
        }
        else {
            result.str[size++] = str.str[i];
        }
    }
    return result;
}
