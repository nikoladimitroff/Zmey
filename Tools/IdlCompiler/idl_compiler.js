const assert = require("assert");
const fs = require("fs");
const path = require("path");
const os = require("os");

const RegexLibrary = {
    // Matches everything between the interface keyword and the closing }
    // Group 1 is the name of the interface
    Interface: /\s*interface\s+(\w+)\s\{[\s\S]*?\}\s*/g,
    Attribute: /attribute\s+(\w+)\s+(\w+);/g,
    // The following akward expression is repeated inside both method and ctor:
    // \(((:?(:?\s*\w+\s+\w+,)*(:?\s*\w+\s+\w+))?)\)
    // This matches all (type1 name1, type2 name2...) including the empty ()
    Method: /(\w+&?)\s+(\w+)\(((:?(:?\s*\w+\s+\w+,)*(:?\s*\w+\s+\w+))?)\);/g,
    // Matches constructor(args..); Group 1 is args
    Constructor: /constructor\(((:?(:?\s*\w+\s+\w+,)*(:?\s*\w+\s+\w+))?)\);/g
};

class ChakraGlueGenerator {
    generateDefinitionForArg(arg) {
        switch (arg.type) {
            case "string":
                return `stl::string _${arg.name};`;
            case "float":
            case "double":
            case "int":
            default:
                return `${arg.type} _${arg.name};`;
        }
    }
    generateParserForArg(arg) {
        let code = null;
        switch (arg.type) {
            case "float":
            case "double":
                code = `JSNumberToDouble(arguments[${arg.index}], &_${arg.name});`;
                return code;
            case "int":
                code = `JSNumberToInt(arguments[${arg.index}], &_${arg.name});`;
                return code;
            case "string":
                code = `
	JsValueRef _${arg.name}stringValue;
	JsConvertValueToString(arguments[${arg.index}], &_${arg.name}stringValue);
	const wchar_t* _${arg.name}string;
	size_t _${arg.name}Stringlength;
	JsStringToPointer(_${arg.name}stringValue, _${arg.name}string, &_${arg.name}Stringlength);
	stl::string _${arg.name} = WStringToUtf8(_${arg.name}String, _${arg.name}Stringlength);
`;
                return code;
            case "long":
            default:
                assert(false, "Unsupported type: " + arg.type);
        }
    }
    generateParserForArgList(argList) {
        return argList.map(arg => {
            console.log(arg);
            const code =`
	${this.generateDefinitionForArg(arg)}
	${this.generateParserForArg(arg)}
`;
            return code;
        }).join(os.EOL);
    }

    generateListForArgList(argList) {
        return argList.map(arg => {
            return `_${arg.name}`;
        }).join(",");
    }
    generateConstructor(interfaceName, argList) {
        const argParsingCode = this.generateParserForArgList(argList);
        const argConstructorCode = this.generateListForArgList(argList);
        const cppCode =
`
JsValueRef CALLBACK JS${interfaceName}Constructor(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	ASSERT(isConstructCall && argumentCount == ${argList.length});
	JsValueRef output = JS_INVALID_REFERENCE;
	${argParsingCode}
	${interfaceName}* object = new ${interfaceName}(${argConstructorCode});
	JsCreateExternalObject(object, nullptr, &output);
	JsSetPrototype(output, JSPointPrototype);
	return output;
}
`
        return cppCode;
    }

    generateMethod(interfaceName, methodName, argList) {
        const argParsingCode = this.generateParserForArgList(argList);
        const argListCode = this.generateListForArgList(argList);
        const cppCode =
`
JsValueRef CALLBACK JS${interfaceName}${methodName}(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == ${argList.length});
	JsValueRef output = JS_INVALID_REFERENCE;
	void* object;
	if (JsGetExternalData(arguments[0], &object) != JsNoError) {
		return output;
	}
	${interfaceName}* ptr = static_cast<${interfaceName}*>(object);
	${argParsingCode}
	ptr->${methodName}(${argListCode});

	return output;
}
`
        return cppCode;
    }

    generateProperty(interfaceName, type, name) {
        return "";
    }

}

class IdlCompiler {
    constructor(srcDir, destDir) {
        this.srcDir = srcDir;
        this.destDir = destDir;
        this.generator = new ChakraGlueGenerator();
    }
    compile() {
        fs.readdir(this.srcDir, this.compileFiles.bind(this));
    }
    compileFiles(error, files) {
        for (let file of files) {
            let fullPath = path.join(this.srcDir, file);
            fs.readFile(fullPath, 'utf8', this.compileSingleFile.bind(this));
        }
    }
    compileSingleFile(error, fileContents) {
        while (RegexLibrary.Interface.lastIndex != fileContents.length) {
            let interfaceMatch = RegexLibrary.Interface.exec(fileContents);
            if (!interfaceMatch) {
                console.error("Couldn't find an interface in file!");
                return;
            }
            const interfaceName = interfaceMatch[1];
            const interfaceBodyStart = interfaceMatch[0].indexOf("{") + 1;
            const interfaceBodyEnd = interfaceMatch[0].indexOf("}");
            const interfaceBody = interfaceMatch[0].substring(interfaceBodyStart, interfaceBodyEnd);
            this.compileSingleInterface(interfaceName, interfaceBody);
        }
    }
    compileSingleInterface(interfaceName, interfaceBody) {
        let finalGlueCode = "";
        while (interfaceBody.length != 0) {
            let [glueCode, lastIndex] = this.parseFirstStatement(interfaceName, interfaceBody);
            interfaceBody = interfaceBody.substring(lastIndex);
            finalGlueCode += glueCode;
        }
        console.log(finalGlueCode);
        fs.writeFile(path.join(this.destDir, interfaceName + ".cpp"), finalGlueCode);
    }
    parseFirstStatement(interfaceName, body) {
        let match = null;
        let glueCode = "";
        let lastIndex = body.length;
        if (match = RegexLibrary.Constructor.exec(body)) {
            const args = this.parseArgs(match[1]);
            glueCode = this.generator.generateConstructor(interfaceName, args);
            lastIndex = RegexLibrary.Constructor.lastIndex;
            RegexLibrary.Constructor.lastIndex = 0;
        }
        if (match = RegexLibrary.Method.exec(body)) {
            /* const returnType = match[1]; unused*/
            const method = match[2];
            const args = this.parseArgs(match[3]);
            glueCode = this.generator.generateMethod(interfaceName, method, args);
            lastIndex = RegexLibrary.Constructor.lastIndex;
            RegexLibrary.Constructor.lastIndex = 0;
        }
        if (match = RegexLibrary.Attribute.exec(body)) {
            const type = match[1];
            const name = match[2];
            glueCode = this.generator.generateProperty(interfaceName, type, name);
            lastIndex = RegexLibrary.Constructor.lastIndex;
            RegexLibrary.Constructor.lastIndex = 0;
        }
        return [glueCode, lastIndex];
    }
    parseArgs(argsAsString) {
        return argsAsString.split(/,/g)
            .map(s => s.trim())
            .filter(s => s.length)
            .map((arg, i) => ({ type: arg.split(" ")[0], name: arg.split(" ")[1], index: i }));
    }
}

let main = function () {
    const idlSourceDir = __dirname + "/../../Source/Scripting/idl";
    const idlDestDir = __dirname + "/../../Source/Scripting/ScriptingGlue/";
    let compiler = new IdlCompiler(idlSourceDir, idlDestDir);
    compiler.compile();
}
main();