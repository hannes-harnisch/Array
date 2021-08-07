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

TEST_CASE("emplace")
{
	FixedList<string, 5> l;
	l.emplace_back("AAA");
	l.emplace_back("BBB");
	l.emplace_back("CCC");
	l.emplace(l.begin() + 1, "XXX");
	CHECK(l[1] == "XXX");
}
