#pragma once

#include <vector>
#include <string>
#include <memory>

#include <Windows.h>

#include <newpluginapi.h>
#include <m_protocols.h>

#include "AccountService.hpp"

namespace Mithgolizer
{
	class PluginCore
	{
	public:
		PluginCore(const PLUGININFOEX &pluginInfo);

		void Initialize(HINSTANCE moduleInstance);
		void Deinitialize();

		HINSTANCE ModuleInstance() const;

	private:
		void InitializeLangpack();
		void InitializeServices();
		void InitializeAccountService(const PROTOACCOUNT &account);

		const PLUGININFOEX &_pluginInfo;
		HINSTANCE _moduleInstance;

		std::vector<std::unique_ptr<AccountService>> _accountServices;
	};
}