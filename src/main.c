#include "common.h"
#include "replacer.h"

int main(int argc, char* argv[])
{
    if (argc != 4) {
        LOG_ERROR("replacer takes 3 arguments: replacer <file-path> <replace-src> <replace-dst>\n");
        LOG_FLUSH();
        return 1;
    }

    const struct String filePath = ToString(argv[1]);
    const struct String replaceSrcEscaped = ToString(argv[2]);
    const struct String replaceSrc = Unescape(replaceSrcEscaped);
    if (replaceSrc.str == NULL) {
        LOG_FLUSH();
        return 1;
    }
    const struct String replaceDstEscaped = ToString(argv[3]);
    const struct String replaceDst = Unescape(replaceDstEscaped);
    if (replaceDst.str == NULL) {
        LOG_FLUSH();
        return 1;
    }

    if (!Replace(filePath, replaceSrc, replaceDst)) {
        LOG_FLUSH();
        return 1;
    }

    StringFree(replaceSrc);
    StringFree(replaceDst);

    LOG_FLUSH();
    return 0;
}

#include "common.c"
#include "replacer.c"

#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>
#undef STB_SPRINTF_IMPLEMENTATION
