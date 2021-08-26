#define HH_ASSERT(condition, message)                                                                                          \
	{                                                                                                                          \
		if(!(condition))                                                                                                       \
		{                                                                                                                      \
			__debugbreak();                                                                                                    \
			std::abort();                                                                                                      \
		}                                                                                                                      \
	}

#include "../Include/FixedList.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace hh;

#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

TEST_CASE("ctor()")
{
	FixedList<string, 5> l;
	REQUIRE(l.count() == 0);

	static_assert(is_nothrow_default_constructible_v<decltype(l)>);
}

TEST_CASE("ctor(count)")
{
	struct A
	{
		string s = "ABC";
	};
	FixedList<A, 15> l(11);
	REQUIRE(l.size() == 11);

	for(auto& a : l)
		REQUIRE(a.s == "ABC");
}

TEST_CASE("ctor(count,value)")
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
	REQUIRE(l.size() == 11);

	for(auto& s : l)
		REQUIRE(s == atla);
}

TEST_CASE("ctor(first,last)")
{
	vector<string> v {"E", "D", "C", "B", "A"};

	FixedList<string, 15> l(v.begin(), v.end());
	REQUIRE(l.size() == 5);

	REQUIRE(l[0] == "E");
	REQUIRE(l[1] == "D");
	REQUIRE(l[2] == "C");
	REQUIRE(l[3] == "B");
	REQUIRE(l[4] == "A");

	static unsigned counter = 8;
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
	vector<unique_ptr<I>> v2;
	v2.emplace_back(make_unique<X>());
	v2.emplace_back(make_unique<X>());
	v2.emplace_back(make_unique<X>());
	v2.emplace_back(make_unique<X>());

	auto move_begin = make_move_iterator(v2.begin());
	auto move_end	= make_move_iterator(v2.end());
	{
		FixedList<unique_ptr<I>, 6> l2(move_begin, move_end);
		REQUIRE(l2.size() == 4);

		for(auto& p : l2)
			p->fun();

		REQUIRE(counter == 4);
	}
	REQUIRE(counter == 0);
}

TEST_CASE("ctor(init)")
{
	FixedList<string, 15> l {"1", "2", "3", "4", "5"};
	REQUIRE(l.size() == 5);

	REQUIRE(l[0] == "1");
	REQUIRE(l[1] == "2");
	REQUIRE(l[2] == "3");
	REQUIRE(l[3] == "4");
	REQUIRE(l[4] == "5");
}

TEST_CASE("ctor(copy)")
{
	FixedList<string, 15> l1 {"1", "2", "3", "4", "5"};
	FixedList<string, 15> l2 = l1;
	REQUIRE(l1.size() == l2.size());

	auto s2		 = l2.begin();
	bool reached = false;
	for(auto& s1 : l1)
	{
		reached = true;
		REQUIRE(s1 == *s2++);
	}
	REQUIRE(reached);

	FixedList<int, 10> l3 {1, 2, 3, 4, 5};
	FixedList<int, 10> l4 = l3;
	REQUIRE(l3.size() == l4.size());

	auto i	= l3.begin();
	reached = false;
	for(auto& s3 : l3)
	{
		reached = true;
		REQUIRE(s3 == *i++);
	}
	REQUIRE(reached);

	static_assert(is_trivially_copy_constructible_v<FixedList<int, 1>>);
	static_assert(!is_trivially_copy_constructible_v<FixedList<string, 1>>);
}

TEST_CASE("ctor(move)")
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

	FixedList<unique_ptr<I>, 15> l1;
	l1.emplace_back(make_unique<X>());
	l1.emplace_back(make_unique<X>());
	l1.emplace_back(make_unique<X>());
	l1.emplace_back(make_unique<X>());
	l1.emplace_back(make_unique<X>());
	REQUIRE(l1.size() == 5);

	{
		FixedList<unique_ptr<I>, 15> l2 = move(l1);
		REQUIRE(l2.size() == 5);

		for(auto& i : l2)
			i->fun();

		REQUIRE(counter == 5);
	}

	REQUIRE(counter == 0);

	static_assert(is_trivially_move_constructible_v<FixedList<int, 1>>);
	static_assert(!is_trivially_move_constructible_v<decltype(l1)>);
}

