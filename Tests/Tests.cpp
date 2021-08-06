#include "../Include/FixedList.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace hh;

TEST_CASE("defaultConstruct")
{
	FixedList<int, 5> l;
	CHECK(l.count() == 0);
}
