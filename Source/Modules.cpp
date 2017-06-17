#include <Zmey/Modules.h>

#include "Platform/WindowsPlatform.h" // TODO(alex):include ???
#include "Scripting/ChakraScriptEngine.h"

namespace Zmey
{
namespace Modules
{
ZMEY_API Zmey::TaskSystem<4>* TaskSystem;
ZMEY_API Zmey::IPlatform* Platform;
ZMEY_API Zmey::Graphics::RendererInterface* Renderer;
ZMEY_API Zmey::ResourceLoader* ResourceLoader;
ZMEY_API Zmey::IScriptEngine* ScriptEngine;
ZMEY_API Zmey::SettingsManager* SettingsManager;
ZMEY_API Zmey::InputController* InputController;

void Initialize()
{
	TaskSystem = StaticAlloc<Zmey::TaskSystem<4>>();

	Platform = StaticAlloc<Zmey::WindowsPlatform>();
	Renderer = StaticAlloc<Zmey::Graphics::RendererInterface>();

	ResourceLoader = StaticAlloc<Zmey::ResourceLoader>();
	ScriptEngine = StaticAlloc<Zmey::Chakra::ChakraScriptEngine>();
	SettingsManager = StaticAlloc<Zmey::SettingsManager>();
	InputController = StaticAlloc<Zmey::InputController>();
}

// Due to lack of better tooling, project modules by hand
JsValueRef CALLBACK JsGetInputController(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 1);
	JsValueRef output = JS_INVALID_REFERENCE;
	JsCreateExternalObject(Zmey::Modules::InputController, nullptr, &output);
	auto hash = Zmey::Hash(Zmey::HashHelpers::CaseInsensitiveStringWrapper("InputController"));
	JsSetPrototype(output, Zmey::Chakra::Binding::AutoNativeClassProjecter::GetPrototypeOf(hash));
	return output;
}

void ProjectToScripting()
{
	using namespace Zmey::Chakra::Binding;

	JsValueRef globalObject;
	CHECKCHAKRA(JsGetGlobalObject(&globalObject));
	JsValueRef object;
	CHECKCHAKRA(JsCreateExternalObject(nullptr, nullptr, &object));

	Zmey::Chakra::Binding::DefineProperty(object, L"inputController", &JsGetInputController);

	SetProperty(globalObject, L"Modules", object);
}

}
}