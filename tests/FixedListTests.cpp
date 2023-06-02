#include "../include/FixedList.hpp"

#include <doctest/doctest.h>

using namespace hh;

#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

TEST_CASE("ctor()") {
	FixedList<string, 5> l;
	REQUIRE(l.count() == 0);

	static_assert(is_nothrow_default_constructible_v<decltype(l)>);
}

TEST_CASE("ctor(count)") {
	struct A {
		string s = "ABC";
	};

	FixedList<A, 15> l(11);
	REQUIRE(l.size() == 11);

	for (auto& a : l)
		REQUIRE(a.s == "ABC");
}

TEST_CASE("ctor(count,value)") {
	constexpr auto atla = "Water. Earth. Fire. Air. My grandmother used to tell me stories about the old days, a time of peace "
						  "when the Avatar kept balance between the Water Tribes, Earth Kingdom, Fire Nation, and Air Nomads. "
						  "But that all changed when the Fire Nation attacked. Only the Avatar mastered all four elements. "
						  "Only he could stop the ruthless fire-benders. But when the world needed him most, he vanished. A "
						  "hundred years have passed and the Fire Nation is nearing victory in the War. Two years ago, my "
						  "father and the men of my tribe journeyed to the Earth Kingdom to help fight against the Fire "
						  "Nation, leaving me and my brother to look after our tribe. Some people believe that the Avatar was "
						  "never reborn into the Air Nomads, and that the cycle is broken. But I haven't lost hope. I still "
						  "believe that somehow, the Avatar will return to save the world.";
	FixedList<string, 15> l(11, atla);
	REQUIRE(l.size() == 11);

	for (auto& s : l)
		REQUIRE(s == atla);
}

TEST_CASE("ctor(first,last) forward") {
	vector<string> v {"E", "D", "C", "B", "A"};

	FixedList<string, 15> l(v.begin(), v.end());
	REQUIRE(l.size() == 5);

	REQUIRE(l[0] == "E");
	REQUIRE(l[1] == "D");
	REQUIRE(l[2] == "C");
	REQUIRE(l[3] == "B");
	REQUIRE(l[4] == "A");
}

