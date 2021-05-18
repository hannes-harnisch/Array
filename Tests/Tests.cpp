#include <random>
#include <string>

#include "Include/Array.hpp"

using namespace hh;

#define check(condition)                                                                                               \
	{                                                                                                                  \
		if(!(condition))                                                                                               \
			throw "Test failed.";                                                                                      \
	}

void testDefaultConstructor()
{
	Array<int> a;

	for(int element : a)
		check(false); // Check that there are no elements.

	check(a.data() == nullptr);
}

void testConstructorFromRawArray()
{
	constexpr size_t testSize {30};
	auto raw {new int[testSize]};
	Array<int> a {raw, testSize};

	check(a.size() == testSize);
	check(a.data() != nullptr);
	// raw array gets deleted here by array destructor
}

void testConstructorWithNonDefaultConstructibleType()
{
	struct A
	{
		A(int)
		{}
	};
	static_assert(!std::is_default_constructible_v<A>);

	Array<A> a {5};
	check(a.data());
}

void testConstructorWithInitialValue()
{
	constexpr auto initialValue {"STRING"};

	Array<std::string> a {30, initialValue};
	for(auto& str : a)
		check(str == initialValue);
}

void testConstructorWithInitializerList()
{
	const auto initializerList = {3, 4, 5};
	Array<unsigned char> a {25, initializerList};

	auto element {a.begin()};
	for(int value : initializerList)
		check(*element++ == value);
}

void testConstructorWithInitializerListAndDefaultValue()
{
	const std::initializer_list<uint16_t> initializerList {3, 4, 5};
	constexpr uint16_t defaultValue {100};
	Array<uint16_t> a {25, initializerList, defaultValue};

	auto element {a.begin()};
	for(uint16_t value : initializerList)
		check(*element++ == value);

	for(size_t i {initializerList.size()}; i < a.size(); ++i)
		check(a[i] == defaultValue);
}

void testCopyConstructor()
{
	Array<float> a {20, 6.66f};
	auto b {a};
	check(a.size() == b.size());

	auto elementA {a.begin()};
	for(float elementB : b)
		check(elementB == *elementA++);
}

void testMoveConstructor()
{
	constexpr size_t testSize {50};
	constexpr double initialValue {1.25};

	Array<double> a {testSize, initialValue};
	auto dataPtr {a.data()};
	auto b {std::move(a)};

	check(b.size() == testSize);
	check(b.data() == dataPtr);
	check(a.data() == nullptr);
	for(double element : b)
		check(element == initialValue);
}

void testCopyAssignment()
{
	Array<short> a {20, 256};
	Array<short> b {40, 512};
	b = a;

	check(a.size() == b.size());
	auto elementB {b.begin()};
	for(short elementA : a)
		check(elementA == *elementB++);
}

void testMoveAssignment()
{
	constexpr size_t sizeA {58};
	constexpr char initialValueA {'x'};

	Array<char> a {sizeA, initialValueA};
	Array<char> b {10};
	auto dataPtrA {a.data()};
	b = std::move(a);

	check(b.size() == sizeA);
	check(b.data() == dataPtrA);
	for(char element : b)
		check(element == initialValueA);
}

void testSubscript()
{
	constexpr size_t indexA {1234};
	constexpr size_t indexB {5678};
	constexpr size_t indexC {9000};
	constexpr unsigned valueA {123};
	constexpr unsigned valueB {456};
	constexpr unsigned valueC {789};

	Array<unsigned> a {10000};
	a[indexA] = valueA;
	a[indexB] = valueB;
	a[indexC] = valueC;

	check(a[indexA] == valueA);
	check(a[indexB] == valueB);
	check(a[indexC] == valueC);
}

void testEquality()
{
	constexpr size_t indexA {33};
	constexpr size_t indexB {333};
	constexpr size_t indexC {3333};
	constexpr long long valueA {123933458};
	constexpr long long valueB {1233457654};
	constexpr long long valueC {12236353338};

	constexpr size_t size {5000};
	Array<long long> a {size, 0};
	Array<long long> b {size, 0};
	a[indexA] = b[indexA] = valueA;
	a[indexB] = b[indexB] = valueB;
	a[indexC] = b[indexC] = valueC;
	check(a == b);
}

void testInequality()
{
	Array<size_t> a {1};
	Array<size_t> b {2};
	check(a != b);

	constexpr size_t size {5000};
	Array<size_t> c {size, 0};
	Array<size_t> d {size, 0};
	check(c == d);

	c[123] = 456;
	check(c != d);
}

