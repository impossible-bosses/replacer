const std = @import("std");

pub const UnescapeError = error {
    Alloc,
    Unescape,
};

pub fn unescapeAlloc(str: []const u8, allocator: std.mem.Allocator) UnescapeError![]u8
{
    var result = std.ArrayList(u8).init(allocator);
    defer result.deinit();

    var i: u64 = 0;
    while (i < str.len) : (i += 1) {
        if (str[i] == '\\') {
            const sequence = str[i + 1..str.len];
            const r = unescapeStartOfSequence(sequence) catch return error.Unescape;
            result.append(r.c) catch return error.Alloc;
            i += r.len;
        } else {
            result.append(str[i]) catch return error.Alloc;
        }
    }

    return result.toOwnedSlice();
}

const UnescapeSosResult = struct {
    c: u8,
    len: u64,
};

fn unescapeStartOfSequence(sequence: []const u8) !UnescapeSosResult
{
    if (sequence.len == 0) {
        return error.EmptySequence;
    }

    var result = UnescapeSosResult {
        .c = 0,
        .len = 1,
    };
    switch (sequence[0]) {
        '\\' => {
            result.c = '\\';
        },
        '0' => {
            result.c = 0;
        },
        'n' => {
            result.c = '\n';
        },
        'r' => {
            result.c = '\r';
        },
        't' => {
            result.c = '\t';
        },
        'x' => {
            if (sequence.len < 3) {
                return error.HexEscapeTooShort;
            }
            result.len = 3;
            const hex = sequence[1..3];
            const n = try std.fmt.parseUnsigned(u8, hex, 16);
            result.c = n;
        },
        else => {
            return error.UnrecognizedEscape;
        },
    }

    return result;
}

fn testUnescape(allocator: std.mem.Allocator, str: []const u8, expected: []const u8) !void
{
    const result = try unescapeAlloc(str, allocator);
    defer allocator.free(result);
    try std.testing.expectEqualSlices(u8, expected, result);
}

fn testUnescapeFail(allocator: std.mem.Allocator, str: []const u8) !void
{
    try std.testing.expectError(error.Unescape, unescapeAlloc(str, allocator));
}

test "unescape"
{
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer {
        if (gpa.deinit()) {
            std.log.err("GPA detected memory leaks (internal tool error)", .{});
        }
    }
    const allocator = gpa.allocator();

    try testUnescape(allocator,
        "escape a normal string",
        "escape a normal string"
    );
    try testUnescape(allocator,
        "escape a normal string\n",
        "escape a normal string\n"
    );
    try testUnescape(allocator,
        "escape a normal string\\n",
        "escape a normal string\n"
    );
    try testUnescape(allocator,
        "",
        ""
    );
    try testUnescape(allocator,
        "\x00\x00\x00\x00\x00\n\x00\x00\x00\n",
        "\x00\x00\x00\x00\x00\n\x00\x00\x00\n"
    );
    try testUnescape(allocator,
        "\\\\\\\\\\\\\\\\",
        "\\\\\\\\"
    );
    try testUnescape(allocator,
        "\r",
        "\r"
    );
    try testUnescape(allocator,
        "\\r",
        "\r"
    );
    try testUnescape(allocator,
        "\t",
        "\t"
    );
    try testUnescape(allocator,
        "\\t",
        "\t"
    );
    try testUnescape(allocator,
        "\x00\x01\x02\x03\x04",
        "\x00\x01\x02\x03\x04"
    );
    try testUnescape(allocator,
        "\\x00\\x01\\x02\\x03\\x04",
        "\x00\x01\x02\x03\x04"
    );
    try testUnescape(allocator,
        "\xab\xba\xff\xfc\xf1\x9f\x0c",
        "\xab\xba\xff\xfc\xf1\x9f\x0c"
    );
    try testUnescape(allocator,
        "\\xab\\xba\\xff\\xfc\\xf1\\x9f\\x0c",
        "\xab\xba\xff\xfc\xf1\x9f\x0c"
    );

    try testUnescapeFail(allocator, "\n\n\n\n\\");
    try testUnescapeFail(allocator, "\\\\\\\\\\\\\\");
    try testUnescapeFail(allocator, "\\y");
    try testUnescapeFail(allocator, "\\x00\\x01\\x02\\x03\\x0r");
}
