#include <random>
#include <string>

#define ARRAY_NAMESPACE array
#include "Include/Array.hh"

using namespace array;

void testDefaultConstructor()
{
	Array<int> a;

	for(int element : a)
		assert(false); // Check that there are no elements.

	assert(a.data() == nullptr);
}

void testConstructorFromRawArray()
{
	constexpr size_t testSize {30};
	auto raw {new int[testSize]};
	Array<int> a {raw, testSize};

	assert(a.size() == testSize);
	assert(a.data() != nullptr);
	// raw array gets deleted here by array destructor
}

void testConstructorWithInitialValue()
{
	constexpr auto initialValue {"STRING"};

	Array<std::string> a {30, initialValue};
	for(auto& str : a)
		assert(str == initialValue);
}

void testConstructorWithInitializerList()
{
	const auto initializerList = {3, 4, 5};
	Array<unsigned char> a {25, initializerList};

	auto element {a.begin()};
	for(int value : initializerList)
		assert(*element++ == value);
}

void testCopyConstructor()
{
	Array<float> a {20, 6.66f};
	auto b {a};
	assert(a.size() == b.size());

	auto elementA {a.begin()};
	for(float elementB : b)
		assert(elementB == *elementA++);
}

void testMoveConstructor()
{
	constexpr size_t testSize {50};
	constexpr double initialValue {1.25};

	Array<double> a {testSize, initialValue};
	auto dataPtr {a.data()};
	auto b {std::move(a)};

	assert(b.size() == testSize);
	assert(b.data() == dataPtr);
	assert(a.data() == nullptr);
	for(double element : b)
		assert(element == initialValue);
}

void testCopyAssignment()
{
	Array<short> a {20, 256};
	Array<short> b {40, 512};
	b = a;

	assert(a.size() == b.size());
	auto elementB {b.begin()};
	for(short elementA : a)
		assert(elementA == *elementB++);
}

void testMoveAssignment()
{
	constexpr size_t sizeA {58};
	constexpr char initialValueA {'x'};

	Array<char> a {sizeA, initialValueA};
	Array<char> b {10};
	auto dataPtrA {a.data()};
	auto dataPtrB {b.data()};
	b = std::move(a);

	assert(b.size() == sizeA);
	assert(b.data() == dataPtrA);
	assert(a.data() == dataPtrB);
	for(char element : b)
		assert(element == initialValueA);
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

	assert(a[indexA] == valueA);
	assert(a[indexB] == valueB);
	assert(a[indexC] == valueC);
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
	Array<long long> a {size};
	Array<long long> b {size};
	a[indexA] = b[indexA] = valueA;
	a[indexB] = b[indexB] = valueB;
	a[indexC] = b[indexC] = valueC;
	assert(a == b);
}

void testInequality()
{
	Array<size_t> a {1};
	Array<size_t> b {2};
	assert(a != b);

	constexpr size_t size {5000};
	Array<size_t> c {size, 0};
	Array<size_t> d {size, 0};
	assert(c == d);

	c[123] = 456;
	assert(c != d);
}

void testLessThan()
{
	Array<long double> a {30, {1.5, 2.5, 4.5}};
	Array<long double> b {30, {1.5, 2.5, 5.5}};
	assert(a < b);
}

void testGreaterThan()
{
	Array<long double> a {30, {1.5, 2.5, 6.5}};
	Array<long double> b {30, {1.5, 2.5, 5.5}};
	assert(a > b);
}

void testLessThanOrEqual()
{
	Array<long double> a {30, {1.5, 2.5, 5.5}};
	Array<long double> b {30, {1.5, 2.5, 5.5}};
	assert(a <= b);
	Array<long double> c {30, {1.5, -3.5, 5.5}};
	Array<long double> d {30, {1.5, 2.5, 5.5}};
	assert(a <= b);
}

void testGreaterThanOrEqual()
{
	Array<long double> a {30, {1.5, 2.5, 5.5}};
	Array<long double> b {30, {1.5, 2.5, 5.5}};
	assert(a >= b);
	Array<long double> c {30, {1.5, 2.5, 5.5}};
	Array<long double> d {30, {1.5, 2.5, 8.5}};
	assert(a >= b);
}

void testSpaceship()
{
	Array<int> a {10, {1, 2, 2}};
	Array<int> b {10, {1, 2, 3}};
	assert(a <=> b == std::strong_ordering::less);

	Array<int> c {10, {1, 2, 3}};
	Array<int> d {10, {1, 2, 3}};
	assert(c <=> d == std::strong_ordering::equal);

	Array<int> e {10, {1, 2, 4}};
	Array<int> f {10, {1, 2, 3}};
	assert(e <=> f == std::strong_ordering::greater);
}

void testAt()
{
	const Array<int> a {10};
	try
	{
		assert(a.at(10) && false);
	}
	catch(std::out_of_range)
	{}
}

void testFront()
{
	Array<int> a {10};
	a.front() = 256;
	assert(a.front() == 256);
	assert(a[0] == 256);
}

