#pragma once

#include "common.h"

struct String Unescape(const struct String str);
bool Replace(const struct String filePath, const struct String replaceSrc, const struct String replaceDst);
