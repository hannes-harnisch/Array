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

	CHECK(a.data() == nullptr);
}

TEST_CASE("ConstructorWithNonDefaultConstructibleType")
{
	struct A
	{
		A(int)
		{}
	};
	static_assert(!std::is_default_constructible_v<A>);

	Array<A> a(5);
	CHECK(a.data());
}

TEST_CASE("ConstructorWithInitialValue")
{
	constexpr char character = 'y';

	Array<std::string> a(30, 5, 'y');
	for(auto& str : a)
		CHECK(str == "yyyyy");

	Array<std::string> b(30, std::string("DONTMOVEINALOOP"));
	for(auto& str : b)
		CHECK(str == "DONTMOVEINALOOP");
}

TEST_CASE("ConstructorWithInitializerList")
{
	std::initializer_list<unsigned char> const initializerList = {3, 4, 5};
	Array<unsigned char>					   a(25, initializerList);

	auto element = a.begin();
	for(int value : initializerList)
		CHECK(*element++ == value);
}

TEST_CASE("ConstructorWithInitializerListAndDefaultValue")
{
	std::initializer_list<uint16_t> const initializerList {3, 4, 5};
	constexpr uint16_t					  defaultValue = 100;
	Array<uint16_t>						  a(25, initializerList, defaultValue);

	auto element = a.begin();
	for(uint16_t value : initializerList)
		CHECK(*element++ == value);

	for(size_t i = initializerList.size(); i < a.size(); ++i)
		CHECK(a[i] == defaultValue);
}

TEST_CASE("CopyConstructor")
{
	Array<float> a(20, 6.66f);
	auto		 b(a);
	CHECK(a.size() == b.size());

	auto elementA = a.begin();
	for(float elementB : b)
		CHECK(elementB == *elementA++);
}

TEST_CASE("MoveConstructor")
{
	constexpr size_t testSize	  = 50;
	constexpr double initialValue = 1.25;

	Array<double> a(testSize, initialValue);
	auto		  dataPtr = a.data();
	auto		  b		  = std::move(a);

	CHECK(b.size() == testSize);
	CHECK(b.data() == dataPtr);
	CHECK(a.data() == nullptr);
	for(double element : b)
		CHECK(element == initialValue);
}

TEST_CASE("CopyAssignment")
{
	Array<short> a(20, static_cast<short>(256));
	Array<short> b(40, static_cast<short>(512));
	b = a;

	CHECK(a.size() == b.size());
	auto elementB = b.begin();
	for(short elementA : a)
		CHECK(elementA == *elementB++);
}

TEST_CASE("MoveAssignment")
{
	constexpr size_t sizeA		   = 58;
	constexpr char	 initialValueA = 'x';

	Array<char> a(sizeA, initialValueA);
	Array<char> b(10);
	auto		dataPtrA = a.data();
	b					 = std::move(a);

	CHECK(b.size() == sizeA);
	CHECK(b.data() == dataPtrA);
	for(char element : b)
		CHECK(element == initialValueA);
}

TEST_CASE("Subscript")
{
	constexpr size_t   indexA = 1234;
	constexpr size_t   indexB = 5678;
	constexpr size_t   indexC = 9000;
	constexpr unsigned valueA = 123;
	constexpr unsigned valueB = 456;
	constexpr unsigned valueC = 789;

	Array<unsigned> a(10000);
	a[indexA] = valueA;
	a[indexB] = valueB;
	a[indexC] = valueC;

	CHECK(a[indexA] == valueA);
	CHECK(a[indexB] == valueB);
	CHECK(a[indexC] == valueC);
}

