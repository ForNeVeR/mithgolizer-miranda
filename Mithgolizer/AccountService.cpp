#include "AccountService.hpp"

#include <array>
#include <codecvt>
#include <exception>
#include <locale>

#include <Windows.h>

#include <m_core.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_genmenu.h>
#include <m_jabber.h>
#include <m_langpack.h>
#include <m_skin.h>

#include "InputDialog.hpp"
#include "PluginCore.hpp"

using namespace std;
using namespace Mithgolizer;

const auto BanServiceNamePrefix = string("Mithgolizer/Ban");
const auto ChatRoomSettingName = "ChatRoom";
const auto ChatRoomIdSettingName = "ChatRoomID";
const auto MyNickSettingName = "MyNick";

AccountService::AccountService(const PluginCore &core, const std::string &accountName)
	: _core(core), _accountName(accountName)
{
	InitializeMenu();
}

AccountService::~AccountService()
{
	// TODO: Destroy menu and service.
}

void AccountService::InitializeMenu()
{
	auto accountMenu = MO_GetProtoRootMenu(_accountName.c_str());
	if (!accountMenu)
	{
		// TODO: Account seem to be inactive. Do something!
		return;
	}

	auto serviceFunction = [](void *servicePointer, LPARAM param1, LPARAM param2) -> INT_PTR
	{
		auto service = static_cast<AccountService*>(servicePointer);
		service->BanUser();
		return 0;
	};

	auto serviceName = BanServiceNamePrefix + _accountName;
	CreateServiceFunctionObj(serviceName.c_str(), serviceFunction, this);

	auto mi = CLISTMENUITEM();
	mi.cbSize = sizeof(mi);
	
	mi.flags = CMIF_CHILDPOPUP | CMIF_UNICODE;
	mi.position = 200004;
	mi.hParentMenu = accountMenu;
	
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.ptszName = LPGENW("&Ban User Everywhere");
	mi.pszService = const_cast<char*>(serviceName.c_str());	

	Menu_AddProtoMenuItem(&mi);
}

void AccountService::BanUser()
{
	auto banInfo = InputDialog::ShowDialog(_core.ModuleInstance());
	if (!banInfo.Jid.empty())
	{
		for (const auto conference : GetActiveConferences())
		{
			BanUser(banInfo, conference);
		}
	}
}

void AccountService::BanUser(const BanInfo &banInfo, MCONTACT conference)
{
	auto contactInfo = CONTACTINFO();
	contactInfo.hContact = conference;
	contactInfo.dwFlag = CNF_NICK | CNF_UNICODE;

	CallService(MS_CONTACT_GETCONTACTINFO, 0, reinterpret_cast<LPARAM>(&contactInfo));

	auto buffer = array<char, 256>();

	if (db_get_static(
		conference,
		contactInfo.szProto,
		ChatRoomIdSettingName,
		buffer.data(),
		buffer.size()))
	{
		throw std::exception("Cannot get chatroom id");
	}

	auto jabber = getJabberApi(contactInfo.szProto);
	if (jabber == nullptr)
	{
		throw std::exception("It seems that protocol is not a Jabber");
	}

	auto xmlApi = XML_API();
	if (!mir_getXI(&xmlApi))
	{
		throw std::exception("Cannot get XML API");
	}

	auto id = wstring(L"mir_" + to_wstring(jabber->SerialNext()));
	auto to = wstring_convert<codecvt_utf8<wchar_t>>().from_bytes(buffer.data());

	auto node = xmlApi.createNode(L"iq", L"", 0);
	xmlApi.addAttr(node, L"type", L"set");
	xmlApi.addAttr(node, L"to", to.c_str());
	xmlApi.addAttr(node, L"id", id.c_str());

	auto query = xmlApi.addChild(node, L"query", L"");
	xmlApi.addAttr(query, L"xmlns", L"http://jabber.org/protocol/muc#admin");

	auto item = xmlApi.addChild(query, L"item", L"");
	xmlApi.addAttr(item, L"jid", banInfo.Jid.c_str());
	xmlApi.addAttr(item, L"affiliation", L"outcast");

	auto reason = xmlApi.addChild(item, L"reason", banInfo.Reason.c_str());

	jabber->SendXmlNode(node);
}

vector<MCONTACT> AccountService::GetActiveConferences()
{
	auto result = vector<MCONTACT>();

	auto handle = db_find_first();
	while (handle != NULL)
	{
		auto contactInfo = CONTACTINFO();
		contactInfo.hContact = handle;
		contactInfo.dwFlag = CNF_NICK | CNF_UNICODE;

		CallService(MS_CONTACT_GETCONTACTINFO, 0, reinterpret_cast<LPARAM>(&contactInfo));

		auto protocol = string(contactInfo.szProto);
		if (protocol != _accountName)
		{
			handle = db_find_next(handle);
			continue;
		}

		auto value = DBVARIANT();
		auto dbcgs = DBCONTACTGETSETTING();
		dbcgs.szModule = contactInfo.szProto;
		dbcgs.szSetting = ChatRoomSettingName;
		dbcgs.pValue = &value;

		if (db_get_static(handle, contactInfo.szProto, ChatRoomSettingName, nullptr, 0))
		{
			handle = db_find_next(handle);
			continue;
		}

		result.push_back(handle);
		handle = db_find_next(handle);
	}

	return result;
}