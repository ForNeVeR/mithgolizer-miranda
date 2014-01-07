#include <Windows.h>

#include <newpluginapi.h>

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

		const PLUGININFOEX &_pluginInfo;
		HINSTANCE _moduleInstance;
	};
}