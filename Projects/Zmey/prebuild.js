const fs = require("fs");
const path = require("path");
const process = require("process");
const exec = require('child_process').execSync;
const sys = require("sys");

const scriptDir = __dirname;
const rootDir = path.join(scriptDir, "../../");

const runToolsIfNeeded = function (toolsToRun) {
    console.log(toolsToRun);
    if (!fs.existsSync("prebuilddata.json")) {
        fs.writeFileSync("prebuilddata.json", "{}", {encoding: "utf8"});
    }
    const storageData = JSON.parse(fs.readFileSync("prebuilddata.json", "utf8", "r"));
    for (const toolInfo of toolsToRun) {
        const lastChangedDate = fs.statSync(toolInfo.directory).mtime;
        const lastRecordedDate = new Date(storageData[toolInfo.directory] || 0);
        if (lastRecordedDate < lastChangedDate) {
            console.log(`Detected that dir ${toolInfo.directory} has changed. Running ${toolInfo.tool}...`);
            exec(toolInfo.tool, { cwd: rootDir});
            storageData[toolInfo.directory] = Date.now();
        }
    }
    fs.writeFileSync("prebuilddata.json", JSON.stringify(storageData), {encoding: "utf8"});
};

let main = function () {
    const buildOutputDir = process.argv[2];
    const isGame = process.argv[3] === "--game";

    const idlScriptsDir = path.join(rootDir, "Source/Scripting/idl");
    const idlCompiler = path.join(rootDir, "Tools/IdlCompiler/idl_compiler.js");

    const shadersDir = path.join(rootDir, "Source/Graphics/Shaders/Source");
    const shaderCompiler = path.join(buildOutputDir, "ShaderCompiler.exe");

    const contentDir = path.join(rootDir, "Games/GiftOfTheSanctum/Content");
    const incineratorCompiler = path.join(buildOutputDir, "Incinerator.exe");

    const engineToolsToRun = [
        { directory: idlScriptsDir, tool: `node ${idlCompiler}` },
        { directory: shadersDir, tool: shaderCompiler },
    ];
    const gameToolsToRun = [
        { directory: contentDir, tool: `${incineratorCompiler} --game Games/GiftOfTheSanctum` }
    ];
    const toolsToRun = isGame ? gameToolsToRun : engineToolsToRun;
    runToolsIfNeeded(toolsToRun);
};
try {
    main();
}
catch (err) {
    console.error(err);
    sys.exit(1);
}