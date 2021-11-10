#include "../Include/Array.hpp"

#include <random>
#include <string>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

using namespace hh;

TEST_CASE("DefaultConstructor")
{
	Array<int> a;

	for([[maybe_unused]] int element : a)
		CHECK(false); // Check that there are no elements.

	static_assert(sizeof a == sizeof(void*) + sizeof(size_t));

	CHECK(a.data() == nullptr);
}

TEST_CASE("ConstructorWithInitialValue")
{
	Array<std::string> a(30, std::string(5, 'y'));
	for(auto& str : a)
		CHECK(str == "yyyyy");

	Array<std::string> b(30, std::string("DONTMOVEINALOOP"));
	for(auto& str : b)
		CHECK(str == "DONTMOVEINALOOP");
}

TEST_CASE("ConstructorWithInitializerList")
{
	std::initializer_list<unsigned char> const initializer_list = {3, 4, 5};

	Array<unsigned char> a(25, initializer_list);

	auto element = a.begin();
	for(int value : initializer_list)
		CHECK(*element++ == value);
}

TEST_CASE("ConstructorWithInitializerListAndDefaultValue")
{
	std::initializer_list<uint16_t> const initializer_list {3, 4, 5};

	constexpr uint16_t default_value = 100;
	Array<uint16_t>	   a(25, initializer_list, default_value);

	auto element = a.begin();
	for(uint16_t value : initializer_list)
		CHECK(*element++ == value);

	for(size_t i = initializer_list.size(); i < a.size(); ++i)
		CHECK(a[i] == default_value);
}

TEST_CASE("CopyConstructor")
{
	Array<float> a(20, 6.66f);
	auto		 b(a);
	CHECK(a.size() == b.size());

	auto element_a = a.begin();
	for(float element_b : b)
		CHECK(element_b == *element_a++);
}

TEST_CASE("MoveConstructor")
{
	constexpr size_t test_size	   = 50;
	constexpr double initial_value = 1.25;

	Array<double> a(test_size, initial_value);

	auto data = a.data();
	auto b	  = std::move(a);

	CHECK(b.size() == test_size);
	CHECK(b.data() == data);
	CHECK(a.data() == nullptr);
	for(double element : b)
		CHECK(element == initial_value);
}

TEST_CASE("CopyAssignment")
{
	Array<short> a(20, static_cast<short>(256));
	Array<short> b(40, static_cast<short>(512));
	b = a;

	CHECK(a.size() == b.size());
	auto element_b = b.begin();
	for(short element_a : a)
		CHECK(element_a == *element_b++);
}

TEST_CASE("MoveAssignment")
{
	constexpr size_t size_a			 = 58;
	constexpr char	 initial_value_a = 'x';

	Array<char> a(size_a, initial_value_a);
	Array<char> b(10);

	auto data_ptr_a = a.data();

	b = std::move(a);

	CHECK(b.size() == size_a);
	CHECK(b.data() == data_ptr_a);
	for(char element : b)
		CHECK(element == initial_value_a);
}

TEST_CASE("Subscript")
{
	constexpr size_t   index_a = 1234;
	constexpr size_t   index_b = 5678;
	constexpr size_t   index_c = 9000;
	constexpr unsigned value_a = 123;
	constexpr unsigned value_b = 456;
	constexpr unsigned value_c = 789;

	Array<unsigned> a(10000);
	a[index_a] = value_a;
	a[index_b] = value_b;
	a[index_c] = value_c;

	CHECK(a[index_a] == value_a);
	CHECK(a[index_b] == value_b);
	CHECK(a[index_c] == value_c);
}

TEST_CASE("Equality")
{
	constexpr size_t	index_a = 33;
	constexpr size_t	index_b = 333;
	constexpr size_t	index_c = 3333;
	constexpr long long value_a = 123933458;
	constexpr long long value_b = 1233457654;
	constexpr long long value_c = 12236353338;

	constexpr size_t size = 5000;
	Array<long long> a(size, 0);
	Array<long long> b(size, 0);
	a[index_a] = b[index_a] = value_a;
	a[index_b] = b[index_b] = value_b;
	a[index_c] = b[index_c] = value_c;
	CHECK(a == b);
}

TEST_CASE("Inequality")
{
	Array<size_t> a(1);
	Array<size_t> b(2);
	CHECK(a != b);

	constexpr size_t size = 5000;
	Array<size_t>	 c(size, 0);
	Array<size_t>	 d(size, 0);
	CHECK(c == d);

	c[123] = 456;
	CHECK(c != d);
}

