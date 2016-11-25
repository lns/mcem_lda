#pragma once

#include "gtest/gtest.h"
#include "SparseMat.hpp"

TEST(MCEM_LDA, SparseMatTest) {
	SparseMat<int> smat;
	for(int i=0; i<5; i++)
		for(int j=0; j<5; j++)
			smat.append(i,j,100*i+j);
	smat.build_CSC_from_CSR();
	for(int i=0; i<5; i++)
		for(int j=0; j<5; j++)
			EXPECT_EQ(smat.at(i,j), 100*i+j);
	smat.apply([](int& value, int i, int j) {
		EXPECT_EQ(value, 100*i+j);
	});
	smat.apply<true>([](int& value, int i, int j) {
		EXPECT_EQ(value, 100*i+j);
	});
	auto cnt = 0;
	smat.apply([&](int& value, int i, int j) {
		cnt += value;
	});
	smat.apply<true>([&](int& value, int i, int j) {
		cnt -= value;
	});
	EXPECT_EQ(cnt, 0);
}

