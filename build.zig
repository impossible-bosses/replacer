const std = @import("std");

const PROJECT_NAME = "replacer";

pub fn build(b: *std.build.Builder) void
{
    const mode = b.standardReleaseOptions();
    const target = b.standardTargetOptions(.{});

    const server = b.addExecutable(PROJECT_NAME, "src/main.zig");
    server.setBuildMode(mode);
    server.setTarget(target);
    server.install();

    const runTests = b.step("test", "Run tests");

    const testSrcs = [_][]const u8 {
        "src/unescape.zig",
    };
    for (testSrcs) |src| {
        const tests = b.addTest(src);
        tests.setBuildMode(mode);
        tests.setTarget(target);
        runTests.dependOn(&tests.step);
    }
}
