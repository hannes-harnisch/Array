#include "../Include/FixedList.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace hh;

#include <iostream>
#include <string>

using namespace std;

TEST_CASE("defaultConstruct")
{
	FixedList<string, 5> l;
	CHECK(l.count() == 0);
}

TEST_CASE("insert_Range")
{
	FixedList<string, 15> l;
	l.emplace_back("AAA");
	l.emplace_back("BBB");
	l.emplace_back("CCC");

	auto il = {"A", "B", "C"};

	l.insert(l.begin() + 1, il.begin(), il.end());
	CHECK(l[1] == "A");
	CHECK(l[2] == "B");
	CHECK(l[3] == "C");
}

TEST_CASE("emplace")
{
	FixedList<string, 5> l;
	l.emplace_back("AAA");
	l.emplace_back("BBB");
	l.emplace_back("CCC");
	l.emplace(l.begin() + 1, "XXX");
	CHECK(l[1] == "XXX");
}

TEST_CASE("erase(It,It)")
{
	FixedList<int, 10> l {2, 3, 4, 5, 6, 7, 8, 9};
	l.erase(l.begin() + 2, l.end());
	static_assert(std::is_trivially_destructible_v<decltype(l)>);
}
