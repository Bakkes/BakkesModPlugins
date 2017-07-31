#include "PythonPlugin.h"
#include "wrapped.h"
#include <boost/python.hpp>
#include "helpers.h"

const string PY_PATH = "./bakkesmod/py/";


#ifdef _AI_BUILD
	BAKKESMOD_PLUGIN(PythonPlugin, "Python plugin", "0.1", PLUGINTYPE_BOTAI)
#else
	BAKKESMOD_PLUGIN(PythonPlugin, "Python plugin", "0.1", 0)
#endif

GameWrapper* gw;
ConsoleWrapper* cons;
extern std::string parse_python_exception();
extern void reinit_python();

object main_module;
dict main_namespace;

dict global_namespace;
dict local_namespace;


void reboundplugin_ConsoleNotifier(std::vector<std::string> params) {
	string command = params.at(0);
	if (!gw->IsInTutorial())
		return;

	if (command.compare("py_exec") == 0)
	{
		if (params.size() < 2) {
			cons->log("usage: " + command + " filename.py");
			return;
		}
		string path = PY_PATH + params.at(1);
		if (!file_exists(path) && !string_ends_with(path, ".py")) {
			path += ".py";
		}
		if (!file_exists(path)) {
			cons->log("Python script " + path + " does not exist!");
			return;
		}

		try {
			exec_file(str(path), global_namespace, global_namespace);
		}
		catch (const error_already_set&) {
			string err = parse_python_exception();
			cons->log("Python threw error: " + err);
		}
	}
#ifdef _AI_BUILD
	else if (command.compare("py_ai") == 0) {
		//Py_Finalize();
		//Py_Initialize();
		reinit_python();

		if (params.size() < 2) {
			cons->log("usage: " + command + " filename.py");
			return;
		}
		string path = PY_PATH + params.at(1);
		if (!file_exists(path) && !string_ends_with(path, ".py")) {
			path += ".py";
		}
		if (!file_exists(path)) {
			cons->log("Python script " + path + " does not exist!");
			return;
		}
		try {
			exec_file(str(path), global_namespace, global_namespace);
		}
		catch (const error_already_set&) {
			string err = parse_python_exception();
			cons->log("Python threw error: " + err);
		}
	}
#endif
}

void reinit_python() {
	try {
		//global_namespace.clear();
		//local_namespace.clear();

		// Copy builtins to new global namespace
		global_namespace = extract<dict>(main_module.attr("__dict__"));
		//global_namespace["__builtins__"] = main_namespace["__builtins__"];
		global_namespace["console"] = boost::python::ptr(cons);
		global_namespace["game_wrapper"] = boost::python::ptr(gw);
	}
	catch (const error_already_set&) {
		string err = parse_python_exception();
		cons->log("Python threw error: " + err);
	}
}

void PythonPlugin::onLoad()
{
	gw = gameWrapper;
	cons = console;
	
	cons->registerNotifier("py_exec", reboundplugin_ConsoleNotifier);
	cons->registerNotifier("py_load", reboundplugin_ConsoleNotifier);

#ifdef _AI_BUILD
	cons->registerNotifier("py_ai", reboundplugin_ConsoleNotifier);
#endif

	Py_Initialize();

	
	initbakkesmod();
	main_module
		= object(handle<>(borrowed(PyImport_AddModule("__main__"))));

	main_namespace = extract<dict>(main_module.attr("__dict__"));
	main_namespace["console"] = boost::python::ptr(cons);
	main_namespace["game_wrapper"] = boost::python::ptr(gw);
	reinit_python();
	
}

void PythonPlugin::onUnload()
{
	Py_Finalize();
}

namespace boost {
	namespace python {
		bool hasattr(object o, const char* name) {
			return PyObject_HasAttrString(o.ptr(), name);
		}
	}
}
#ifdef _AI_BUILD
void PythonPlugin::on_tick(ControllerInput * input, CarWrapper * localCar, BallWrapper * ball)
{
	if (hasattr(main_module, "on_tick")) {
		auto tick_func = main_module.attr("on_tick");
		tick_func(ptr(input));
	}
}
#endif

std::string parse_python_exception() {
	PyObject *type_ptr = NULL, *value_ptr = NULL, *traceback_ptr = NULL;
	// Fetch the exception info from the Python C API  
	PyErr_Fetch(&type_ptr, &value_ptr, &traceback_ptr);

	// Fallback error  
	std::string ret("Unfetchable Python error");
	// If the fetch got a type pointer, parse the type into the exception string  
	if (type_ptr != NULL) {
		handle<> h_type(type_ptr);
		str type_pstr(h_type);
		// Extract the string from the boost::python object  
		extract<std::string> e_type_pstr(type_pstr);
		// If a valid string extraction is available, use it   
		//  otherwise use fallback  
		if (e_type_pstr.check())
			ret = e_type_pstr();
		else
			ret = "Unknown exception type";
	}
	// Do the same for the exception value (the stringification of the exception)  
	if (value_ptr != NULL) {
		handle<> h_val(value_ptr);
		str a(h_val);
		extract<std::string> returned(a);
		if (returned.check())
			ret += ": " + returned();
		else
			ret += std::string(": Unparseable Python error: ");
	}
	// Parse lines from the traceback using the Python traceback module  
	if (traceback_ptr != NULL) {
		handle<> h_tb(traceback_ptr);
		// Load the traceback module and the format_tb function  
		object tb(import("traceback"));
		object fmt_tb(tb.attr("format_tb"));
		// Call format_tb to get a list of traceback strings  
		object tb_list(fmt_tb(h_tb));
		// Join the traceback strings into a single string  
		object tb_str(str("\n").join(tb_list));
		// Extract the string, check the extraction, and fallback in necessary  
		extract<std::string> returned(tb_str);
		if (returned.check())
			ret += ": " + returned();
		else
			ret += std::string(": Unparseable Python traceback");
	}
	return ret;
}