TEST_CASE("Equality")
{
	constexpr size_t	indexA = 33;
	constexpr size_t	indexB = 333;
	constexpr size_t	indexC = 3333;
	constexpr long long valueA = 123933458;
	constexpr long long valueB = 1233457654;
	constexpr long long valueC = 12236353338;

	constexpr size_t size = 5000;
	Array<long long> a(size, 0);
	Array<long long> b(size, 0);
	a[indexA] = b[indexA] = valueA;
	a[indexB] = b[indexB] = valueB;
	a[indexC] = b[indexC] = valueC;
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

TEST_CASE("Release")
{
	Array<int> a(25);
	auto	   ptr = a.release();
	CHECK(a.data() == nullptr);
	delete ptr;
}

TEST_CASE("Reset")
{
	Array<long long> a(25);
	a.reset();
	CHECK(a.data() == nullptr);
}

TEST_CASE("Fill")
{
	constexpr int fillValue = 244;
	Array<int>	  a(10, fillValue);
	for(int element : a)
		CHECK(element == fillValue);
}

TEST_CASE("Swap")
{
	std::random_device random;
	Array<unsigned>	   a(5, {random(), random(), random()});
	Array<unsigned>	   b(5, {random(), random(), random()});
	auto			   c(a);
	auto			   d(b);

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
	Array<short>	 a(size);

	auto data = a.data();
	CHECK(data == a.begin().operator->());
	CHECK(data + size == a.end().operator->());
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
	Array<int>			 a(100);
	Array<int>::iterator it;
	CHECK(true);
}

TEST_CASE("IteratorDereference")
{
	Array<std::string> a(25, {"AA", "BB", "CC"});
	auto			   first = a.begin();
	CHECK(*first == "AA");
}

TEST_CASE("IteratorArrow")
{
	Array<std::string> a(25, {"AAA", "BB", "C"});
	auto			   first = a.begin();
	CHECK(first->length() == 3);
}

TEST_CASE("IteratorEquality")
{
	Array<long> a(30);
	auto		begin1 = a.begin();
	auto		begin2 = a.cbegin();
	auto		end1   = a.end();
	auto		end2   = a.cend();
	CHECK(begin1 == begin2);
	CHECK(end1 == end2);
}

TEST_CASE("IteratorInequality")
{
	Array<long> a(30);
	auto		getsIncremented = a.cbegin();
	auto		getsDecremented = a.cend();
	CHECK(a.begin() != ++getsIncremented);
	CHECK(a.end() != --getsDecremented);
}

TEST_CASE("IteratorLessThan")
{
	Array<long> const a(30);
	auto			  getsIncremented = a.begin();
	CHECK(a.begin() < ++getsIncremented);
}

TEST_CASE("IteratorGreaterThan")
{
	Array<long> a(30);
	auto		getsIncremented = a.begin();
	CHECK(++getsIncremented > a.begin());
}

TEST_CASE("IteratorLessThanOrEqual")
{
	Array<long> a(30);
	CHECK(a.begin() <= a.begin());
	auto getsIncremented = a.begin();
	CHECK(a.begin() <= ++getsIncremented);
}

TEST_CASE("IteratorGreaterThanOrEqual")
{
	Array<long> a(30);
	auto		getsIncremented = a.begin();
	CHECK(a.begin() >= a.begin());
	CHECK(++getsIncremented >= a.begin());
}

TEST_CASE("IteratorSpaceship")
{
#ifdef __cpp_lib_three_way_comparison
	Array<long> a(30);
	auto const	secondElement = a.begin() + 1;
	auto		getsMutated	  = a.begin() + 2;
	CHECK((getsMutated <=> secondElement) == std::strong_ordering::greater);
	CHECK((--getsMutated <=> secondElement) == std::strong_ordering::equal);
	CHECK((--getsMutated <=> secondElement) == std::strong_ordering::less);
#endif
}

TEST_CASE("IteratorPreIncrement")
{
	Array<long> a(3, {5, 6, 7});
	auto		getsIncremented = a.begin();
	CHECK(*++getsIncremented == 6);
}

TEST_CASE("IteratorPostIncrement")
{
	Array<long> a(3, {5, 6, 7});
	auto		incremented = a.begin();
	auto		first		= incremented++;
	CHECK(*first == 5);
	CHECK(*incremented == 6);
}

TEST_CASE("IteratorPreDecrement")
{
	Array<long> a(3, {5, 6, 7});
	auto		getsDecremented = a.end();
	CHECK(*--getsDecremented == 7);
}

TEST_CASE("IteratorPostDecrement")
{
	Array<long> a(3, {5, 6, 7});
	auto		getsDecremented = a.end();
	auto		end {getsDecremented--};
	CHECK(end != getsDecremented);
}

TEST_CASE("IteratorAdditionAssignment")
{
	Array<int> a(5, {10, 11, 12});
	auto	   begin = a.begin();
	CHECK(*(begin += 2) == 12);
	CHECK(*begin == 12);
}

TEST_CASE("IteratorAddition")
{
	Array<int> a(5, {10, 11, 12});
	auto	   second = a.begin() + 1;
	auto	   third {2 + a.begin()};
	CHECK(*second == 11);
	CHECK(*third == 12);
}

TEST_CASE("IteratorSubtractionAssignment")
{
	Array<int> a(3, {10, 11, 12});
	auto	   end = a.end();
	CHECK(*(end -= 2) == 11);
	CHECK(*end == 11);
}

TEST_CASE("IteratorSubtraction")
{
	Array<int> a(3, {10, 11, 12});
	auto	   last	  = a.end() - 1;
	auto	   offset = last - a.begin();
	CHECK(*last == 12);
	CHECK(size_t(offset) == a.size() - 1);
}

TEST_CASE("IteratorSubscript")
{
	Array<int> a(6, {11, 12, 13, 14, 15, 16});
	auto	   it = a.begin() + 2;
	CHECK(it[2] == 15);
}
