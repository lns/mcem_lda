#include <boost/python.hpp>
#include <string>
#include <cstdio>
#include "SparseMat.hpp"
#include "LDA.hpp"
#include "py_util.hpp"

namespace py = boost::python;

BOOST_PYTHON_MODULE(/* Name here */ mcemlda ) {
	py::class_<LDA>("LDA")
		.add_property("n_topic", &LDA::get_n_topic, &LDA::set_n_topic)
		.add_property("alpha", &LDA::get_alpha, &LDA::set_alpha)
		.add_property("beta", &LDA::get_beta, &LDA::set_beta)
		.def("check", &LDA::check)
		.def("init", &LDA::init)
		.def("sample_by_word", &LDA::sample_by_word)
		.def("sample_by_doc", &LDA::sample_by_doc)
		.def("mcem", &LDA::mcem)
		.def("get_top_words", &LDA::py_get_top_words)
		.def("read_corpus", &LDA::py_read_corpus)
		.def("show", &LDA::print)
		.def("write2csv", &LDA::write2csv)
	;
}

