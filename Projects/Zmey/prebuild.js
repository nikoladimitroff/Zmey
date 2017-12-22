const fs = require("fs");
const path = require("path");
const process = require("process");
const exec = require('child_process').execSync;

const scriptDir = __dirname;
const rootDir = path.join(scriptDir, "../../");

var walkSync = function(dir, filelist) {
    const files = fs.readdirSync(dir);
    filelist = filelist || [];
    files.forEach(function(file) {
        const filepath = path.join(dir, file);
        if (fs.statSync(filepath).isDirectory()) {
            filelist = walkSync(path.join(dir, file), filelist);
        }
        else {
            filelist.push(filepath);
        }
    });
    return filelist;
};

const getLastModifiedTime = function (directory) {
    const files = walkSync(directory);
    return Math.max.apply(null, files.map(f => fs.statSync(f).mtime));
};

const runToolsIfNeeded = function (toolsToRun) {
    if (!fs.existsSync("prebuilddata.json")) {
        fs.writeFileSync("prebuilddata.json", "{}", {encoding: "utf8"});
    }
    const storageData = JSON.parse(fs.readFileSync("prebuilddata.json", "utf8", "r"));
    for (const toolInfo of toolsToRun) {
        const lastChangedDate = getLastModifiedTime(toolInfo.directory);
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

    const shadersDir = path.join(rootDir, "Source/Graphics/Shaders/Source");
    const shaderCompiler = path.join(buildOutputDir, "ShaderCompiler.exe");

    const contentDir = path.join(rootDir, "Games/GiftOfTheSanctum/Content");
    const incineratorCompiler = path.join(buildOutputDir, "Incinerator.exe");

    const engineToolsToRun = [
        { directory: shadersDir, tool: shaderCompiler },
    ];
    const gameToolsToRun = [
        { directory: contentDir, tool: `${incineratorCompiler} --game Games/GiftOfTheSanctum` }
    ];
    const toolsToRun = isGame ? gameToolsToRun : engineToolsToRun;
    console.log("Running checks for changed content...");
    runToolsIfNeeded(toolsToRun);
};
try {
    main();
}
catch (err) {
    console.error(err);
    process.exit(1);
}