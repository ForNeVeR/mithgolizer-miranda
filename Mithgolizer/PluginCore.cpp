#include "PluginCore.hpp"

#include <m_clist.h>
#include <m_langpack.h>
#include <m_skin.h>

#include "InputDialog.hpp"

using namespace Mithgolizer;

const auto BanServiceName = "Mithgolizer/Ban";

PluginCore::PluginCore(const PLUGININFOEX &pluginInfo)
	: _pluginInfo(pluginInfo)
{
}

void PluginCore::Initialize(HINSTANCE moduleInstance)
{
	_moduleInstance = moduleInstance;

	InitializeLangpack();
	InitializeMainMenu();
}

void PluginCore::Deinitialize()
{
}

void PluginCore::InitializeLangpack()
{
	mir_getLP(&_pluginInfo);
}

void PluginCore::InitializeMainMenu()
{
	auto banService = [](void *corePointer, LPARAM param1, LPARAM param2) -> INT_PTR
	{
		auto core = static_cast<PluginCore*>(corePointer);
		core->BanUser();
		return 0;
	};

	CreateServiceFunctionObj(BanServiceName, banService, this);

	auto mi = CLISTMENUITEM();
	mi.cbSize = sizeof(mi);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_UNICODE;
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName = LPGENW("&Ban User Everywhere");
	mi.pszService = const_cast<char*>(BanServiceName);

	Menu_AddMainMenuItem(&mi);
}

void PluginCore::BanUser()
{
	auto banInfo = InputDialog::ShowDialog(_moduleInstance);
	if (!banInfo.Jid.empty())
	{
		// TODO: Ban user.
	}
}