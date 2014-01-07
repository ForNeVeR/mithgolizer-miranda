#include "InputDialog.hpp"

#include <array>

#include <m_core.h>

#include "resource.h"

using namespace Mithgolizer;


static INT_PTR CALLBACK DialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(dialog);
		SetWindowLongPtrW(dialog, GWLP_USERDATA, lParam); // initial parameter
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			{
				std::array<wchar_t, 256> jid, reason;

				GetDlgItemTextW(dialog, IDC_JID, jid.data(), jid.size());
				GetDlgItemTextW(dialog, IDC_REASON, reason.data(), reason.size());

				auto result = reinterpret_cast<BanInfo*>(GetWindowLongPtrW(dialog, GWLP_USERDATA));
				result->Jid = std::wstring(jid.begin(), jid.end());
				result->Reason = std::wstring(reason.begin(), reason.end());

				EndDialog(dialog, true);
				break;
			}
		case IDCANCEL:
			EndDialog(dialog, false);
			break;
		}

		break;
	}

	return 0;
}

BanInfo InputDialog::ShowDialog(HINSTANCE moduleInstance)
{
	auto banInfo = BanInfo();

	DialogBoxParamW(
		moduleInstance,
		MAKEINTRESOURCE(IDD_INPUTDIALOG),
		nullptr,
		DialogProc,
		reinterpret_cast<LPARAM>(&banInfo));

	return banInfo;
}