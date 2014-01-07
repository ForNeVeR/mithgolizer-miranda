#pragma once

#include "Windows.h"

#include "BanInfo.hpp"

namespace Mithgolizer
{
	class InputDialog
	{
	public:
		static BanInfo ShowDialog(HINSTANCE moduleInstance);
	};
}