const os = require("os");

function getTypeEnumOfAnyType(anyType) {
    const match = /anyof<([\w:]+)>/.exec(anyType);
    if (match) {
        return match[1];
    }
    return null;
}

class ChakraGlueGenerator {
    generateCastingOperatorForType(type) {
        switch (type) {
            case "string":
                return `(stl::string);`;
            case "float":
                return `(float)`;
            case "double":
                return `(double)`;
            case "int":
                return `(int)`;
            default:
                return `(${type}*)`;
        }
    }
    generateCastingOperatorForTypeOrAny(type) {
        const match = getTypeEnumOfAnyType(type);
        if (match) {
            return "(void*)";
        }
        return this.generateCastingOperatorForType(type);
    }
    generateDefinitionForArg(arg) {
        switch (arg.type) {
            case "string":
                return `stl::string _${arg.name};`;
            case "float":
            case "double":
                return `double _${arg.name};`;
            case "int":
                return `${arg.type} _${arg.name};`;
            default:
                return `${arg.type}* _${arg.name};`;
        }
    }
    generateDefinitionForArgOrAny(arg) {
        const typeEnumeration = getTypeEnumOfAnyType(arg.type);
        if (typeEnumeration) {
            return `void* _${arg.name};`;
        }
        return this.generateDefinitionForArg(arg);
    }
    generateParserForArg(arg) {
        let code = null;
        const actualArgIndex = arg.index + 1; // arg[0] is always this
        switch (arg.type) {
            case "float":
            case "double":
                code = `JsNumberToDouble(arguments[${actualArgIndex}], &_${arg.name});`;
                return code;
            case "int":
                code = `JsNumberToInt(arguments[${actualArgIndex}], &_${arg.name});`;
                return code;
            case "string":
                code = `
	JsValueRef _${arg.name}stringValue;
	JsConvertValueToString(arguments[${actualArgIndex}], &_${arg.name}stringValue);
	const wchar_t* _${arg.name}string;
	size_t _${arg.name}Stringlength;
	JsStringToPointer(_${arg.name}stringValue, _${arg.name}string, &_${arg.name}Stringlength);
	stl::string _${arg.name} = ConvertWideStringToUtf8(_${arg.name}String, _${arg.name}Stringlength);
`;
                return code;
            case "long":
                assert(false, "Unsupported type: " + arg.type);
            default:
                code = `JsGetExternalData(arguments[${actualArgIndex}], &_${arg.name});`
                return code;
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

    generateOutputForType(type) {
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
                assert(false, "Unsupported type: " + type);
            default:
                const uniqueInterfaceName = Common.convertQualifiedToUniqueTypename(type);
                code = `
	JsCreateExternalObject(_result, nullptr, &output);
	JsSetPrototype(output, Js${uniqueInterfaceName}Prototype);
`;
                return code;
        }
    }
    generateOutputForTypeOrAny(type, indexArg) {
        const typeEnumeration = getTypeEnumOfAnyType(type);
        if (typeEnumeration) {
            const code = `
	JsCreateExternalObject(_result, nullptr, &output);
	JsSetPrototype(output, Zmey::Chakra::Binding::GetProtototypeOfAnyTypeSet(Zmey::Hash("${typeEnumeration}"), _${indexArg.name}));
`;
            return code;
        }
        return this.generateOutputForType(type);
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
	assert(isConstructCall && argumentCount == ${argList.length + 1});
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
    generateCallCode(returnType, methodName, argList) {
        const argListCode = this.generateListForArgList(argList);
        const callMethod = `ptr->${methodName}(${argListCode});`;
        if (returnType == "void") {
            return callMethod;
        }
        const returnArg = {type: returnType, name: "result"};
        const cppCode =
`
	${this.generateDefinitionForArgOrAny(returnArg)}
	_result = ${this.generateCastingOperatorForTypeOrAny(returnType)}${callMethod}
	${this.generateOutputForTypeOrAny(returnType, argList[0])}
`;
        return cppCode;
    }

    generateMethod(qualifiedName, uniqueInterfaceName, returnType, methodName, argList, attributes) {
        const argParsingCode = this.generateParserForArgList(argList);
        const cppMethodName = attributes.nameAsIs ? methodName : methodName[0].toUpperCase() + methodName.slice(1);
        const callCode = this.generateCallCode(returnType, cppMethodName, argList);
        const cppCode =
`
JsValueRef CALLBACK Js${uniqueInterfaceName}${methodName}(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == ${argList.length + 1});
	JsValueRef output = JS_INVALID_REFERENCE;
	void* object;
	if (JsGetExternalData(arguments[0], &object) != JsNoError) {
		return output;
	}
	${qualifiedName}* ptr = static_cast<${qualifiedName}*>(object);
	${argParsingCode}
	${callCode}

	return output;
}
`
        return cppCode;
    }

    generateProperty(qualifiedName, uniqueInterfaceName, type, name, propertyAttributes) {
        const cppPropName = propertyAttributes.nameAsIs ? name : name[0].toUpperCase() + name.slice(1);
        let cppCode =
`
JsValueRef CALLBACK Js${uniqueInterfaceName}${name}Getter(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 1);
	JsValueRef output = JS_INVALID_REFERENCE;
	void* object;
	if (JsGetExternalData(arguments[0], &object) != JsNoError) {
		return output;
	}
	${qualifiedName}* ptr = static_cast<${qualifiedName}*>(object);
	${type} _result = ptr->${cppPropName};
	${this.generateOutputForType(type)}

	return output;
}
`;
        if (propertyAttributes.isReadonly) {
            return cppCode;
        }
        const arg = { type: type, name: name, index: 0 };
        cppCode +=
`
JsValueRef CALLBACK Js${uniqueInterfaceName}${name}Setter(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 2);
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
    generateProjection(qualifiedName, uniqueInterfaceName, hasConstructor, methodList) {
        const shortName = qualifiedName.substring(qualifiedName.lastIndexOf("::") + 2);
        const constructorName = hasConstructor ? `&Js${uniqueInterfaceName}Constructor` : "nullptr";
        if (methodList.length == 0) {
            // No methods, shortcut
                const cppCode =
`
Zmey::Chakra::Binding::AutoNativeClassProjecter ${uniqueInterfaceName}Projector(
	L"${shortName}",
	${constructorName},
	Js${uniqueInterfaceName}Prototype
);
`;
            return cppCode;
        }

        const initializerListForNames = methodList.map(m => `L"${m}"`).join(",");
        const initializerListForFuncs = methodList.map(m => `&Js${uniqueInterfaceName}${m}`).join(",");
        const cppCode =
`
const wchar_t* ${uniqueInterfaceName}MemberNames[] = {${initializerListForNames}};
const JsNativeFunction ${uniqueInterfaceName}MemberFuncs[] = {${initializerListForFuncs}};
Zmey::Chakra::Binding::AutoNativeClassProjecter ${uniqueInterfaceName}Projector(
	L"${shortName}",
	${constructorName},
	Js${uniqueInterfaceName}Prototype,
	${methodList.length},
	${uniqueInterfaceName}MemberNames,
	${uniqueInterfaceName}MemberFuncs
);
`;
        return cppCode
    }
}

module.exports = ChakraGlueGenerator;
