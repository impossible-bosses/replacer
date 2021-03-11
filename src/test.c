#include <stdio.h>

#include "replacer.h"

struct EscapeTest
{
    struct String input, output;
};

struct ReplaceTest
{
    struct String filePath, src, dst, outFilePath;
};

int main(int argc, char* argv[])
{
    const struct String nullString = {0};

    // Escape tests
    const struct EscapeTest escapeTests[] = {
        {
            ToString("escape a normal string"),
            ToString("escape a normal string")
        },
        {
            ToString("escape a normal string\\n"),
            ToString("escape a normal string\n")
        },
        {
            ToString(""),
            ToString("")
        },
        {
            {
                .size = 10,
                .str = "\0\0\0\0\0\n\0\0\0\n"
            },
            {
                .size = 10,
                .str = "\0\0\0\0\0\n\0\0\0\n"
            }
        },
        {
            ToString("\\0\\0\\0\\0\\0\\n\\0\\0\\0\\n"),
            {
                .size = 10,
                .str = "\0\0\0\0\0\n\0\0\0\n"
            }
        },
        {
            ToString("\\\\\\\\\\\\\\\\"),
            ToString("\\\\\\\\")
        },
        {
            ToString("\\r"),
            ToString("\r")
        },
        {
            ToString("\\t"),
            ToString("\t")
        },
        {
            ToString("\n\n\n\n\\"),
            nullString
        },
        {
            ToString("\\\\\\\\\\\\\\"),
            nullString
        },
        {
            ToString("\\x00\\x01\\x02\\x03\\x04"),
            {
                .size = 5,
                .str = "\x00\x01\x02\x03\x04"
            }
        },
        {
            ToString("\\xab\\xba\\xff\\xfc\\xf1\\x9f\\x0c"),
            ToString("\xab\xba\xff\xfc\xf1\x9f\x0c")
        }
    };

    for (int i = 0; i < C_ARRAY_LENGTH(escapeTests); i++) {
        const struct String result = Unescape(escapeTests[i].input);
        if (result.str == NULL) {
            if (escapeTests[i].output.str != NULL) {
                LOG_ERROR("NULL result for escape test #%d\n", i);
                LOG_FLUSH();
                return 1;
            }
        }
        else {
            if (!StringEqual(result, escapeTests[i].output)) {
                LOG_ERROR("Unequal result for escape test #%d, got length %d, \"%.*s\"\n", i, (int)result.size, (int)result.size, result.str);
                LOG_FLUSH();
                return 1;
            }
        }
    }

    // Replace tests
    const struct ReplaceTest replaceTests[] = {
        {
            ToString("test1-in.txt"),
            ToString("ingtestingtest"),
            ToString("666"),
            ToString("test1-out.txt")
        },
        {
            ToString("test2-in.w3i"),
            ToString("Impossible Bosses DEV MAP"),
            ToString("Impossible Bosses vX.Y.ZZ"),
            ToString("test2-out.w3i")
        }
    };

    const struct String testPath = ToString("temp-test-data");
    for (int i = 0; i < C_ARRAY_LENGTH(replaceTests); i++) {
        const struct String inPath = replaceTests[i].filePath;
        if (!KmCopyFile(inPath, testPath, false)) {
            LOG_ERROR("Failed to copy test file for test #%d, file \"%.*s\n", i, (int)inPath.size, inPath.str);
            LOG_FLUSH();
            return 1;
        }

        if (!Replace(testPath, replaceTests[i].src, replaceTests[i].dst)) {
            LOG_ERROR("Replace failed for test #%d\n", i);
            LOG_FLUSH();
            return 1;
        }

        const struct String result = ReadEntireFile(testPath);
        if (result.str == NULL) {
            LOG_ERROR("Failed to read output file for test #%d\n", i);
            LOG_FLUSH();
            return 1;
        }
        const struct String resultExpected = ReadEntireFile(replaceTests[i].outFilePath);
        if (resultExpected.str == NULL) {
            LOG_ERROR("Failed to read expected output file for test #%d\n", i);
            LOG_FLUSH();
            return 1;
        }

        if (!StringEqual(result, resultExpected)) {
            LOG_ERROR("Unequal output for test #%d\n", i);
            LOG_FLUSH();
            return 1;
        }
    }

    LOG_INFO("All tests passed\n");
    LOG_FLUSH();
    return 0;
}

#include "common.c"
#include "replacer.c"
