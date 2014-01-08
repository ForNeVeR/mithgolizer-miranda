#pragma once

#include <vector>

#include <Windows.h>

#include <newpluginapi.h>

#include "BanInfo.hpp"

namespace Mithgolizer
{
	class PluginCore
	{
	public:
		PluginCore(const PLUGININFOEX &pluginInfo);

		void Initialize(HINSTANCE moduleInstance);
		void Deinitialize();

	private:
		void InitializeLangpack();
		void InitializeMainMenu();

		void BanUser();
		void BanUser(const BanInfo &banInfo, HANDLE conference);

		std::vector<HANDLE> GetActiveConferences();

		const PLUGININFOEX &_pluginInfo;
		HINSTANCE _moduleInstance;
	};
}