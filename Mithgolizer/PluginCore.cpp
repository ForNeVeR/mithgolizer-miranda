#include "PluginCore.hpp"

#include <array>
#include <codecvt>
#include <exception>
#include <locale>

#include <m_clist.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_jabber.h>
#include <m_langpack.h>
#include <m_skin.h>

#include "InputDialog.hpp"

using namespace std;
using namespace Mithgolizer;

const auto BanServiceName = "Mithgolizer/Ban";
const auto ChatRoomSettingName = "ChatRoom";
const auto ChatRoomIdSettingName = "ChatRoomID";
const auto MyNickSettingName = "MyNick";

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
		for (const auto conference : GetActiveConferences())
		{
			BanUser(banInfo, conference);
		}
	}
}

void PluginCore::BanUser(const BanInfo &banInfo, HANDLE conference)
{
	auto contactInfo = CONTACTINFO();
	contactInfo.hContact = conference;
	contactInfo.dwFlag = CNF_NICK | CNF_UNICODE;

	CallService(MS_CONTACT_GETCONTACTINFO, 0, reinterpret_cast<LPARAM>(&contactInfo));
	
	auto buffer = array<char, 256>();
	
	auto value = DBVARIANT();
	value.type = DBVT_ASCIIZ;
	value.pszVal = buffer.data();
	value.cchVal = buffer.size();

	auto dbcgs = DBCONTACTGETSETTING();
	dbcgs.szModule = contactInfo.szProto;
	dbcgs.szSetting = ChatRoomIdSettingName;
	dbcgs.pValue = &value;

	if (CallService(
		MS_DB_CONTACT_GETSETTINGSTATIC,
		reinterpret_cast<WPARAM>(conference),
		reinterpret_cast<LPARAM>(&dbcgs)))
	{
		throw std::exception("Cannot get chatroom id");
	}

	auto jabber = getJabberApi(contactInfo.szProto);
	if (jabber == nullptr)
	{
		// Not a Jabber contact, return.
		return;
	}

	auto xmlApi = XML_API();
	if (!mir_getXI(&xmlApi))
	{
		throw std::exception("Cannot get XML API");
	}

	auto id = wstring(L"mir_" + to_wstring(jabber->SerialNext()));
	auto to = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(buffer.data());

	auto node = xmlApi.createNode(L"iq", L"", 0);
	xmlApi.addAttr(node, L"type", L"set");
	xmlApi.addAttr(node, L"to", to.c_str());
	xmlApi.addAttr(node, L"id", id.c_str());
	
	auto query = xmlApi.addChild(node, L"query", L"");
	xmlApi.addAttr(query, L"xmlns", L"http://jabber.org/protocol/muc#admin");

	auto item = xmlApi.addChild(query, L"item", L"");
	xmlApi.addAttr(item, L"jid", banInfo.Jid.c_str());
	xmlApi.addAttr(item, L"affilation", L"outcast");

	auto reason = xmlApi.addChild(item, L"reason", banInfo.Reason.c_str());

	jabber->SendXmlNode(node);
}

vector<HANDLE> PluginCore::GetActiveConferences()
{
	auto result = vector<HANDLE>();
	
	auto handle = db_find_first();
	while (handle != NULL)
	{
		auto contactInfo = CONTACTINFO();
		contactInfo.hContact = handle;
		contactInfo.dwFlag = CNF_NICK | CNF_UNICODE;
		
		CallService(MS_CONTACT_GETCONTACTINFO, 0, reinterpret_cast<LPARAM>(&contactInfo));

		auto value = DBVARIANT();
		auto dbcgs = DBCONTACTGETSETTING();
		dbcgs.szModule = contactInfo.szProto;
		dbcgs.szSetting = ChatRoomSettingName;
		dbcgs.pValue = &value;

		if (CallService(
			MS_DB_CONTACT_GETSETTINGSTATIC,
			reinterpret_cast<WPARAM>(handle),
			reinterpret_cast<LPARAM>(&dbcgs)))
		{
			handle = db_find_next(handle);
			continue;
		}

		result.push_back(handle);
		handle = db_find_next(handle);
	}

	return result;
}