#define ARRAY_NAMESPACE a
#include "Include/Array.hh"

#include <string>

using namespace a;

void testDefaultConstructor()
{
	Array<int> a;
	assert(a.size() == 0);
	assert(a.data() == nullptr);
}

void testConstructorFromRawArray()
{
	constexpr size_t testSize {30};
	auto raw {new int[testSize]};
	Array<int> a(testSize, raw);
	assert(a.size() == testSize);
	assert(a.data() != nullptr);
	// raw array gets deleted here automatically
}

void testConstructorWithInitialValue()
{
	Array<std::string> a(30, "STRING");
	for(auto& str : a)
		assert(str == "STRING");
}

void testCopyConstructor()
{
	Array<float> a(20, 6.66f);
	auto b {a};
	assert(a.size() == b.size());

	auto elementA {a.begin()};
	for(float elementB : b)
		assert(elementB == *elementA++);
}

int main()
{
	testDefaultConstructor();
	testConstructorFromRawArray();
	testConstructorWithInitialValue();
	testCopyConstructor();
}