void testBack()
{
	Array<int> a {10};
	a.back() = 256;
	assert(a.back() == 256);
	assert(a[a.size() - 1] == 256);
}

void testEmpty()
{
	Array<int> a;
	assert(a.empty());
}

void testSwap()
{
	std::random_device random;
	Array<unsigned> a {5, {random(), random(), random()}};
	Array<unsigned> b {5, {random(), random(), random()}};
	auto c {a};
	auto d {b};

	a.swap(b);
	assert(a == d);
	assert(b == c);
}

void testFill()
{
	constexpr int fillValue {244};
	Array<int> a {10, fillValue};
	for(int element : a)
		assert(element == fillValue);
}

void testBeginAndEnd()
{
	constexpr size_t size {25};
	Array<short> a {size};

	auto data {a.data()};
	assert(data == a.begin().operator->());
	assert(data + size == a.end().operator->());
}

void testReverseBeginAndEnd()
{
	Array<short> a {3, {111, 222, 333}};
	assert(*a.rbegin() == 333);
	assert(*--a.rend() == 111);
}

void testIteratorContiguousProperty()
{
	static_assert(std::contiguous_iterator<Array<float>::Iterator>);
}

void testIteratorDefaultConstructor()
{
	Array<int> a {100};
	Array<int>::Iterator it;
	assert(it.operator->() == nullptr);
}

void testIteratorDereference()
{
	Array<std::string> a {25, {"AA", "BB", "CC"}};
	auto first {a.begin()};
	assert(*first == "AA");
}

void testIteratorArrow()
{
	Array<std::string> a {25, {"AAA", "BB", "C"}};
	auto first {a.begin()};
	assert(first->length() == 3);
}

void testIteratorEquality()
{
	Array<long> a {30};
	auto begin1 {a.begin()};
	auto begin2 {a.cbegin()};
	auto end1 {a.end()};
	auto end2 {a.cend()};
	assert(begin1 == begin2);
	assert(end1 == end2);
}

void testIteratorInequality()
{
	Array<long> a {30};
	auto getsIncremented {a.cbegin()};
	auto getsDecremented {a.cend()};
	assert(a.begin() != ++getsIncremented);
	assert(a.end() != --getsDecremented);
}

void testIteratorLessThan()
{
	const Array<long> a {30};
	auto getsIncremented {a.begin()};
	assert(a.begin() < ++getsIncremented);
}

void testIteratorGreaterThan()
{
	Array<long> a {30};
	auto getsIncremented {a.begin()};
	assert(++getsIncremented > a.begin());
}

void testIteratorLessThanOrEqual()
{
	Array<long> a {30};
	assert(a.begin() <= a.begin());
	auto getsIncremented {a.begin()};
	assert(a.begin() <= ++getsIncremented);
}

void testIteratorGreaterThanOrEqual()
{
	Array<long> a {30};
	auto getsIncremented {a.begin()};
	assert(a.begin() >= a.begin());
	assert(++getsIncremented >= a.begin());
}

void testIteratorPreIncrement()
{
	Array<long> a {3, {5, 6, 7}};
	auto getsIncremented {a.begin()};
	assert(*++getsIncremented == 6);
}

void testIteratorPostIncrement()
{
	Array<long> a {3, {5, 6, 7}};
	auto incremented {a.begin()};
	auto first {incremented++};
	assert(*first == 5);
	assert(*incremented == 6);
}

void testIteratorPreDecrement()
{
	Array<long> a {3, {5, 6, 7}};
	auto getsDecremented {a.end()};
	assert(*--getsDecremented == 7);
}

void testIteratorPostDecrement()
{
	Array<long> a {3, {5, 6, 7}};
	auto getsDecremented {a.end()};
	auto end {getsDecremented--};
	assert(end != getsDecremented);
}

void testIteratorAdditionAssignment()
{
	Array<int> a {5, {10, 11, 12}};
	auto begin {a.begin()};
	assert(*(begin += 2) == 12);
	assert(*begin == 12);
}

void testIteratorAddition()
{
	Array<int> a {5, {10, 11, 12}};
	auto second {a.begin() + 1};
	auto third {2 + a.begin()};
	assert(*second == 11);
	assert(*third == 12);
}

void testIteratorSubtractionAssignment()
{
	Array<int> a {3, {10, 11, 12}};
	auto end {a.end()};
	assert(*(end -= 2) == 11);
	assert(*end == 11);
}

void testIteratorSubtraction()
{
	Array<int> a {3, {10, 11, 12}};
	auto last {a.end() - 1};
	auto offset {last - a.begin()};
	assert(*last == 12);
	assert(offset == a.size() - 1);
}

void testIteratorSubscript()
{
	Array<int> a {6, {11, 12, 13, 14, 15, 16}};
	auto it {a.begin() + 2};
	assert(it[2] == 15);
}

int main()
{
	testDefaultConstructor();
	testConstructorFromRawArray();
	testConstructorWithInitialValue();
	testConstructorWithInitializerList();
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
