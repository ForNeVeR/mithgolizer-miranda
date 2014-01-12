#include "PluginCore.hpp"

#include <m_system.h>
#include <m_langpack.h>

using namespace std;
using namespace Mithgolizer;

const auto JabberProtocolName = string("JABBER");

PluginCore::PluginCore(const PLUGININFOEX &pluginInfo)
	: _pluginInfo(pluginInfo)
{
}

void PluginCore::Initialize(HINSTANCE moduleInstance)
{
	_moduleInstance = moduleInstance;

	InitializeLangpack();

	auto hook = [](void *corePointer, WPARAM, LPARAM) -> int
	{
		auto core = static_cast<PluginCore*>(corePointer);
		core->InitializeServices();

		return 0;
	};
	
	HookEventObj(ME_SYSTEM_MODULESLOADED, hook, this);
}

void PluginCore::Deinitialize()
{
	// TODO: Destroy services.
}

HINSTANCE PluginCore::ModuleInstance() const
{
	return _moduleInstance;
}

void PluginCore::InitializeLangpack()
{
	mir_getLP(&_pluginInfo);
}

void PluginCore::InitializeServices()
{
	PROTOACCOUNT **accounts = nullptr;
	auto accountCount = 0;
	ProtoEnumAccounts(&accountCount, &accounts);

	for (auto index = 0; index < accountCount; ++index)
	{
		auto &account = *accounts[index];
		auto protocol = string(account.szProtoName);

		if (protocol == JabberProtocolName)
		{
			InitializeAccountService(account);
		}
	}	
}

void PluginCore::InitializeAccountService(const PROTOACCOUNT &account)
{
	auto accountName = std::string(account.szModuleName);
	_accountServices.emplace_back(make_unique<AccountService>(*this, accountName));
}