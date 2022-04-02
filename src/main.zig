const std = @import("std");

const unescape = @import("unescape.zig");

pub const log_level: std.log.Level = .err;

const Mode = enum {
    Stdout,
    Inplace,
};

const MODE_STRING_MAP = std.ComptimeStringMap(Mode, .{
    .{ "stdout", .Stdout },
    .{ "inplace", .Inplace },
});

const Args = struct {
    filePath: []const u8,
    src: []const u8,
    dst: []const u8,
    mode: Mode,
};

fn stringToMode(str: []const u8) ?Mode
{
    return MODE_STRING_MAP.get(str);
}

fn parseArgs(args: [][]const u8) ?Args
{
    if (args.len != 5) {
        std.log.err("replacer takes 4 arguments: <mode> <file-path> <replace-src> <replace-dst>", .{});
        return null;
    }

    const modeStr = args[1];
    const mode = stringToMode(modeStr) orelse {
        std.log.err("Unrecognized mode {s}", .{modeStr});
        // TODO we can log out all the modes
        return null;
    };

    return Args {
        .filePath = args[2],
        .src = args[3],
        .dst = args[4],
        .mode = mode,
    };
}

pub fn main() !void
{
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer {
        if (gpa.deinit()) {
            std.log.err("GPA detected memory leaks (internal tool error)", .{});
        }
    }
    const allocator = gpa.allocator();

    const cmdArgs = try std.process.argsAlloc(allocator);
    defer allocator.free(cmdArgs);

    const args = parseArgs(cmdArgs) orelse return error.ParseArgs;

    const srcUnescaped = unescape.unescapeAlloc(args.src, allocator) catch |err| {
        std.log.err("Error un-escaping replace src {s}", .{args.src});
        return err;
    };
    defer allocator.free(srcUnescaped);
    const dstUnescaped = unescape.unescapeAlloc(args.dst, allocator) catch |err| {
        std.log.err("Error un-escaping replace dst {s}", .{args.dst});
        return err;
    };
    defer allocator.free(dstUnescaped);

    const cwd = std.fs.cwd();
    const MAX_FILE_SIZE = 32 * 1024 * 1024 * 1024; // 32 GB, idk
    const fileData = cwd.readFileAlloc(allocator, args.filePath, MAX_FILE_SIZE) catch |err| {
        std.log.err("Error opening file {s}", .{args.filePath});
        return err;
    };
    defer allocator.free(fileData);

    const newSize = std.mem.replacementSize(u8, fileData, srcUnescaped, dstUnescaped);
    var buf = try allocator.alloc(u8, newSize);
    defer allocator.free(buf);
    _ = std.mem.replace(u8, fileData, srcUnescaped, dstUnescaped, buf);

    switch (args.mode) {
        .Stdout => {
            const stdout = std.io.getStdOut();
            _ = try stdout.write(buf);
        },
        .Inplace => {
            cwd.writeFile(args.filePath, buf) catch |err| {
                std.log.err("Error writing back to file {s}", .{args.filePath});
                return err;
            };
        },
    }
}