TEST_CASE("LessThan")
{
	Array<long double> a(3, {1.5, 2.5, 4.5});
	Array<long double> b(3, {1.5, 2.5, 5.5});
	CHECK(a < b);
}

TEST_CASE("GreaterThan")
{
	Array<long double> a(3, {1.5, 2.5, 6.5});
	Array<long double> b(3, {1.5, 2.5, 5.5});
	CHECK(a > b);
}

TEST_CASE("LessThanOrEqual")
{
	Array<long double> a(3, {1.5, 2.5, 5.5});
	Array<long double> b(3, {1.5, 2.5, 5.5});
	CHECK(a <= b);
	Array<long double> c(3, {1.5, -3.5, 5.5});
	Array<long double> d(3, {1.5, 2.5, 5.5});
	CHECK(a <= b);
}

TEST_CASE("GreaterThanOrEqual")
{
	Array<long double> a(3, {1.5, 2.5, 5.5});
	Array<long double> b(3, {1.5, 2.5, 5.5});
	CHECK(a >= b);
	Array<long double> c(3, {1.5, 2.5, 5.5});
	Array<long double> d(3, {1.5, 2.5, 8.5});
	CHECK(a >= b);
}

TEST_CASE("Spaceship")
{
#ifdef __cpp_lib_three_way_comparison
	Array<int> a(4, {1, 2, 2, 0});
	Array<int> b(4, {1, 2, 3, 0});

	auto comp = a <=> b;
	CHECK(comp == std::strong_ordering::less);

	Array<int> c(4, {1, 2, 3, 0});
	Array<int> d(4, {1, 2, 3, 0});
	comp = c <=> d;
	CHECK(comp == std::strong_ordering::equal);

	Array<int> e(4, {1, 2, 4, 0});
	Array<int> f(4, {1, 2, 3, 0});
	comp = e <=> f;
	CHECK(comp == std::strong_ordering::greater);
#endif
}

TEST_CASE("At")
{
	Array<int> const a(10);
	try
	{
		a.at(10);
		CHECK(false);
	}
	catch(std::out_of_range)
	{
		CHECK(true);
	}
}

TEST_CASE("Get")
{
	Array<int> const a(10);
	CHECK(a.get(10) == nullptr);
}

TEST_CASE("Front")
{
	Array<int> a(10);
	a.front() = 256;
	CHECK(a.front() == 256);
	CHECK(a[0] == 256);
}

TEST_CASE("Back")
{
	Array<int> a(10);
	a.back() = 256;
	CHECK(a.back() == 256);
	CHECK(a[a.size() - 1] == 256);
}

TEST_CASE("Empty")
{
	Array<int> a;
	CHECK(a.empty());
}

TEST_CASE("MaxSize")
{
	Array<int> a;
	CHECK(a.max_size());
}

TEST_CASE("Reset")
{
	Array<long long> a(25);
	a.reset();
	CHECK(a.data() == nullptr);
}

TEST_CASE("Fill")
{
	constexpr int fill_value = 244;
	Array<int>	  a(10, fill_value);
	for(int element : a)
		CHECK(element == fill_value);
}

TEST_CASE("Swap")
{
	std::random_device random;

	Array<unsigned> a(5, {random(), random(), random()});
	Array<unsigned> b(5, {random(), random(), random()});

	auto c(a);
	auto d(b);

	a.swap(b);
	CHECK(a == d);
	CHECK(b == c);

	swap(a, b);
	CHECK(a == c);
	CHECK(b == d);
}

TEST_CASE("BeginAndEnd")
{
	constexpr size_t size = 25;

	Array<short> a(size);
	a.front() = 23;
	a.back()  = 47;

	CHECK(*a.begin() == 23);
	CHECK(a.end()[-1] == 47);
}

TEST_CASE("ReverseBeginAndEnd")
{
	Array<short> const a(3, std::initializer_list<short> {111, 222, 333});
	CHECK(*a.rbegin() == 333);
	CHECK(*--a.rend() == 111);

	Array<short> b(3, std::initializer_list<short> {444, 555, 666});
	CHECK(*b.rbegin() == 666);
	CHECK(*--b.rend() == 444);
}

TEST_CASE("IteratorContiguousProperty")
{
#ifdef __cpp_lib_concepts
	static_assert(std::contiguous_iterator<Array<float>::iterator>);
#else
	static_assert(std::is_same<std::iterator_traits<Array<float>::iterator>::iterator_category,
							   std::random_access_iterator_tag>::value,
				  "Iterator category not satisfied.");
#endif
}

