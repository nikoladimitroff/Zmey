const assert = require("assert");
const fs = require("fs");
const path = require("path");
const os = require("os");

const RegexLibrary = {
    // Matches everything between the interface keyword and the closing }
    // Group 1 is the name of the interface
    ExtraHeaders: /#include <.+>/g,
    Interface: /\s*interface\s+([\w:]+)\s\{[\s\S]*?\}\s*/g,
    Attribute: /(?:readonly )?attribute\s+(\w+)\s+(\w+);/g,
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
                return `double _${arg.name};`;
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
                code = `JsNumberToDouble(arguments[${arg.index}], &_${arg.name});`;
                return code;
            case "int":
                code = `JsNumberToInt(arguments[${arg.index}], &_${arg.name});`;
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
            const code =`
	${this.generateDefinitionForArg(arg)}
	${this.generateParserForArg(arg)}
`;
            return code;
        }).join(os.EOL);
    }

    generateOutputForArg(type) {
        let code = null;
        switch (type) {
            case "float":
            case "double":
                code = `JsDoubleToNumber((double)_result, &output);`;
                return code;
            case "int":
                code = `JsIntToNumber(_result, &output);`;
                return code;
            case "string":
                code = `
	const wchar_t* _resultWString;
	size_t _resultStringlength;
	Utf8ToWString(result, _resultWString, _resultStringlength);
	JsPointerToString(_resultWString, _resultStringlength, &output);
`;
                return code;
            case "long":
            default:
                assert(false, "Unsupported type: " + type);
        }
    }

    generateListForArgList(argList) {
        return argList.map(arg => {
            return `(${arg.type})_${arg.name}`;
        }).join(",");
    }
    generateHeader() {
        const cppHeaders =
`
#include <cassert>
#include <ChakraCore/ChakraCore.h>
#include <Zmey/Scripting/Binding.h>
`;
        return cppHeaders;
    }
    generateConstructor(qualifiedName, uniqueInterfaceName, argList) {
        const argParsingCode = this.generateParserForArgList(argList);
        const argConstructorCode = this.generateListForArgList(argList);
        const cppCode =
`
JsValueRef CALLBACK Js${uniqueInterfaceName}Constructor(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(isConstructCall && argumentCount == ${argList.length});
	JsValueRef output = JS_INVALID_REFERENCE;
	${argParsingCode}
	${qualifiedName}* object = new ${qualifiedName}(${argConstructorCode});
	JsCreateExternalObject(object, nullptr, &output);
	JsSetPrototype(output, Js${uniqueInterfaceName}Prototype);
	Js${uniqueInterfaceName}DefineProperties(output);
	return output;
}
`
        return cppCode;
    }

    generateMethod(qualifiedName, uniqueInterfaceName, methodName, argList) {
        const argParsingCode = this.generateParserForArgList(argList);
        const argListCode = this.generateListForArgList(argList);
        const cppMethodName = methodName[0].toUpperCase() + methodName.slice(1);
        const cppCode =
`
JsValueRef CALLBACK Js${uniqueInterfaceName}${methodName}(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == ${argList.length});
	JsValueRef output = JS_INVALID_REFERENCE;
	void* object;
	if (JsGetExternalData(arguments[0], &object) != JsNoError) {
		return output;
	}
	${qualifiedName}* ptr = static_cast<${qualifiedName}*>(object);
	${argParsingCode}
	ptr->${cppMethodName}(${argListCode});

	return output;
}
`
        return cppCode;
    }

    generateProperty(qualifiedName, uniqueInterfaceName, type, name, isReadonly) {
        const cppPropName = name[0].toUpperCase() + name.slice(1);
        let cppCode =
`
JsValueRef CALLBACK Js${uniqueInterfaceName}${name}Getter(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 0);
	JsValueRef output = JS_INVALID_REFERENCE;
	void* object;
	if (JsGetExternalData(arguments[0], &object) != JsNoError) {
		return output;
	}
	${qualifiedName}* ptr = static_cast<${qualifiedName}*>(object);
	${type} _result = ptr->${cppPropName};
	${this.generateOutputForArg(type)}

	return output;
}
`;
        if (isReadonly) {
            return cppCode;
        }
        const arg = { type: type, name: name, index: 0 };
        cppCode +=
`
JsValueRef CALLBACK Js${uniqueInterfaceName}${name}Setter(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 1);
	JsValueRef output = JS_INVALID_REFERENCE;
	void* object;
	if (JsGetExternalData(arguments[0], &object) != JsNoError) {
		return output;
	}
	${qualifiedName}* ptr = static_cast<${qualifiedName}*>(object);
	${this.generateDefinitionForArg(arg)}
	${this.generateParserForArg(arg)}
	ptr->${cppPropName} = _${arg.name};
	return output;
}
`;
        return cppCode;
    }
    generatePropertiesSetup(uniqueInterfaceName, propertyList) {
        const propertiesGeneratorCode = propertyList.map(p => {
            const getterName = `Js${uniqueInterfaceName}${p.name}Getter`;
            const setterName = `Js${uniqueInterfaceName}${p.name}Setter`;
            if (p.isReadonly) {
                return `Zmey::Chakra::Binding::DefineProperty(object, L"${p.name}", &${getterName});`;
            }
            return `Zmey::Chakra::Binding::DefineProperty(object, L"${p.name}", &${getterName}, &${setterName});`;
        }).join(os.EOL + "\t");
        const cppCode =
`
void Js${uniqueInterfaceName}DefineProperties(JsValueRef object)
{
	${propertiesGeneratorCode}
}
`;
        return cppCode;
    }

    generatePrototypeDefinition(uniqueInterfaceName) {
        return `JsValueRef Js${uniqueInterfaceName}Prototype;`;
    }
    generateProjection(qualifiedName, uniqueInterfaceName, methodList) {
        const shortName = qualifiedName.substring(qualifiedName.lastIndexOf("::") + 2);
        if (methodList.length == 0) {
            // No methods, shortcut
                const cppCode =
`
Zmey::Chakra::Binding::AutoNativeClassProjecter ${uniqueInterfaceName}Projector(
	L"${shortName}",
	&Js${uniqueInterfaceName}Constructor,
	Js${uniqueInterfaceName}Prototype
);
`;
            return cppCode;
        }

        const initializerListForNames = methodList.map(m => `L"${m}"`).join(",");
        const initializerListForFuncs = methodList.map(m => `&Js${uniqueInterfaceName}${m}`).join(",");
        const cppCode =
`
Zmey::Chakra::Binding::AutoNativeClassProjecter ${uniqueInterfaceName}Projector(
	L"${shortName}",
	&Js${uniqueInterfaceName}Constructor,
	Js${uniqueInterfaceName}Prototype,
	const wchar_t* []({${initializerListForNames}}),
	const JsNativeFunction []({${initializerListForFuncs}})
);
`;
        return cppCode
    }

}

class IdlCompiler {
    constructor(srcDir, destDir) {
        this.srcDir = srcDir;
        this.destDir = destDir;
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
            const uniqueInterfaceName = qualifiedName.replace(/::/g, "");
            const interfaceBodyStart = interfaceMatch[0].indexOf("{") + 1;
            const interfaceBodyEnd = interfaceMatch[0].indexOf("}");
            const interfaceBody = interfaceMatch[0].substring(interfaceBodyStart, interfaceBodyEnd);

            this.constructorCode = null;
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
            /* const returnType = match[1]; unused*/
            const method = match[2];
            const args = this.parseArgs(match[3]);
            glueCode = this.generator.generateMethod(qualifiedName, uniqueInterfaceName, method, args);
            this.methodList.push(method);
            lastIndex = RegexLibrary.Method.lastIndex;
            RegexLibrary.Method.lastIndex = 0;
        }
        else if (match = RegexLibrary.Attribute.exec(body)) {
            const type = match[1];
            const name = match[2];
            const isReadonly = /\breadonly\b/.test(match[0]);
            glueCode = this.generator.generateProperty(qualifiedName, uniqueInterfaceName, type, name, isReadonly);
            this.propertyList.push({name: name, isReadonly: isReadonly});
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