void testLessThan()
{
	Array<long double> a {3, {1.5, 2.5, 4.5}};
	Array<long double> b {3, {1.5, 2.5, 5.5}};
	check(a < b);
}

void testGreaterThan()
{
	Array<long double> a {3, {1.5, 2.5, 6.5}};
	Array<long double> b {3, {1.5, 2.5, 5.5}};
	check(a > b);
}

void testLessThanOrEqual()
{
	Array<long double> a {3, {1.5, 2.5, 5.5}};
	Array<long double> b {3, {1.5, 2.5, 5.5}};
	check(a <= b);
	Array<long double> c {3, {1.5, -3.5, 5.5}};
	Array<long double> d {3, {1.5, 2.5, 5.5}};
	check(a <= b);
}

void testGreaterThanOrEqual()
{
	Array<long double> a {3, {1.5, 2.5, 5.5}};
	Array<long double> b {3, {1.5, 2.5, 5.5}};
	check(a >= b);
	Array<long double> c {3, {1.5, 2.5, 5.5}};
	Array<long double> d {3, {1.5, 2.5, 8.5}};
	check(a >= b);
}

void testSpaceship()
{
#ifdef __cpp_lib_three_way_comparison
	Array<int> a {4, {1, 2, 2, 0}};
	Array<int> b {4, {1, 2, 3, 0}};
	check(a <=> b == std::strong_ordering::less);

	Array<int> c {4, {1, 2, 3, 0}};
	Array<int> d {4, {1, 2, 3, 0}};
	check(c <=> d == std::strong_ordering::equal);

	Array<int> e {4, {1, 2, 4, 0}};
	Array<int> f {4, {1, 2, 3, 0}};
	check(e <=> f == std::strong_ordering::greater);
#endif
}

void testAt()
{
	const Array<int> a {10};
	try
	{
		check(a.at(10) && false);
	}
	catch(std::out_of_range)
	{}
}

void testFront()
{
	Array<int> a {10};
	a.front() = 256;
	check(a.front() == 256);
	check(a[0] == 256);
}

void testBack()
{
	Array<int> a {10};
	a.back() = 256;
	check(a.back() == 256);
	check(a[a.size() - 1] == 256);
}

void testEmpty()
{
	Array<int> a;
	check(a.empty());
}

void testMaxSize()
{
	Array<int> a;
	check(a.max_size());
}

void testSwap()
{
	std::random_device random;
	Array<unsigned> a {5, {random(), random(), random()}};
	Array<unsigned> b {5, {random(), random(), random()}};
	auto c {a};
	auto d {b};

	a.swap(b);
	check(a == d);
	check(b == c);

	swap(a, b);
	check(a == c);
	check(b == d);
}

void testFill()
{
	constexpr int fillValue {244};
	Array<int> a {10, fillValue};
	for(int element : a)
		check(element == fillValue);
}

void testBeginAndEnd()
{
	constexpr size_t size {25};
	Array<short> a {size};

	auto data {a.data()};
	check(data == a.begin().operator->());
	check(data + size == a.end().operator->());
}

void testReverseBeginAndEnd()
{
	const Array<short> a {3, {111, 222, 333}};
	check(*a.rbegin() == 333);
	check(*--a.rend() == 111);

	Array<short> b {3, {444, 555, 666}};
	check(*b.rbegin() == 666);
	check(*--b.rend() == 444);
}

void testIteratorContiguousProperty()
{
#ifdef __cpp_lib_concepts
	static_assert(std::contiguous_iterator<Array<float>::iterator>);
#else
	static_assert(std::is_same<std::iterator_traits<Array<float>::iterator>::iterator_category,
							   std::random_access_iterator_tag>::value,
				  "Iterator category not satisfied.");
#endif
}

void testIteratorDefaultConstructor()
{
	Array<int> a {100};
	Array<int>::iterator it;
	check(it.operator->() == nullptr);
}

void testIteratorDereference()
{
	Array<std::string> a {25, {"AA", "BB", "CC"}};
	auto first {a.begin()};
	check(*first == "AA");
}

void testIteratorArrow()
{
	Array<std::string> a {25, {"AAA", "BB", "C"}};
	auto first {a.begin()};
	check(first->length() == 3);
}

void testIteratorEquality()
{
	Array<long> a {30};
	auto begin1 {a.begin()};
	auto begin2 {a.cbegin()};
	auto end1 {a.end()};
	auto end2 {a.cend()};
	check(begin1 == begin2);
	check(end1 == end2);
}

void testIteratorInequality()
{
	Array<long> a {30};
	auto getsIncremented {a.cbegin()};
	auto getsDecremented {a.cend()};
	check(a.begin() != ++getsIncremented);
	check(a.end() != --getsDecremented);
}