TEST_CASE("dtor")
{
	FixedList<int, 5>	 l1;
	FixedList<string, 5> l2;

	static_assert(is_trivially_destructible_v<decltype(l1)>);
	static_assert(!is_trivially_destructible_v<decltype(l2)>);
}

TEST_CASE("operator=(copy)")
{
	FixedList<string, 3> l1 {"AA", "BB", "CC"};
	FixedList<string, 3> l2 {"DD", "EE", "FF"};

	REQUIRE(l2[0] == "DD");
	REQUIRE(l2[1] == "EE");
	REQUIRE(l2[2] == "FF");

	l2 = l1;

	REQUIRE(l2[0] == "AA");
	REQUIRE(l2[1] == "BB");
	REQUIRE(l2[2] == "CC");
}

TEST_CASE("operator=(move)")
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

	FixedList<unique_ptr<I>, 15> l1;
	l1.emplace_back(make_unique<X>());
	l1.emplace_back(make_unique<X>());
	l1.emplace_back(make_unique<X>());
	l1.emplace_back(make_unique<X>());
	l1.emplace_back(make_unique<X>());
	l1.emplace_back(make_unique<X>());
	REQUIRE(l1.size() == 6);

	{
		FixedList<unique_ptr<I>, 15> l2;
		l2.emplace_back(make_unique<X>());
		l2.emplace_back(make_unique<X>());
		l2.emplace_back(make_unique<X>());
		REQUIRE(l2.size() == 3);

		for(auto& i : l2)
			i->fun();

		l2 = move(l1);
		REQUIRE(counter == 6);
	}

	REQUIRE(counter == 0);
}

TEST_CASE("operator==")
{
	FixedList<string, 7> l1 {"9", "8", "7"};
	FixedList<string, 7> l2 {"9", "8", "7"};
	REQUIRE(l1 == l2);
}

TEST_CASE("operator!=")
{
	FixedList<string, 7> l1 {"9", "8", "7"};
	FixedList<string, 7> l2 {"9", "6", "7"};
	REQUIRE(l1 != l2);
}

TEST_CASE("operator<")
{
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "D"};
	REQUIRE(l1 < l2);
}

TEST_CASE("operator<=")
{
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "D"};
	REQUIRE(l1 <= l2);

	FixedList<string, 7> l3 {"A", "B", "C"};
	FixedList<string, 7> l4 {"A", "B", "C"};
	REQUIRE(l3 <= l4);
}

TEST_CASE("operator>")
{
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "B"};
	REQUIRE(l1 > l2);
}

TEST_CASE("operator>=")
{
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "B"};
	REQUIRE(l1 >= l2);

	FixedList<string, 7> l3 {"A", "B", "C"};
	FixedList<string, 7> l4 {"A", "B", "C"};
	REQUIRE(l3 >= l4);
}

TEST_CASE("operator<=>")
{
	strong_ordering result;

	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "C"};
	result = l1 <=> l2;
	REQUIRE(result == strong_ordering::equal);

	FixedList<string, 7> l3 {"A", "B", "C"};
	FixedList<string, 7> l4 {"A", "B", "B"};
	result = l3 <=> l4;
	REQUIRE(result == strong_ordering::greater);

	FixedList<string, 7> l5 {"A", "B", "B"};
	FixedList<string, 7> l6 {"A", "C", "C"};
	result = l5 <=> l6;
	REQUIRE(result == strong_ordering::less);
}

TEST_CASE("operator[]")
{
	FixedList<string, 7> l {"X", "Y", "Z"};
	REQUIRE(l[2] == "Z");

	l[2] = "_";
	REQUIRE(l[2] != "Z");
	REQUIRE(l[2] == "_");

	FixedList<string, 7> const l2 = {};
	static_assert(is_const_v<remove_reference_t<decltype(l2[0])>>);
}

