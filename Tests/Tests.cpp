#include "../Include/FixedList.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace hh;

#include <iostream>
#include <memory>
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

TEST_CASE("copyConstruct")
{
	FixedList<string, 15> l1 {"1", "2", "3", "4", "5"};
	FixedList<string, 15> l2 = l1;
	CHECK(l1.size() == l2.size());

	auto s2		 = l2.begin();
	bool reached = false;
	for(auto& s1 : l1)
	{
		reached = true;
		CHECK(s1 == *s2++);
	}
	CHECK(reached);

	static_assert(std::is_trivially_copy_constructible_v<FixedList<int, 1>>);
	static_assert(!std::is_trivially_copy_constructible_v<FixedList<string, 1>>);
}

TEST_CASE("moveConstruct")
{
	static unsigned counter = 10;
	struct I
	{
		virtual ~I() = default;

		virtual void fun() = 0;
	};
	struct X : I
	{
		~X()
		{
			--counter;
		}

		void fun() override
		{
			--counter;
		}
	};

	FixedList<std::unique_ptr<I>, 15> l1;
	l1.emplace_back(std::make_unique<X>());
	l1.emplace_back(std::make_unique<X>());
	l1.emplace_back(std::make_unique<X>());
	l1.emplace_back(std::make_unique<X>());
	l1.emplace_back(std::make_unique<X>());
	CHECK(l1.size() == 5);

	{
		FixedList<std::unique_ptr<I>, 15> l2 = std::move(l1);
		CHECK(l2.size() == 5);

		for(auto& i : l2)
			i->fun();
		CHECK(counter == 5);
	}

	CHECK(counter == 0);

	static_assert(std::is_trivially_move_constructible_v<FixedList<int, 1>>);
	static_assert(!std::is_trivially_move_constructible_v<decltype(l1)>);
}

TEST_CASE("destructor")
{
	FixedList<int, 5>	 l1;
	FixedList<string, 5> l2;

	static_assert(std::is_trivially_destructible_v<decltype(l1)>);
	static_assert(!std::is_trivially_destructible_v<decltype(l2)>);
}

TEST_CASE("copyAssign")
{
	FixedList<string, 3> l1 {"AA", "BB", "CC"};
	FixedList<string, 3> l2 {"DD", "EE", "FF"};

	CHECK(l2[0] == "DD");
	CHECK(l2[1] == "EE");
	CHECK(l2[2] == "FF");

	l2 = l1;

	CHECK(l2[0] == "AA");
	CHECK(l2[1] == "BB");
	CHECK(l2[2] == "CC");
}

TEST_CASE("moveAssign")
{
	static unsigned counter = 12;
	struct I
	{
		virtual ~I() = default;

		virtual void fun() = 0;
	};
	struct X : I
	{
		~X()
		{
			--counter;
		}

		void fun() override
		{
			--counter;
		}
	};

	FixedList<std::unique_ptr<I>, 15> l1;
	l1.emplace_back(std::make_unique<X>());
	l1.emplace_back(std::make_unique<X>());
	l1.emplace_back(std::make_unique<X>());
	l1.emplace_back(std::make_unique<X>());
	l1.emplace_back(std::make_unique<X>());
	l1.emplace_back(std::make_unique<X>());
	CHECK(l1.size() == 6);

	{
		FixedList<std::unique_ptr<I>, 15> l2;
		l2.emplace_back(std::make_unique<X>());
		l2.emplace_back(std::make_unique<X>());
		l2.emplace_back(std::make_unique<X>());
		CHECK(l2.size() == 3);

		for(auto& i : l2)
			i->fun();

		l2 = std::move(l1);
		CHECK(counter == 6);
	}

	CHECK(counter == 0);
}

TEST_CASE("operator==")
{
	FixedList<string, 7> l1 {"9", "8", "7"};
	FixedList<string, 7> l2 {"9", "8", "7"};
	CHECK(l1 == l2);
}

TEST_CASE("operator!=")
{
	FixedList<string, 7> l1 {"9", "8", "7"};
	FixedList<string, 7> l2 {"9", "6", "7"};
	CHECK(l1 != l2);
}

TEST_CASE("operator<")
{
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "D"};
	CHECK(l1 < l2);
}

TEST_CASE("operator<=")
{
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "D"};
	CHECK(l1 <= l2);

	FixedList<string, 7> l3 {"A", "B", "C"};
	FixedList<string, 7> l4 {"A", "B", "C"};
	CHECK(l3 <= l4);
}

TEST_CASE("operator>")
{
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "B"};
	CHECK(l1 > l2);
}

TEST_CASE("operator>=")
{
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "B"};
	CHECK(l1 >= l2);

	FixedList<string, 7> l3 {"A", "B", "C"};
	FixedList<string, 7> l4 {"A", "B", "C"};
	CHECK(l3 >= l4);
}

TEST_CASE("operator<=>")
{
	std::strong_ordering result;

	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "C"};
	result = l1 <=> l2;
	CHECK(result == std::strong_ordering::equal);

	FixedList<string, 7> l3 {"A", "B", "C"};
	FixedList<string, 7> l4 {"A", "B", "B"};
	result = l3 <=> l4;
	CHECK(result == std::strong_ordering::greater);

	FixedList<string, 7> l5 {"A", "B", "B"};
	FixedList<string, 7> l6 {"A", "C", "C"};
	result = l5 <=> l6;
	CHECK(result == std::strong_ordering::less);
}

TEST_CASE("operator[]")
{
	FixedList<string, 7> l {"X", "Y", "Z"};
	CHECK(l[2] == "Z");

	l[2] = "_";
	CHECK(l[2] != "Z");
	CHECK(l[2] == "_");

	FixedList<string, 7> const l2 = {};
	static_assert(std::is_const_v<std::remove_reference_t<decltype(std::declval<decltype(l2)>()[0])>>);
}

TEST_CASE("at")
{
	FixedList<string, 7> l {"X", "Y", "Z"};

	bool reached = false;
	try
	{
		l.at(3);
	}
	catch(std::out_of_range)
	{
		reached = true;
	}
	CHECK(reached);

	CHECK(l.at(1) == "Y");
}

TEST_CASE("get")
{
	FixedList<string, 7> l {"X", "Y", "Z"};

	CHECK(l.get(3) == nullptr);
	CHECK(l.get(6) == nullptr);
	CHECK(*l.get(2) == "Z");

	*l.get(1) = ".";
	CHECK(*l.get(1) != "Z");
	CHECK(*l.get(1) == ".");
}

TEST_CASE("front")
{
	FixedList<string, 7> l {"X", "Y", "Z"};

	CHECK(l.front() == "X");
	l.front() = "A";
	CHECK(l.front() == "A");
}

TEST_CASE("back")
{
	FixedList<string, 7> l {"X", "Y", "Z"};

	CHECK(l.back() == "Z");
	l.back() = "C";
	CHECK(l.back() == "C");
}

TEST_CASE("assignWithCount")
{
	FixedList<string, 7> l {"X", "Y", "Z"};
	CHECK(l[0] == "X");
	CHECK(l[1] == "Y");
	CHECK(l[2] == "Z");

	l.assign(7, "...");
	CHECK(l.size() == 7);
	for(auto& s : l)
		CHECK(s == "...");
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
