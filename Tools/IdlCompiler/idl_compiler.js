const assert = require("assert");
const fs = require("fs");
const path = require("path");
const os = require("os");

const ChakraGlueGenerator = require("./chakra_generator");

const RegexLibrary = {
    // Matches everything between the interface keyword and the closing }
    // Group 1 is the name of the interface
    ExtraHeaders: /#include <.+>/g,
    Interface: /\s*interface\s+([\w:]+)\s\{[\s\S]*?\}\s*/g,
    Attribute: /^(?:nameasis )?(?:readonly )?attribute\s+([\w:]+)\s+(\w+);/g,
    // The following akward expression is repeated inside both method and ctor:
    // \(((:?(:?\s*[\w:]+\s+\w+,)*(:?\s*[\w:]+\s+\w+))?)\)
    // This matches all (type1 name1, type2 name2...) including the empty ()
    Method: /^(?:nameasis )?([\w:]+&?|anyof<[\w:]+>)\s+(\w+)\(((:?(:?\s*[\w:]+\s+\w+,)*(:?\s*[\w:]+\s+\w+))?)\);/g,
    // Matches constructor(args..); Group 1 is args
    Constructor: /^constructor\(((:?(:?\s*[\w:]+\s+\w+,)*(:?\s*[\w:]+\s+\w+))?)\);/g
};

const Common = {
    convertQualifiedToUniqueTypename(qualifiedName) {
        return qualifiedName.replace(/::/g, "");
    }
};

class IdlCompiler {
    constructor(srcDir, destDir) {
        this.srcDir = srcDir;
        this.destDir = destDir;
        if (!fs.existsSync(destDir)){
            fs.mkdirSync(destDir);
        }
        this.generator = new ChakraGlueGenerator();
        // We need to save the ctor, properties and methods as we need to rearrange them in the glue code
        this.constructorCode = null;
        this.propertyList = []; // {name, type, isReadonly}
        this.methodList = []; // name as string
    }
    compile() {
        fs.readdir(this.srcDir, this.compileFiles.bind(this));
    }
    compileFiles(error, files) {
        for (let file of files) {
            let fullPath = path.join(this.srcDir, file);
            let destinationPath = path.join(this.destDir, file.replace(".idl", ".cpp"));
            fs.readFile(fullPath, 'utf8', this.compileSingleFile.bind(this, destinationPath));
        }
    }
    compileSingleFile(destinationFile, error, fileContents) {
        RegexLibrary.Interface.lastIndex = 0;
        RegexLibrary.ExtraHeaders.lastIndex = 0;

        let headersMatch = null;
        let extraHeaders = this.generator.generateHeader();
        while (headersMatch = RegexLibrary.ExtraHeaders.exec(fileContents)) {
            extraHeaders += headersMatch[0] + os.EOL;
        }
        fileContents = fileContents.substring(RegexLibrary.ExtraHeaders.lastIndex).trim();

        let interfaceCode = [];
        while (RegexLibrary.Interface.lastIndex != fileContents.length) {
            let interfaceMatch = RegexLibrary.Interface.exec(fileContents);
            if (!interfaceMatch) {
                console.error("Couldn't find an interface in file!");
                return;
            }
            const qualifiedName = interfaceMatch[1];
            const uniqueInterfaceName = Common.convertQualifiedToUniqueTypename(qualifiedName);
            const interfaceBodyStart = interfaceMatch[0].indexOf("{") + 1;
            const interfaceBodyEnd = interfaceMatch[0].indexOf("}");
            const interfaceBody = interfaceMatch[0].substring(interfaceBodyStart, interfaceBodyEnd).trim();

            this.constructorCode = "";
            this.propertyList = [];
            this.methodList = [];
            const code = this.compileSingleInterface(qualifiedName, uniqueInterfaceName, interfaceBody);
            const propertiesSetup = this.generator.generatePropertiesSetup(uniqueInterfaceName, this.propertyList);
            const projector = this.generator.generateProjection(qualifiedName, uniqueInterfaceName, this.methodList);
            const prototype = this.generator.generatePrototypeDefinition(uniqueInterfaceName);
            interfaceCode.push(code + propertiesSetup + prototype + this.constructorCode + projector);
        }

        const openningBrace =
`
namespace
{
`;
        const closingBrace =
`
}
`;
        const finalGlueCode = extraHeaders + openningBrace + interfaceCode.join(os.EOL) + closingBrace;
        console.log(`Done with file ${destinationFile}.`);
        fs.writeFile(destinationFile, finalGlueCode, { flag: "w+"}, (err) => assert(!err));
    }
    compileSingleInterface(qualifiedName, uniqueInterfaceName, interfaceBody) {
        let finalGlueCode = "";
        while (interfaceBody.length != 0) {
            let [glueCode, lastIndex] = this.parseFirstStatement(qualifiedName, uniqueInterfaceName, interfaceBody);
            interfaceBody = interfaceBody.substring(lastIndex).trim();
            finalGlueCode += glueCode;
        }
        return finalGlueCode;
    }
    parseAttributes(statement) {
        return {
            isReadonly: /\breadonly\b/.test(statement),
            nameAsIs: /\bnameasis\b/.test(statement),
        };
    }
    parseFirstStatement(qualifiedName, uniqueInterfaceName, body) {
        let match = null;
        let glueCode = "";
        let lastIndex = -1;
        if (match = RegexLibrary.Constructor.exec(body)) {
            const args = this.parseArgs(match[1]);
            // Constructor code must be added last so save it for later
            this.constructorCode = this.generator.generateConstructor(qualifiedName, uniqueInterfaceName, args);
            lastIndex = RegexLibrary.Constructor.lastIndex;
            RegexLibrary.Constructor.lastIndex = 0;
        }
        else if (match = RegexLibrary.Method.exec(body)) {
            const returnType = match[1];
            const method = match[2];
            const args = this.parseArgs(match[3]);
            let attributes = this.parseAttributes(match[0]);
            glueCode = this.generator.generateMethod(qualifiedName, uniqueInterfaceName, returnType, method, args, attributes);
            this.methodList.push(method);
            lastIndex = RegexLibrary.Method.lastIndex;
            RegexLibrary.Method.lastIndex = 0;
        }
        else if (match = RegexLibrary.Attribute.exec(body)) {
            const type = match[1];
            const name = match[2];
            let propertyAttributes = this.parseAttributes(match[0]);
            glueCode = this.generator.generateProperty(qualifiedName, uniqueInterfaceName, type, name, propertyAttributes);
            this.propertyList.push({name: name, isReadonly: propertyAttributes.isReadonly});
            lastIndex = RegexLibrary.Attribute.lastIndex;
            RegexLibrary.Attribute.lastIndex = 0;
        }
        assert(lastIndex != -1, `Failed to parse file body: ${body}`);
        return [glueCode, lastIndex];
    }
    parseArgs(argsAsString) {
        return argsAsString.split(/,/g)
            .map(s => s.trim())
            .filter(s => s.length)
            .map((arg, i) => ({
                type: arg.split(" ")[0],
                name: arg.split(" ")[1],
                index: i
            }));
    }
}

let main = function () {
    const idlSourceDir = __dirname + "/../../Source/Scripting/idl";
    const idlDestDir = __dirname + "/../../Source/Scripting/ScriptingGlue/";
    let compiler = new IdlCompiler(idlSourceDir, idlDestDir);
    compiler.compile();
}
main();