TEST_CASE("at")
{
	FixedList<string, 7> l {"X", "Y", "Z"};

	bool reached = false;
	try
	{
		l.at(3);
	}
	catch(out_of_range)
	{
		reached = true;
	}
	REQUIRE(reached);

	REQUIRE(l.at(1) == "Y");
}

TEST_CASE("get")
{
	FixedList<string, 7> l {"X", "Y", "Z"};

	REQUIRE(l.get(3) == nullptr);
	REQUIRE(l.get(6) == nullptr);
	REQUIRE(*l.get(2) == "Z");

	*l.get(1) = ".";
	REQUIRE(*l.get(1) != "Z");
	REQUIRE(*l.get(1) == ".");
}

TEST_CASE("front")
{
	FixedList<string, 7> l {"X", "Y", "Z"};

	REQUIRE(l.front() == "X");
	l.front() = "A";
	REQUIRE(l.front() == "A");
}

TEST_CASE("back")
{
	FixedList<string, 7> l {"X", "Y", "Z"};

	REQUIRE(l.back() == "Z");
	l.back() = "C";
	REQUIRE(l.back() == "C");
}

TEST_CASE("assign(count,value)")
{
	FixedList<string, 7> l {"X", "Y", "Z"};
	REQUIRE(l[0] == "X");
	REQUIRE(l[1] == "Y");
	REQUIRE(l[2] == "Z");

	l.assign(7, "...");

	REQUIRE(l.size() == 7);
	for(auto& s : l)
		REQUIRE(s == "...");
}

class TestException : public std::exception
{};

