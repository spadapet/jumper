#include "pch.h"
#include "String/StringUtil.h"
#include "UI/Components.h"
#include "UI/DebugControl.h"

static Noesis::BaseComponent* NullComponentCreator(Noesis::Symbol symbol8)
{
	ff::String symbol = ff::StringFromUTF8(symbol8.GetStr());
	ff::String message = ff::String::format_new(L"Can't create type in XAML: %s", symbol.c_str());
	assertSz(false, message.c_str());

	return nullptr;
}

void RegisterNoesisComponents()
{
	// Creatable
	NsRegisterComponent<DebugControl>();
	NsRegisterComponent<ValueHolderTemplateSelector>();

	// Uncreatable
	Noesis::RegisterComponent(Noesis::TypeOf<BoolHolder>(), ::NullComponentCreator);
	Noesis::RegisterComponent(Noesis::TypeOf<EventHolder>(), ::NullComponentCreator);
	Noesis::RegisterComponent(Noesis::TypeOf<PropertyCategory>(), ::NullComponentCreator);
	Noesis::RegisterComponent(Noesis::TypeOf<ValueHolder>(), ::NullComponentCreator);
}