TEST_CASE("IteratorDefaultConstructor")
{
	Array<int> a(100);

	Array<int>::iterator it;
	CHECK(true);
}

TEST_CASE("IteratorDereference")
{
	Array<std::string> a(25, {"AA", "BB", "CC"});

	auto first = a.begin();
	CHECK(*first == "AA");
}

TEST_CASE("IteratorArrow")
{
	Array<std::string> a(25, {"AAA", "BB", "C"});

	auto first = a.begin();
	CHECK(first->length() == 3);
}

TEST_CASE("IteratorEquality")
{
	Array<long> a(30);

	auto begin1 = a.begin();
	auto begin2 = a.cbegin();
	auto end1	= a.end();
	auto end2	= a.cend();
	CHECK(begin1 == begin2);
	CHECK(end1 == end2);
}

TEST_CASE("IteratorInequality")
{
	Array<long> a(30);

	auto gets_incremented = a.cbegin();
	auto gets_decremented = a.cend();
	CHECK(a.begin() != ++gets_incremented);
	CHECK(a.end() != --gets_decremented);
}

TEST_CASE("IteratorLessThan")
{
	Array<long> const a(30);

	auto gets_incremented = a.begin();
	CHECK(a.begin() < ++gets_incremented);
}

TEST_CASE("IteratorGreaterThan")
{
	Array<long> a(30);

	auto gets_incremented = a.begin();
	CHECK(++gets_incremented > a.begin());
}

TEST_CASE("IteratorLessThanOrEqual")
{
	Array<long> a(30);
	CHECK(a.begin() <= a.begin());

	auto gets_incremented = a.begin();
	CHECK(a.begin() <= ++gets_incremented);
}

TEST_CASE("IteratorGreaterThanOrEqual")
{
	Array<long> a(30);

	auto gets_incremented = a.begin();
	CHECK(a.begin() >= a.begin());
	CHECK(++gets_incremented >= a.begin());
}

TEST_CASE("IteratorSpaceship")
{
#ifdef __cpp_lib_three_way_comparison
	Array<long> a(30);
	auto const	second_element = a.begin() + 1;
	auto		gets_mutated   = a.begin() + 2;
	CHECK((gets_mutated <=> second_element) == std::strong_ordering::greater);
	CHECK((--gets_mutated <=> second_element) == std::strong_ordering::equal);
	CHECK((--gets_mutated <=> second_element) == std::strong_ordering::less);
#endif
}

TEST_CASE("IteratorPreIncrement")
{
	Array<long> a(3, {5, 6, 7});

	auto gets_incremented = a.begin();
	CHECK(*++gets_incremented == 6);
}

TEST_CASE("IteratorPostIncrement")
{
	Array<long> a(3, {5, 6, 7});

	auto incremented = a.begin();
	auto first		 = incremented++;
	CHECK(*first == 5);
	CHECK(*incremented == 6);
}

TEST_CASE("IteratorPreDecrement")
{
	Array<long> a(3, {5, 6, 7});

	auto gets_decremented = a.end();
	CHECK(*--gets_decremented == 7);
}

TEST_CASE("IteratorPostDecrement")
{
	Array<long> a(3, {5, 6, 7});

	auto gets_decremented = a.end();
	auto end {gets_decremented--};
	CHECK(end != gets_decremented);
}

TEST_CASE("IteratorAdditionAssignment")
{
	Array<int> a(5, {10, 11, 12});

	auto begin = a.begin();
	CHECK(*(begin += 2) == 12);
	CHECK(*begin == 12);
}

TEST_CASE("IteratorAddition")
{
	Array<int> a(5, {10, 11, 12});

	auto second = a.begin() + 1;
	auto third {2 + a.begin()};
	CHECK(*second == 11);
	CHECK(*third == 12);
}

TEST_CASE("IteratorSubtractionAssignment")
{
	Array<int> a(3, {10, 11, 12});

	auto end = a.end();
	CHECK(*(end -= 2) == 11);
	CHECK(*end == 11);
}

TEST_CASE("IteratorSubtraction")
{
	Array<int> a(3, {10, 11, 12});

	auto last	= a.end() - 1;
	auto offset = last - a.begin();
	CHECK(*last == 12);
	CHECK(size_t(offset) == a.size() - 1);
}

TEST_CASE("IteratorSubscript")
{
	Array<int> a(6, {11, 12, 13, 14, 15, 16});

	auto it = a.begin() + 2;
	CHECK(it[2] == 15);
}
