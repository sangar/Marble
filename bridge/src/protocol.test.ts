import assert from "node:assert/strict";
import { test } from "node:test";

import { formatLevel, parseLine } from "./protocol.ts";

test("parses received and lost messages", () => {
    assert.deepEqual(parseLine("3 4 -1250\r"), { type: "received", message: { node: 3, kind: 4, value: -1250 } });
    assert.deepEqual(parseLine("lost 3 2 128"), { type: "lost", message: { node: 3, kind: 2, value: 128 } });
    assert.deepEqual(parseLine("ready"), { type: "ready" });
});

test("ignores lines that are not messages", () => {
    for (const line of ["", "3 2", "3 2 1 4", "x 2 1", "lost", "3 2 1x"]) {
        assert.equal(parseLine(line), undefined, line);
    }
});

test("formats a level for the gateway", () => {
    assert.equal(formatLevel(3, 128), "3 2 128\n");
});
