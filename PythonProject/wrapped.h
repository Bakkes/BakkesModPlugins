#pragma once

#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE(game_wrapper)
{
	class_<GameWrapper, boost::noncopyable>("GameWrapper", no_init).
		def("is_in_tutorial", &GameWrapper::IsInTutorial);
}