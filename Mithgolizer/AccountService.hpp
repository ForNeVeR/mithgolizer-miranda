#pragma once

#include <vector>
#include <string>

#include <Windows.h>

#include "BanInfo.hpp"

namespace Mithgolizer
{
	class PluginCore;

	class AccountService
	{
	public:
		AccountService(const PluginCore &core, const std::string &accountName);
		AccountService(const AccountService &) = delete;
		~AccountService();

	private:
		void InitializeMenu();

		void BanUser();
		void BanUser(const BanInfo &banInfo, HANDLE conference);

		std::vector<HANDLE> GetActiveConferences();

		const PluginCore &_core;
		const std::string _accountName;
	};
}

