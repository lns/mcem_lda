#pragma once

#include "gtest/gtest.h"
#include "Heap.hpp"

TEST(MCEM_LDA, HeapTest) {
	Heap<int,std::less<int>> min_heap;
	Heap<int,std::greater<int>> max_heap;
	min_heap.set_max_size(3);
	max_heap.set_max_size(3);

	if(true) {
		min_heap.clear();
		for(int i=0; i<10; i++)
			min_heap.push(i);

		min_heap.sort();
		auto& v = min_heap.get();
		EXPECT_EQ(v[0], 0);
		EXPECT_EQ(v[1], 1);
		EXPECT_EQ(v[2], 2);
	}
	if(true) {	
		min_heap.clear();
		for(int i=0; i<10; i++)
			min_heap.push(10-i);

		min_heap.sort();
		auto& v = min_heap.get();
		EXPECT_EQ(v[0], 1);
		EXPECT_EQ(v[1], 2);
		EXPECT_EQ(v[2], 3);
	}
	
	if(true) {
		max_heap.clear();
		for(int i=0; i<10; i++)
			max_heap.push(i);

		max_heap.sort();
		auto& v = max_heap.get();
		EXPECT_EQ(v[0], 9);
		EXPECT_EQ(v[1], 8);
		EXPECT_EQ(v[2], 7);
	}
	if(true) {	
		max_heap.clear();
		for(int i=0; i<10; i++)
			max_heap.push(10-i);

		max_heap.sort();
		auto& v = max_heap.get();
		EXPECT_EQ(v[0], 10);
		EXPECT_EQ(v[1],  9);
		EXPECT_EQ(v[2],  8);
	}
}

