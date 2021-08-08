#include "../Include/FixedList.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace hh;

#include <iostream>
#include <string>
#include <vector>

using namespace std;

TEST_CASE("defaultConstruct")
{
	FixedList<string, 5> l;
	CHECK(l.count() == 0);
}

TEST_CASE("constructWithCount")
{
	struct A
	{
		string s = "ABC";
	};
	FixedList<A, 15> l(11);
	CHECK(l.size() == 11);

	for(auto& a : l)
		CHECK(a.s == "ABC");
}

TEST_CASE("constructWithCountDefaultValue")
{
	constexpr auto atla =
		"Water. Earth. Fire. Air. My grandmother used to tell me stories about the old days, a time of peace when the Avatar "
		"kept balance between the Water Tribes, Earth Kingdom, Fire Nation, and Air Nomads. But that all changed when the Fire "
		"Nation attacked. Only the Avatar mastered all four elements. Only he could stop the ruthless fire-benders. But when "
		"the world needed him most, he vanished. A hundred years have passed and the Fire Nation is nearing victory in the "
		"War. Two years ago, my father and the men of my tribe journeyed to the Earth Kingdom to help fight against the Fire "
		"Nation, leaving me and my brother to look after our tribe. Some people believe that the Avatar was never reborn into "
		"the Air Nomads, and that the cycle is broken. But I haven't lost hope. I still believe that somehow, the Avatar will "
		"return to save the world.";
	FixedList<string, 15> l(11, atla);
	CHECK(l.size() == 11);

	for(auto& s : l)
		CHECK(s == atla);
}

TEST_CASE("constructWithInitList")
{
	FixedList<string, 15> l {"1", "2", "3", "4", "5"};
	CHECK(l.size() == 5);

	CHECK(l[0] == "1");
	CHECK(l[1] == "2");
	CHECK(l[2] == "3");
	CHECK(l[3] == "4");
	CHECK(l[4] == "5");
}

TEST_CASE("insert(It,It)")
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
	vector<int> v {2, 3};
	CHECK(l == v);
}