TEST_CASE("ctor(first,last) input") {
	static unsigned counter = 8;

	struct I {
		virtual ~I() = default;

		virtual void fun() = 0;
	};

	struct X : I {
		~X() {
			--counter;
		}

		void fun() override {
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

		for (auto& p : l2)
			p->fun();

		REQUIRE(counter == 4);
	}
	REQUIRE(counter == 0);
}

TEST_CASE("ctor(init)") {
	FixedList<string, 15> l {"1", "2", "3", "4", "5"};
	REQUIRE(l.size() == 5);

	REQUIRE(l[0] == "1");
	REQUIRE(l[1] == "2");
	REQUIRE(l[2] == "3");
	REQUIRE(l[3] == "4");
	REQUIRE(l[4] == "5");
}

TEST_CASE("ctor(copy)") {
	FixedList<string, 15> l1 {"1", "2", "3", "4", "5"};
	FixedList<string, 15> l2 = l1;
	REQUIRE(l1.size() == l2.size());

	auto s2		= l2.begin();
	bool thrown = false;
	for (auto& s1 : l1) {
		thrown = true;
		REQUIRE(s1 == *s2++);
	}
	REQUIRE(thrown);

	FixedList<int, 10> l3 {1, 2, 3, 4, 5};
	FixedList<int, 10> l4 = l3;
	REQUIRE(l3.size() == l4.size());

	auto i = l3.begin();
	thrown = false;
	for (auto& s3 : l3) {
		thrown = true;
		REQUIRE(s3 == *i++);
	}
	REQUIRE(thrown);
}

TEST_CASE("ctor(move)") {
	static unsigned counter = 10;

	struct I {
		virtual ~I() = default;

		virtual void fun() = 0;
	};

	struct X : I {
		~X() {
			--counter;
		}

		void fun() override {
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

		for (auto& i : l2)
			i->fun();

		REQUIRE(counter == 5);
	}

	REQUIRE(counter == 0);
}

TEST_CASE("dtor") {
	FixedList<int, 5>	 l1;
	FixedList<string, 5> l2;
}

TEST_CASE("operator=(copy)") {
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

TEST_CASE("operator=(move)") {
	static unsigned counter = 12;

	struct I {
		virtual ~I() = default;

		virtual void fun() = 0;
	};

	struct X : I {
		~X() {
			--counter;
		}

		void fun() override {
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

		for (auto& i : l2)
			i->fun();

		l2 = move(l1);
		REQUIRE(counter == 6);
	}

	REQUIRE(counter == 0);
}

TEST_CASE("operator==") {
	FixedList<string, 7> l1 {"9", "8", "7"};
	FixedList<string, 7> l2 {"9", "8", "7"};
	REQUIRE(l1 == l2);
}

TEST_CASE("operator!=") {
	FixedList<string, 7> l1 {"9", "8", "7"};
	FixedList<string, 7> l2 {"9", "6", "7"};
	REQUIRE(l1 != l2);
}

TEST_CASE("operator<") {
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "D"};
	REQUIRE(l1 < l2);
}

TEST_CASE("operator<=") {
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "D"};
	REQUIRE(l1 <= l2);

	FixedList<string, 7> l3 {"A", "B", "C"};
	FixedList<string, 7> l4 {"A", "B", "C"};
	REQUIRE(l3 <= l4);
}

TEST_CASE("operator>") {
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "B"};
	REQUIRE(l1 > l2);
}

TEST_CASE("operator>=") {
	FixedList<string, 7> l1 {"A", "B", "C"};
	FixedList<string, 7> l2 {"A", "B", "B"};
	REQUIRE(l1 >= l2);

	FixedList<string, 7> l3 {"A", "B", "C"};
	FixedList<string, 7> l4 {"A", "B", "C"};
	REQUIRE(l3 >= l4);
}

TEST_CASE("operator<=>") {
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

TEST_CASE("operator[]") {
	FixedList<string, 7> l {"X", "Y", "Z"};
	REQUIRE(l[2] == "Z");

	l[2] = "_";
	REQUIRE(l[2] != "Z");
	REQUIRE(l[2] == "_");

	const FixedList<string, 7> l2 = {};
	static_assert(is_const_v<remove_reference_t<decltype(l2[0])>>);
}

TEST_CASE("at") {
	FixedList<string, 7> l {"X", "Y", "Z"};

	bool thrown = false;
	try {
		l.at(3);
	} catch (out_of_range) {
		thrown = true;
	}
	REQUIRE(thrown);

	REQUIRE(l.at(1) == "Y");
}

TEST_CASE("get") {
	FixedList<string, 7> l {"X", "Y", "Z"};

	REQUIRE(l.get(3) == nullptr);
	REQUIRE(l.get(6) == nullptr);
	REQUIRE(*l.get(2) == "Z");

	*l.get(1) = ".";
	REQUIRE(*l.get(1) != "Z");
	REQUIRE(*l.get(1) == ".");
}

TEST_CASE("front") {
	FixedList<string, 7> l {"X", "Y", "Z"};

	REQUIRE(l.front() == "X");
	l.front() = "A";
	REQUIRE(l.front() == "A");
}

TEST_CASE("back") {
	FixedList<string, 7> l {"X", "Y", "Z"};

	REQUIRE(l.back() == "Z");
	l.back() = "C";
	REQUIRE(l.back() == "C");
}

TEST_CASE("assign(count,value)") {
	FixedList<string, 7> l {"X", "Y", "Z"};
	REQUIRE(l[0] == "X");
	REQUIRE(l[1] == "Y");
	REQUIRE(l[2] == "Z");

	l.assign(7, "...");

	REQUIRE(l.size() == 7);
	for (auto& s : l)
		REQUIRE(s == "...");
}

class TestException : public std::exception {};

TEST_CASE("assign(count,value) throw") {
	static unsigned counter = 2;

	struct X {
		X() = default;

		X(const X&) {
			if (!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool thrown = false;

	FixedList<X, 9> l(3);
	try {
		l.assign(5, {});
	} catch (TestException) {
		thrown = true;
		REQUIRE(l.empty());
	}
	REQUIRE(thrown);
}

TEST_CASE("assign(first,last) forward") {
	FixedList<string, 7> l {"X", "Y", "Z"};
	REQUIRE(l[0] == "X");
	REQUIRE(l[1] == "Y");
	REQUIRE(l[2] == "Z");

	vector<string> v {"4", "5", "6", "7", "8", "9", "10"};
	l.assign(v.begin() + 1, v.end());

	REQUIRE(l.size() == 6);

	auto vb = v.begin() + 1;
	for (auto& s : l)
		REQUIRE(s == *vb++);
}

TEST_CASE("assign(first,last) forward throw") {
	static unsigned counter = 2;

	struct X {
		X() = default;

		X(const X&) {
			if (!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool thrown = false;

	FixedList<X, 9> l(3);
	FixedList<X, 9> l2(3);
	try {
		l.assign(l2.begin(), l2.end());
	} catch (TestException) {
		thrown = true;
		REQUIRE(l.empty());
	}
	REQUIRE(thrown);
}

TEST_CASE("assign(first,last) input") {
	vector<unique_ptr<string>> v;
	v.emplace_back(make_unique<string>("A"));
	v.emplace_back(make_unique<string>("B"));
	v.emplace_back(make_unique<string>("C"));
	v.emplace_back(make_unique<string>("D"));

	auto move_begin = make_move_iterator(v.begin() + 1);
	auto move_end	= make_move_iterator(v.end());

	FixedList<unique_ptr<string>, 7> l;
	l.emplace_back(make_unique<string>("X"));
	l.emplace_back(make_unique<string>("Y"));
	l.emplace_back(make_unique<string>("Z"));

	l.assign(move_begin, move_end);

	REQUIRE(l.size() == 3);
	REQUIRE(*l[0] == "B");
	REQUIRE(*l[1] == "C");
	REQUIRE(*l[2] == "D");
}

TEST_CASE("assign(first,last) input throw") {
	static unsigned counter = 8;

	struct X {
		X() = default;

		X(X&&) {
			if (!counter--)
				throw TestException();
		}
	};

	vector<X> v;
	v.emplace_back();
	v.emplace_back();
	v.emplace_back();
	v.emplace_back();

	auto move_begin = make_move_iterator(v.begin() + 1);
	auto move_end	= make_move_iterator(v.end());

	FixedList<X, 7> l;
	l.emplace_back();
	l.emplace_back();
	l.emplace_back();
	REQUIRE(l.size() == 3);

	bool thrown = false;
	try {
		l.assign(move_begin, move_end);
	} catch (TestException) {
		thrown = true;
		REQUIRE(l.empty());
	}
	REQUIRE(thrown);
}

TEST_CASE("assign(init)") {
	FixedList<string, 10> l(5);
	REQUIRE(l.size() == 5);

	for (auto& s : l)
		REQUIRE(s.empty());

	l.assign({"A", "B", "C"});
	REQUIRE(l[0] == "A");
	REQUIRE(l[1] == "B");
	REQUIRE(l[2] == "C");
}

TEST_CASE("assign(init) throw") {
	static unsigned counter = 1;

	struct X {
		X() = default;

		X(const X&) {
			if (!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool thrown = false;

	FixedList<X, 9> l(3);
	try {
		l.assign({X(), X(), X()});
	} catch (TestException) {
		thrown = true;
		REQUIRE(l.empty());
	}
	REQUIRE(thrown);
}

TEST_CASE("insert(pos,value)") {
	FixedList<string, 11> l {"A", "B", "D", "E"};

	auto it = l.insert(l.begin() + 2, "C");
	REQUIRE(l[2] == "C");
	REQUIRE(*it == "C");

	array arr {"A", "B", "C", "D", "E"};
	REQUIRE(l == arr);
}

TEST_CASE("insert(pos,value) throw") {
	static unsigned counter = 3;

	struct X {
		string s;

		X(const char* s) :
			s(s) {
		}

		X(const X& x) :
			s(x.s) {
			if (!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool thrown = false;

	FixedList<X, 9> l {"1", "2", "3"};
	try {
		X x("NOPE");
		l.insert(l.begin(), x); // Info: This line currently causes the false warning about unreachable code in MSVC.
	} catch (TestException) {
		thrown = true;

		array arr {"1", "2", "3"};
		auto  i = l.begin();
		for (auto str : arr)
			REQUIRE(i++->s == str);
	}
	REQUIRE(thrown);
}

TEST_CASE("try_insert(pos,value)") {
	FixedList<string, 4> l {"A", "B", "D", "E"};

	auto it = l.try_insert(l.begin(), "C");
	REQUIRE(it == l.end());
}

TEST_CASE("insert(pos,count,value)") {
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

TEST_CASE("insert(pos,count,value) throw") {
	static unsigned counter = 4;

	struct X {
		string s;

		X(const char* s) :
			s(s) {
		}

		X(const X& x) :
			s(x.s) {
			if (!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool thrown = false;

	FixedList<X, 9> l {"1", "2", "3"};
	try {
		X x("6");
		l.insert(l.begin(), 5, x);
	} catch (TestException) {
		thrown = true;

		array arr {"1", "2", "3"};
		auto  i = l.begin();
		for (auto str : arr)
			REQUIRE(i++->s == str);
	}
	REQUIRE(thrown);
}

TEST_CASE("try_insert(pos,count,value)") {
	FixedList<string, 8> l {"B", "C", "D", "E"};

	auto it = l.try_insert(l.begin(), 5, "A");
	REQUIRE(it == l.end());
}

TEST_CASE("insert(pos,first,last) forward") {
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

TEST_CASE("insert(pos,first,last) forward throw") {
	static unsigned counter = 4;

	struct X {
		string s;

		X(const char* s) :
			s(s) {
		}

		X(const X& x) :
			s(x.s) {
			if (!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool thrown = false;

	FixedList<X, 9> l {"A", "B", "C"};
	try {
		vector v {X("ok"), X("ok"), X("no"), X("oops"), X("well...")};
		l.insert(l.begin(), v.begin(), v.end() - 2);
	} catch (TestException) {
		thrown = true;

		array arr {"A", "B", "C"};
		auto  i = l.begin();
		for (auto str : arr)
			REQUIRE(i++->s == str);
	}
	REQUIRE(thrown);
}

TEST_CASE("try_insert(pos,first,last) forward") {
	FixedList<string, 8> l {"A", "E", "F", "G", "H", "I"};

	vector<string> v {"A", "B", "C", "D", "E", "F"};

	auto it = l.try_insert(l.begin() + 1, v.begin() + 1, v.end() - 2);
	REQUIRE(it == l.end());
}

TEST_CASE("insert(pos,first,last) input") {
	vector<unique_ptr<string>> v;
	v.emplace_back(make_unique<string>("B"));
	v.emplace_back(make_unique<string>("C"));
	v.emplace_back(make_unique<string>("D"));
	v.emplace_back(make_unique<string>("E"));

	auto move_begin = make_move_iterator(v.begin() + 1);
	auto move_end	= make_move_iterator(v.end());

	FixedList<unique_ptr<string>, 7> l;
	l.emplace_back(make_unique<string>("A"));
	l.emplace_back(make_unique<string>("B"));
	l.emplace_back(make_unique<string>("F"));

	auto it = l.insert(l.begin() + 2, move_begin, move_end);

	REQUIRE(l.size() == 6);
	REQUIRE(**it == "C");
	REQUIRE(*l[0] == "A");
	REQUIRE(*l[1] == "B");
	REQUIRE(*l[2] == "C");
	REQUIRE(*l[3] == "D");
	REQUIRE(*l[4] == "E");
	REQUIRE(*l[5] == "F");
}

struct InputThrowTest {
	static inline unsigned counter = 4;

	string s;

	friend istream& operator>>(istream& stream, InputThrowTest& val) {
		if (!counter--)
			throw TestException();

		return stream >> val.s;
	}
};

TEST_CASE("insert(pos,first,last) input throw") {
	istringstream stream("1 2 3 4 5 6");

	istream_iterator<InputThrowTest> it(stream);

	FixedList<InputThrowTest, 7> l;
	l.emplace_back("1");
	l.emplace_back("2");
	l.emplace_back("3");

	bool thrown = false;
	try {
		l.insert(l.begin() + 2, it, {});
	} catch (TestException) {
		thrown = true;
		REQUIRE(l.size() == 3);
		REQUIRE(l[0].s == "1");
		REQUIRE(l[1].s == "2");
		REQUIRE(l[2].s == "3");
	}
	REQUIRE(thrown);
}

struct InputTest {
	string s;

	InputTest() = default;

	InputTest(const char* s) :
		s(s) {
	}

	friend istream& operator>>(istream& stream, InputTest& val) {
		return stream >> val.s;
	}
};

TEST_CASE("try_insert(pos,first,last) input") {
	FixedList<InputTest, 8> l {"A", "B", "C", "D", "E", "F"};

	istringstream stream("1 2 3 4 5 6");

	istream_iterator<InputTest> stream_it(stream);

	auto it = l.try_insert(l.begin() + 1, stream_it, {});
	REQUIRE(it == l.end());
	REQUIRE(l.size() == 6);
	REQUIRE(l[0].s == "A");
	REQUIRE(l[1].s == "B");
	REQUIRE(l[2].s == "C");
	REQUIRE(l[3].s == "D");
	REQUIRE(l[4].s == "E");
	REQUIRE(l[5].s == "F");
}

TEST_CASE("insert(pos,init)") {
	FixedList<string, 15> l {"AAA", "BBB", "CCC"};

	auto it = l.insert(l.begin() + 3, {"A", "B", "C"});
	REQUIRE(l[0] == "AAA");
	REQUIRE(l[1] == "BBB");
	REQUIRE(l[2] == "CCC");
	REQUIRE(l[3] == "A");
	REQUIRE(l[4] == "B");
	REQUIRE(l[5] == "C");
	REQUIRE(l.size() == 6);
	REQUIRE(*it == "A");
}

TEST_CASE("insert(pos,init) throw") {
	static unsigned counter = 4;

	struct X {
		string s;

		X(const char* s) :
			s(s) {
		}

		X(const X& x) :
			s(x.s) {
			if (!counter--)
				throw TestException();
		}

		X(X&&) = default;
	};

	bool thrown = false;

	FixedList<X, 9> l {"A", "B", "C"};
	try {
		l.insert(l.begin(), {X("ok"), X("ok"), X("no"), X("oops"), X("well...")});
	} catch (TestException) {
		thrown = true;

		array arr {"A", "B", "C"};
		auto  i = l.begin();
		for (auto str : arr)
			REQUIRE(i++->s == str);
	}
	REQUIRE(thrown);
}

TEST_CASE("try_insert(pos,init)") {
	FixedList<string, 8> l {"A", "E", "F", "G", "H", "I"};

	auto it = l.try_insert(l.begin() + 1, {"A", "B", "C", "D", "E", "F"});
	REQUIRE(it == l.end());
}

TEST_CASE("emplace") {
	FixedList<string, 5> l {"AAA", "BBB", "CCC"};

	auto it = l.emplace(l.begin() + 1, "XXX");
	REQUIRE(l[0] == "AAA");
	REQUIRE(l[1] == "XXX");
	REQUIRE(l[2] == "BBB");
	REQUIRE(l[3] == "CCC");
	REQUIRE(*it == "XXX");
	REQUIRE(l.size() == 4);
}

TEST_CASE("emplace throw") {
	static unsigned counter = 3;

	struct X {
		int x;

		X(int x) :
			x(x) {
			if (!counter--)
				throw TestException();
		}
	};

	bool thrown = false;

	FixedList<X, 9> l {3, 4, 5};
	try {
		l.emplace(l.begin(), 999);
	} catch (TestException) {
		thrown = true;

		array arr {3, 4, 5};
		auto  i = l.begin();
		for (int n : arr)
			REQUIRE(i++->x == n);
	}
	REQUIRE(thrown);
}

TEST_CASE("try_emplace") {
	FixedList<string, 5> l {"AAA", "BBB", "CCC", "DDD", "EEE"};

	auto it = l.try_emplace(l.begin() + 1, "XXX");
	REQUIRE(it == l.end());
}

TEST_CASE("emplace_back") {
	FixedList<string, 5> l {"AAA", "BBB", "CCC", "DDD"};

	auto& elem = l.emplace_back("EEE");
	REQUIRE(elem == "EEE");
	REQUIRE(l[4] == "EEE");
	REQUIRE(l.size() == 5);
}

TEST_CASE("emplace_back throw") {
	struct X {
		int x = 3;

		X() = default;

		X(int) {
			throw TestException();
		}
	};

	bool thrown = false;

	FixedList<X, 9> l(5);
	try {
		l.emplace_back(-1);
	} catch (TestException) {
		thrown = true;

		array arr {3, 3, 3, 3, 3};
		auto  i = arr.begin();
		for (X n : l)
			REQUIRE(*i++ == n.x);
		REQUIRE(l.size() == 5);
	}
	REQUIRE(thrown);
}

TEST_CASE("try_emplace_back") {
	FixedList<string, 5> l {"AAA", "BBB", "CCC", "DDD", "EEE"};

	auto it = l.try_emplace_back("XXX");
	REQUIRE(!it);
}

TEST_CASE("pop_back") {
	FixedList<string, 5> l {"AAA", "BBB", "CCC", "DDD", "EEE"};

	l.pop_back();
	REQUIRE(l.size() == 4);
	REQUIRE(l[0] == "AAA");
	REQUIRE(l[1] == "BBB");
	REQUIRE(l[2] == "CCC");
	REQUIRE(l[3] == "DDD");
}

TEST_CASE("try_pop_back") {
	FixedList<string, 5> l {"AAA", "BBB", "CCC", "DDD", "EEE"};

	REQUIRE(l.try_pop_back());
	REQUIRE(l.try_pop_back());
	REQUIRE(l.try_pop_back());
	REQUIRE(l.try_pop_back());
	REQUIRE(l.try_pop_back());
	REQUIRE(!l.try_pop_back());
}

TEST_CASE("erase(pos)") {
	FixedList<int, 10> l {2, 3, 4, 5, 6, 7, 8, 9};

	auto it = l.erase(l.begin() + 2);
	REQUIRE(*it == 5);
	REQUIRE(l[1] == 3);
	REQUIRE(l[2] == 5);
}

TEST_CASE("erase(first,last)") {
	FixedList<int, 10> l {2, 3, 4, 5, 6, 7, 8, 9};

	auto it = l.erase(l.begin() + 2, l.end());

	vector<int> v {2, 3};
	REQUIRE(l == v);
	REQUIRE(it == l.end());
}

TEST_CASE("clear") {
	static unsigned counter = 4;

	struct X {
		~X() {
			--counter;
		}
	};

	FixedList<X, 5> l(4);

	l.clear();

	REQUIRE(l.empty());
	REQUIRE(counter == 0);
}
