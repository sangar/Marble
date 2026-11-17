// The gateway's serial lines, as written by src/gateway.c and read by src/line.c.

export const MessageKind = {
    hello: 1,
    level: 2,
    contact: 3,
    temperature: 4,
} as const;

export type Message = { node: number; kind: number; value: number };

export type GatewayLine =
    | { type: "ready" }
    | { type: "received"; message: Message }
    | { type: "lost"; message: Message };

const messagePattern = /^(lost )?(\d{1,3}) (\d) (-?\d{1,5})$/;

export function parseLine(line: string): GatewayLine | undefined {
    const text = line.trimEnd();
    if (text === "ready") return { type: "ready" };

    const match = messagePattern.exec(text);
    if (!match) return undefined;

    const message = { node: Number(match[2]), kind: Number(match[3]), value: Number(match[4]) };
    return { type: match[1] ? "lost" : "received", message };
}

export function formatLevel(node: number, level: number): string {
    return `${node} ${MessageKind.level} ${level}\n`;
}
