let fs = require("fs");
let path = require("path");

let RegexLibrary = {
    Interface: /interface\s+(\w+)/g,
    Attribute: /attribute\s+(\w+)\s+(\w+)/g,
    Function: /(\w+&?)\s+(\w+)\(((:?\w+\s+\w+)*)\)/g,
    Constructor: /constructor\(((:?\w+\s+\w+)*)\)/g
}

class ChakraGlueGenerator {
    generateConstructor(interfaceName, argList) {
        const argParsingCode = "";
        const argConstructorCode = "";
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
}

class IdlCompiler {
    constructor(srcDir, destDir) {
        this.srcDir = srcDir;
        this.destDir = destDir;
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
        for (let regexName in RegexLibrary) {
            let regex = RegexLibrary[regexName];
            console.log(regex.exec(fileContents));
        }
        const generator = new ChakraGlueGenerator();
        console.log(generator.generateConstructor("World", []));
    }
}

let main = function () {
    const idlSourceDir = __dirname + "/../../Source/Scripting/idl";
    const idlDestDir = __dirname + "/../../Source/Scripting/ScriptingGlue/";
    let compiler = new IdlCompiler(idlSourceDir, idlDestDir);
    compiler.compile();
}
main();