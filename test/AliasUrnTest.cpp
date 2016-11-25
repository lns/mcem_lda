#pragma once

#include "gtest/gtest.h"
#include "qrand.hpp"
#include "AliasUrn.hpp"

TEST(MCEM_LDA, AliasUrnTest) {
	qlib::XOR128PLUS rng;
	AliasUrn<decltype(rng)> alias_urn(rng);
	vector<double> prob{0.3, 0.2, 0.3, 0.2};
	alias_urn.setup(prob);
	alias_urn.setup(vector<double>({0.5,0.5}));
	alias_urn.setup({0.33,0.33,0.34});
	alias_urn.setup({0.989,0.010,0.001});
	alias_urn.setup({0.2,0.2,0.2,0.2,0.2});
	alias_urn.setup({0.9,0.09,0.009,0.0009,0.0001});
	// Not normalized
	alias_urn.setup({1,1,2,2,4,4,6});
	//alias_urn.print();
}

