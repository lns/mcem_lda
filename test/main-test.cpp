#include "gtest/gtest.h"

#include "AliasUrnTest.cpp"
#include "HeapTest.cpp"
#include "SparseMatTest.cpp"

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