TEST_CASE("assign(count,value) throw")
{
	static unsigned counter = 2;
	struct X
	{
		X() = default;

		X(X const&)
		{
			if(!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool reached = false;

	FixedList<X, 9> l(3);
	try
	{
		l.assign(5, {});
	}
	catch(TestException)
	{
		reached = true;
		REQUIRE(l.empty());
	}
	REQUIRE(reached);
}

TEST_CASE("assign(first,last)")
{
	FixedList<string, 7> l {"X", "Y", "Z"};
	REQUIRE(l[0] == "X");
	REQUIRE(l[1] == "Y");
	REQUIRE(l[2] == "Z");

	vector<string> v {"4", "5", "6", "7", "8", "9", "10"};

	l.assign(v.begin() + 1, v.end());

	REQUIRE(l.size() == 6);

	auto vb = v.begin() + 1;
	for(auto& s : l)
		REQUIRE(s == *vb++);
}

TEST_CASE("assign(first,last) throw")
{
	static unsigned counter = 2;
	struct X
	{
		X() = default;

		X(X const&)
		{
			if(!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool reached = false;

	FixedList<X, 9> l(3);
	FixedList<X, 9> l2(3);
	try
	{
		l.assign(l2.begin(), l2.end());
	}
	catch(TestException)
	{
		reached = true;
		REQUIRE(l.empty());
	}
	REQUIRE(reached);
}

TEST_CASE("assign(init)")
{
	FixedList<string, 10> l(5);
	REQUIRE(l.size() == 5);

	for(auto& s : l)
		REQUIRE(s.empty());

	l.assign({"A", "B", "C"});
	REQUIRE(l[0] == "A");
	REQUIRE(l[1] == "B");
	REQUIRE(l[2] == "C");
}

TEST_CASE("assign(init) throw")
{
	static unsigned counter = 1;
	struct X
	{
		X() = default;

		X(X const&)
		{
			if(!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool reached = false;

	FixedList<X, 9> l(3);
	try
	{
		l.assign({X(), X(), X()});
	}
	catch(TestException)
	{
		reached = true;
		REQUIRE(l.empty());
	}
	REQUIRE(reached);
}

TEST_CASE("insert(pos,value)")
{
	FixedList<string, 11> l {"A", "B", "D", "E"};

	auto it = l.insert(l.begin() + 2, "C");
	REQUIRE(l[2] == "C");
	REQUIRE(*it == "C");

	array arr {"A", "B", "C", "D", "E"};
	REQUIRE(l == arr);
}

TEST_CASE("insert(pos,value) throw")
{
	struct X
	{
		string s;

		X(string s) : s(s)
		{}

		X(X const&)
		{
			throw TestException();
		}

		X(X&&) = default;
	};

	bool reached = false;

	FixedList<X, 9> l {"1", "2", "3"};
	try
	{
		X x("NOPE");
		l.insert(l.begin(), x); // Info: This line currently causes the false warning about unreachable code in MSVC.
	}
	catch(TestException)
	{
		reached = true;

		array arr {"1", "2", "3"};
		auto  i = l.begin();
		for(auto str : arr)
			REQUIRE(i++->s == str);
	}
	REQUIRE(reached);
}

TEST_CASE("try_insert(pos,value)")
{
	FixedList<string, 4> l {"A", "B", "D", "E"};

	auto it = l.try_insert(l.begin(), "C");
	REQUIRE(it == l.end());
}

TEST_CASE("insert(pos,count,value)")
{
	FixedList<string, 11> l {"A", "B", "C"};

	auto it = l.insert(l.begin() + 3, 5, "D");
	REQUIRE(l[3] == "D");
	REQUIRE(l[4] == "D");
	REQUIRE(l[5] == "D");
	REQUIRE(l[6] == "D");
	REQUIRE(l[7] == "D");
	REQUIRE(*it == "D");

	array arr {"A", "B", "C", "D", "D", "D", "D", "D"};
	REQUIRE(l == arr);
}

TEST_CASE("insert(pos,count,value) throw")
{
	static unsigned counter = 3;
	struct X
	{
		string s;

		X(string s) : s(s)
		{}

		X(X const&)
		{
			if(!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool reached = false;

	FixedList<X, 9> l {"1", "2", "3"};
	try
	{
		X x("6");
		l.insert(l.begin(), 5, x);
	}
	catch(TestException)
	{
		reached = true;

		array arr {"1", "2", "3"};
		auto  i = l.begin();
		for(auto str : arr)
			REQUIRE(i++->s == str);
	}
	REQUIRE(reached);
}

TEST_CASE("try_insert(pos,count,value)")
{
	FixedList<string, 8> l {"B", "C", "D", "E"};

	auto it = l.try_insert(l.begin(), 5, "A");
	REQUIRE(it == l.end());
}

TEST_CASE("insert(first,last)")
{
	FixedList<string, 15> l;
	l.emplace_back("AAA");
	l.emplace_back("BBB");
	l.emplace_back("CCC");

	auto il = {"A", "B", "C"};

	auto it = l.insert(l.begin() + 1, il.begin(), il.end() - 1);
	REQUIRE(l[1] == "A");
	REQUIRE(l[2] == "B");
	REQUIRE(l[3] == "BBB");
	REQUIRE(*it == "A");
}

TEST_CASE("insert(first,last) throw")
{
	static unsigned counter = 3;
	struct X
	{
		string s;

		X(string s) : s(s)
		{}

		X(X const&)
		{
			if(!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool reached = false;

	FixedList<X, 9> l {"A", "B", "C"};
	try
	{
		vector v {X("ok"), X("ok"), X("no"), X("oops"), X("well...")};
		l.insert(l.begin(), v.begin(), v.end() - 2);
	}
	catch(TestException)
	{
		reached = true;

		array arr {"A", "B", "C"};
		auto  i = l.begin();
		for(auto str : arr)
			REQUIRE(i++->s == str);
	}
	REQUIRE(reached);
}

TEST_CASE("emplace")
{
	FixedList<string, 5> l;
	l.emplace_back("AAA");
	l.emplace_back("BBB");
	l.emplace_back("CCC");
	l.emplace(l.begin() + 1, "XXX");
	REQUIRE(l[1] == "XXX");
}

TEST_CASE("erase(first,last)")
{
	FixedList<int, 10> l {2, 3, 4, 5, 6, 7, 8, 9};
	l.erase(l.begin() + 2, l.end());
	vector<int> v {2, 3};
	REQUIRE(l == v);
}
