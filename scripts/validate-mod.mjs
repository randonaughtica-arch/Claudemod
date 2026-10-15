// Checks mod.json before the (slow) native builds run: required fields, and
// that every setting key the C++ code reads exists with the right type. A
// missing key doesn't break the build, it silently breaks that toggle in game.
//
// Usage: node scripts/validate-mod.mjs   (Node.js 24+, no dependencies)

import { readFileSync, readdirSync } from "node:fs";
import { join, relative } from "node:path";

const root = join(import.meta.dirname, "..");
const errors = [];

let mod;
try {
    mod = JSON.parse(readFileSync(join(root, "mod.json"), "utf8"));
}
catch (err) {
    console.error(`mod.json is not valid JSON: ${err.message}`);
    process.exit(1);
}

for (const field of ["geode", "gd", "id", "name", "version", "developer"]) {
    if (mod[field] === undefined) {
        errors.push(`mod.json is missing "${field}"`);
    }
}
if (typeof mod.id === "string" && !/^[a-z0-9\-_]+\.[a-z0-9\-_]+$/.test(mod.id)) {
    errors.push(`mod id "${mod.id}" must look like developer.mod-name (lowercase letters, digits, - and _)`);
}
if (typeof mod.version === "string" && !/^v?\d+\.\d+\.\d+/.test(mod.version)) {
    errors.push(`version "${mod.version}" is not a semver version`);
}
if (typeof mod.geode === "string" && !/^v?\d+\.\d+\.\d+/.test(mod.geode)) {
    errors.push(`geode "${mod.geode}" must be an exact SDK version, since the workflow builds against it`);
}

// Collect the setting keys the sources use, with the type each one needs
const sources = readdirSync(join(root, "src"), { recursive: true })
    .filter((file) => file.endsWith(".cpp") || file.endsWith(".hpp"))
    .map((file) => join(root, "src", file));

const usedKeys = new Map();
const use = (key, type, file) => {
    if (!usedKeys.has(key)) {
        usedKeys.set(key, { type, file: relative(root, file) });
    }
};

for (const file of sources) {
    const text = readFileSync(file, "utf8");
    // Rows of the HACKS table: { Hack::Noclip, Tab::Level, "noclip" }
    for (const [, key] of text.matchAll(/\{\s*Hack::\w+\s*,\s*Tab::\w+\s*,\s*"([^"]+)"\s*\}/g)) {
        use(key, "bool", file);
    }
    for (const [, key] of text.matchAll(/SPEED_KEY\s*=\s*"([^"]+)"/g)) {
        use(key, "float", file);
    }
    for (const [, key] of text.matchAll(/listenForKeybindSettingPresses\(\s*"([^"]+)"/g)) {
        use(key, "keybind", file);
    }
}

if (usedKeys.size === 0) {
    errors.push("found no setting keys in src/, so the patterns in this script are out of date");
}

const settings = mod.settings ?? {};
for (const [key, { type, file }] of usedKeys) {
    const setting = settings[key];
    if (!setting) {
        errors.push(`${file} uses setting "${key}", which mod.json doesn't define`);
    }
    else if (setting.type !== type) {
        errors.push(`setting "${key}" is "${setting.type}" in mod.json, but ${file} uses it as "${type}"`);
    }
}

if (errors.length > 0) {
    for (const error of errors) {
        console.error(`error: ${error}`);
    }
    process.exit(1);
}

console.log(`mod.json OK: ${mod.id} ${mod.version}, Geode ${mod.geode}, ${usedKeys.size} setting keys checked (Node.js ${process.version})`);
