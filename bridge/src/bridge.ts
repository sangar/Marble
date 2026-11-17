// A Matter bridge for the Marble radio nodes. Each node shows up as a dimmable light,
// a contact sensor (its button) and a temperature sensor.
//
//   node src/bridge.ts SERIAL_PORT NODE_ID...

import { Endpoint, ServerNode, VendorId } from "@matter/main";
import { BridgedDeviceBasicInformationServer } from "@matter/main/behaviors/bridged-device-basic-information";
import { ContactSensorDevice } from "@matter/main/devices/contact-sensor";
import { DimmableLightDevice } from "@matter/main/devices/dimmable-light";
import { TemperatureSensorDevice } from "@matter/main/devices/temperature-sensor";
import { AggregatorEndpoint } from "@matter/main/endpoints/aggregator";
import { ReadlineParser, SerialPort } from "serialport";

import { formatLevel, MessageKind, parseLine } from "./protocol.ts";

// Nodes report their temperature every minute; three missed reports mean the node is gone.
const UNREACHABLE_AFTER_MS = 3 * 60_000;

const [portPath, ...nodeArgs] = process.argv.slice(2).filter(arg => !arg.startsWith("--"));
const nodeIds = nodeArgs.map(Number);
if (!portPath || nodeIds.length === 0 || nodeIds.some(id => !Number.isInteger(id) || id < 1 || id > 255)) {
    console.error("usage: node src/bridge.ts SERIAL_PORT NODE_ID... (node IDs 1-255)");
    process.exit(1);
}

const server = await ServerNode.create({
    id: "marble-bridge",
    productDescription: { name: "Marble bridge", deviceType: AggregatorEndpoint.deviceType },
    basicInformation: {
        vendorName: "Marble",
        vendorId: VendorId(0xfff1),
        productName: "Marble bridge",
        productId: 0x8000,
    },
});
const aggregator = new Endpoint(AggregatorEndpoint, { id: "aggregator" });
await server.add(aggregator);

function bridged(nodeId: number, part: string) {
    const name = `Marble ${nodeId} ${part}`;
    const id = `node-${nodeId}-${part}`;
    return {
        id,
        bridgedDeviceBasicInformation: { nodeLabel: name, productName: name, uniqueId: id, reachable: false },
    };
}

class RadioNode {
    readonly id: number;
    readonly light;
    readonly contact;
    readonly temperature;
    lastHeardAt = 0;

    constructor(id: number) {
        this.id = id;
        this.light = new Endpoint(DimmableLightDevice.with(BridgedDeviceBasicInformationServer), bridged(id, "light"));
        this.contact = new Endpoint(ContactSensorDevice.with(BridgedDeviceBasicInformationServer), bridged(id, "button"));
        this.temperature = new Endpoint(
            TemperatureSensorDevice.with(BridgedDeviceBasicInformationServer),
            bridged(id, "temperature"),
        );
    }

    get endpoints(): Endpoint[] {
        return [this.light, this.contact, this.temperature];
    }

    level() {
        const { onOff } = this.light.state.onOff;
        return onOff ? (this.light.state.levelControl.currentLevel ?? 254) : 0;
    }

    async setReachable(reachable: boolean) {
        const patch = { bridgedDeviceBasicInformation: { reachable } };
        await this.light.set(patch);
        await this.contact.set(patch);
        await this.temperature.set(patch);
    }
}

const nodes = new Map(nodeIds.map(id => [id, new RadioNode(id)]));
for (const node of nodes.values()) {
    for (const endpoint of node.endpoints) await aggregator.add(endpoint);
    node.light.events.onOff.onOff$Changed.on(() => sendLevel(node));
    node.light.events.levelControl.currentLevel$Changed.on(() => sendLevel(node));
}

const port = new SerialPort({ path: portPath, baudRate: 115200 });
port.on("error", error => {
    console.error(`serial port ${portPath}: ${error.message}`);
    process.exit(1);
});

function sendLevel(node: RadioNode) {
    port.write(formatLevel(node.id, node.level()));
}

async function heard(node: RadioNode, kind: number, value: number) {
    node.lastHeardAt = Date.now();
    if (!node.light.state.bridgedDeviceBasicInformation.reachable) await node.setReachable(true);

    switch (kind) {
        case MessageKind.hello:
            sendLevel(node);
            break;
        case MessageKind.contact:
            await node.contact.set({ booleanState: { stateValue: value === 1 } });
            break;
        case MessageKind.temperature:
            await node.temperature.set({ temperatureMeasurement: { measuredValue: value } });
            break;
    }
}

port.pipe(new ReadlineParser({ delimiter: "\n" })).on("data", async (text: string) => {
    const line = parseLine(text);
    if (!line) return console.warn(`gateway: ${text.trimEnd()}`);
    if (line.type === "ready") {
        console.log("gateway ready");
        for (const node of nodes.values()) sendLevel(node);
        return;
    }

    const node = nodes.get(line.message.node);
    if (!node) return console.warn(`gateway: unknown node ${line.message.node}`);
    if (line.type === "lost") return node.setReachable(false);
    await heard(node, line.message.kind, line.message.value);
});

const reachabilityCheck = setInterval(async () => {
    for (const node of nodes.values()) {
        const silent = Date.now() - node.lastHeardAt > UNREACHABLE_AFTER_MS;
        if (silent && node.light.state.bridgedDeviceBasicInformation.reachable) await node.setReachable(false);
    }
}, 30_000);

await server.run();
clearInterval(reachabilityCheck);
port.close();
