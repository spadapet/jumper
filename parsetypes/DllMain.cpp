#include "pch.h"
#include "MainUtilInclude.h"
#include "Module/Module.h"

// {94463003-D529-498C-981D-DC031BA4CD89}
static const GUID s_moduleId = { 0x94463003, 0xd529, 0x498c, { 0x98, 0x1d, 0xdc, 0x3, 0x1b, 0xa4, 0xcd, 0x89 } };
static ff::StaticString s_moduleName(L"parsetypes");
static ff::ModuleFactory RegisterThisModule(s_moduleName, s_moduleId, ff::GetDelayLoadInstance, ff::GetModuleStartup);

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		ff::SetThisModule(s_moduleName, s_moduleId, instance);
	}

	return TRUE;
}
