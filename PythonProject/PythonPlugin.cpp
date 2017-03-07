#include "PythonPlugin.h"
#include "wrapped.h"
#if PY_MAJOR_VERSION >= 3
#   define INIT_MODULE PyInit_mymodule
extern "C" PyObject* INIT_MODULE();
#else
#   define INIT_MODULE initmymodule
extern "C" void INIT_MODULE();
#endif

BAKKESMOD_PLUGIN(PythonPlugin, "Python plugin", "0.1", 0)

GameWrapper* gw;
ConsoleWrapper* cons;

void reboundplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);
	if (!gw->IsInTutorial())
		return;

	if (command.compare("rebound_shoot") == 0)
	{
		
	}

}

void PythonPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	
}

void PythonPlugin::onUnload()
{
}