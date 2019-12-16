#define NOMINMAX
#include <Windows.h>

#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"

#include "hooks.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "render.hpp"

DWORD WINAPI OnDllAttach(LPVOID base)
{ 
    if(Utils::WaitForModules(15000, { L"client_panorama.dll", L"engine.dll", L"shaderapidx9.dll" }) == WAIT_TIMEOUT) {
        return FALSE;
    }

#ifdef _DEBUG
    Utils::AttachConsole();
#endif

    try {
        Utils::ConsolePrint("Starting up...\n");

        Interfaces::Initialize();
        Interfaces::Dump();

        NetvarSys::Get().Initialize();

        InputSys::Get().Initialize();

		Render::Get().Initialize();

        Menu::Get().Initialize();

        Hooks::Initialize();

        InputSys::Get().RegisterHotkey(VK_DELETE, [base]() {
            g_Unload = true;
        });

        InputSys::Get().RegisterHotkey('Q', [base]() {
            Menu::Get().Toggle();
        });


		InputSys::Get().RegisterHotkey('Z', [base]() {
			if (!g_VGuiSurface->IsCursorVisible())
			g_Options.fakelag_enabled.value = std::make_shared<bool>(!g_Options.fakelag_enabled);
		});
		
		InputSys::Get().RegisterHotkey('X', [base]() {
			if (!g_Options.fakelag_enabled)
				g_Options.fakelag_enabled.value = std::make_shared<bool>(true);
			if (g_Options.fakelag != .2f)
				g_Options.fakelag.value = std::make_shared<float>(.2f);
		});

		InputSys::Get().RegisterHotkey('C', [base]() {
			if (!g_Options.fakelag_enabled)
				g_Options.fakelag_enabled.value = std::make_shared<bool>(true);
			if (g_Options.fakelag != .8f)
				g_Options.fakelag.value = std::make_shared<float>(.8f);

		});

		InputSys::Get().RegisterHotkey('F', [base]() {
			g_Options.esp_enabled.value = std::make_shared<bool>(!g_Options.esp_enabled);
			g_Options.chams_arms_enabled.value = std::make_shared<bool>(!g_Options.chams_arms_enabled);
		});

        Utils::ConsolePrint("Finished.\n");

        while(!g_Unload)
            Sleep(1000);

        g_CVar->FindVar("crosshair")->SetValue(true);

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);

    } catch(const std::exception& ex) {
        Utils::ConsolePrint("An error occured during startup:\n");
        Utils::ConsolePrint("%s\n", ex.what());
        Utils::DetachConsole();

        FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
    }
}

BOOL WINAPI OnDllDetach()
{
#ifdef _DEBUG
    Utils::DetachConsole();
#endif
	Menu::Get().Shutdown();
    Hooks::Shutdown();
    return TRUE;
}

BOOL WINAPI DllMain( _In_ HINSTANCE hinstDll, _In_ DWORD fdwReason, _In_opt_ LPVOID lpvReserved){
    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDll);
            CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);
            return TRUE;
        case DLL_PROCESS_DETACH:
			if (lpvReserved == nullptr) {
				return OnDllDetach();
			}
            return TRUE;
        default:
            return TRUE;
    }
}
