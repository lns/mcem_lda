#pragma once

#include <boost/python.hpp>

namespace py = boost::python;

// A simple std::vector to py::list converter
template<typename T>
py::list toPyList(const std::vector<T>& vec) {
	py::list l;
	for(const auto& each : vec)
		l.append(each);
	return l;
}

