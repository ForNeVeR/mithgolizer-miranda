#include "PluginCore.hpp"

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	"Mothgolizer",
	PLUGIN_MAKE_VERSION(0, 0, 0, 1),
	"Mass ban plugin.",
	"F. von Never",
	"neverthness@gmail.com",
	"© 2014 F. von Never",
	"https://github.com/ForNeVeR",
	UNICODE_AWARE,
	{ 0x1db515f2, 0x11c0, 0x4f5d, { 0x9f, 0xf4, 0xba, 0xff, 0xaa, 0x92, 0xc9, 0x5f } } // {1DB515F2-11C0-4F5D-9FF4-BAFFAA92C95F}
};

auto Core = Mithgolizer::PluginCore(pluginInfo);

HINSTANCE libraryInstance = nullptr;

int hLangpack;

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	libraryInstance = instance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID interfaces[] = { MIID_LAST };

extern "C" __declspec(dllexport) int Load()
{
	Core.Initialize();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	Core.Deinitialize();
	return 0;
}