void testIteratorLessThan()
{
	const Array<long> a {30};
	auto getsIncremented {a.begin()};
	check(a.begin() < ++getsIncremented);
}

void testIteratorGreaterThan()
{
	Array<long> a {30};
	auto getsIncremented {a.begin()};
	check(++getsIncremented > a.begin());
}

void testIteratorLessThanOrEqual()
{
	Array<long> a {30};
	check(a.begin() <= a.begin());
	auto getsIncremented {a.begin()};
	check(a.begin() <= ++getsIncremented);
}

void testIteratorGreaterThanOrEqual()
{
	Array<long> a {30};
	auto getsIncremented {a.begin()};
	check(a.begin() >= a.begin());
	check(++getsIncremented >= a.begin());
}

void testIteratorSpaceship()
{
#ifdef __cpp_lib_three_way_comparison
	Array<long> a {30};
	const auto secondElement {a.begin() + 1};
	auto getsMutated {a.begin() + 2};
	check((getsMutated <=> secondElement) == std::strong_ordering::greater);
	check((--getsMutated <=> secondElement) == std::strong_ordering::equal);
	check((--getsMutated <=> secondElement) == std::strong_ordering::less);
#endif
}

void testIteratorPreIncrement()
{
	Array<long> a {3, {5, 6, 7}};
	auto getsIncremented {a.begin()};
	check(*++getsIncremented == 6);
}

void testIteratorPostIncrement()
{
	Array<long> a {3, {5, 6, 7}};
	auto incremented {a.begin()};
	auto first {incremented++};
	check(*first == 5);
	check(*incremented == 6);
}

void testIteratorPreDecrement()
{
	Array<long> a {3, {5, 6, 7}};
	auto getsDecremented {a.end()};
	check(*--getsDecremented == 7);
}

void testIteratorPostDecrement()
{
	Array<long> a {3, {5, 6, 7}};
	auto getsDecremented {a.end()};
	auto end {getsDecremented--};
	check(end != getsDecremented);
}

void testIteratorAdditionAssignment()
{
	Array<int> a {5, {10, 11, 12}};
	auto begin {a.begin()};
	check(*(begin += 2) == 12);
	check(*begin == 12);
}

void testIteratorAddition()
{
	Array<int> a {5, {10, 11, 12}};
	auto second {a.begin() + 1};
	auto third {2 + a.begin()};
	check(*second == 11);
	check(*third == 12);
}

void testIteratorSubtractionAssignment()
{
	Array<int> a {3, {10, 11, 12}};
	auto end {a.end()};
	check(*(end -= 2) == 11);
	check(*end == 11);
}

void testIteratorSubtraction()
{
	Array<int> a {3, {10, 11, 12}};
	auto last {a.end() - 1};
	auto offset {last - a.begin()};
	check(*last == 12);
	check(offset == a.size() - 1);
}

void testIteratorSubscript()
{
	Array<int> a {6, {11, 12, 13, 14, 15, 16}};
	auto it {a.begin() + 2};
	check(it[2] == 15);
}

int main()
{
	testDefaultConstructor();
	testConstructorFromRawArray();
	testConstructorWithNonDefaultConstructibleType();
	testConstructorWithInitialValue();
	testConstructorWithInitializerList();
	testConstructorWithInitializerListAndDefaultValue();
	testCopyConstructor();
	testMoveConstructor();
	testCopyAssignment();
	testMoveAssignment();
	testSubscript();
	testEquality();
	testInequality();
	testLessThan();
	testGreaterThan();
	testLessThanOrEqual();
	testGreaterThanOrEqual();
	testSpaceship();
	testAt();
	testFront();
	testBack();
	testEmpty();
	testMaxSize();
	testSwap();
	testFill();
	testBeginAndEnd();
	testReverseBeginAndEnd();
	testIteratorContiguousProperty();
	testIteratorDefaultConstructor();
	testIteratorDereference();
	testIteratorArrow();
	testIteratorEquality();
	testIteratorInequality();
	testIteratorLessThan();
	testIteratorGreaterThan();
	testIteratorLessThanOrEqual();
	testIteratorGreaterThanOrEqual();
	testIteratorSpaceship();
	testIteratorPreIncrement();
	testIteratorPostIncrement();
	testIteratorPreDecrement();
	testIteratorPostDecrement();
	testIteratorAdditionAssignment();
	testIteratorAddition();
	testIteratorSubtractionAssignment();
	testIteratorSubtraction();
	testIteratorSubscript();

	std::printf("All tests passed.\n");
}
