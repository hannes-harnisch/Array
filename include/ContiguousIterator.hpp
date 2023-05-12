#pragma once

#include <iterator>
#include <memory>

#ifdef __cpp_lib_three_way_comparison
	#include <compare>
#endif

#ifndef HH_ASSERT
	#include <cassert>
	#define HH_ASSERT(condition, message) assert((condition) && (message))
#endif

#ifndef HH_DEBUG
	#undef HH_ASSERT
	#define HH_ASSERT(condition, message)
#endif

namespace hh {

#ifdef HH_DEBUG
struct IteratorDebugBase {};
#endif

template<typename T, typename Diff, typename Ptr, typename ConstPtr>
class ContiguousConstIterator
#ifdef HH_DEBUG
	: private IteratorDebugBase
#endif
{

public:
#ifdef __cpp_lib_concepts
	using iterator_concept = std::contiguous_iterator_tag;
#endif
	using iterator_category = std::random_access_iterator_tag;
	using value_type		= T;
	using difference_type	= Diff;
	using pointer			= ConstPtr;
	using reference			= const T&;

	constexpr ContiguousConstIterator() noexcept :
		ptr() {
	}

	constexpr reference operator*() const noexcept {
#ifdef HH_DEBUG
		assert_deref();
#endif
		return *ptr;
	}

	constexpr pointer operator->() const noexcept {
#ifdef HH_DEBUG
		assert_deref();
#endif
		return ptr;
	}

	constexpr ContiguousConstIterator& operator++() noexcept {
#ifdef HH_DEBUG
		assert_increment();
#endif
		++ptr;
		return *this;
	}

	constexpr ContiguousConstIterator operator++(int) noexcept {
		ContiguousConstIterator old = *this;
#ifdef HH_DEBUG
		assert_increment();
#endif
		++ptr;
		return old;
	}

	constexpr ContiguousConstIterator& operator--() noexcept {
#ifdef HH_DEBUG
		assert_decrement();
#endif
		--ptr;
		return *this;
	}

	constexpr ContiguousConstIterator operator--(int) noexcept {
		ContiguousConstIterator old = *this;
#ifdef HH_DEBUG
		assert_decrement();
#endif
		--ptr;
		return old;
	}

	constexpr ContiguousConstIterator& operator+=(difference_type offset) noexcept {
#ifdef HH_DEBUG
		assert_offset(offset);
#endif
		ptr += offset;
		return *this;
	}

	constexpr ContiguousConstIterator operator+(difference_type offset) const noexcept {
#ifdef HH_DEBUG
		assert_offset(offset);
#endif
		ContiguousConstIterator copy = *this;
		copy.ptr += offset;
		return copy;
	}

	friend constexpr ContiguousConstIterator operator+(difference_type offset, ContiguousConstIterator it) noexcept {
#ifdef HH_DEBUG
		it.assert_offset(offset);
#endif
		it.ptr += offset;
		return it;
	}

	constexpr ContiguousConstIterator& operator-=(difference_type offset) noexcept {
#ifdef HH_DEBUG
		assert_offset(offset);
#endif
		ptr -= offset;
		return *this;
	}

	constexpr ContiguousConstIterator operator-(difference_type offset) const noexcept {
#ifdef HH_DEBUG
		assert_offset(offset);
#endif
		ptr += offset;
		return *this;
	}

	constexpr difference_type operator-(ContiguousConstIterator other) const noexcept {
#ifdef HH_DEBUG
		assert_compatible(other);
#endif
		return ptr - other.ptr;
	}

	constexpr reference operator[](difference_type offset) const noexcept {
#ifdef HH_DEBUG
		assert_offset(offset);
#endif
		return ptr[offset];
	}

	constexpr bool operator==(ContiguousConstIterator other) const noexcept {
#ifdef HH_DEBUG
		assert_compatible(other);
#endif
		return ptr == other.ptr;
	}

	constexpr bool operator!=(ContiguousConstIterator other) const noexcept {
#ifdef HH_DEBUG
		assert_compatible(other);
#endif
		return ptr != other.ptr;
	}

	constexpr bool operator<(ContiguousConstIterator other) const noexcept {
#ifdef HH_DEBUG
		assert_compatible(other);
#endif
		return ptr < other.ptr;
	}

	constexpr bool operator>(ContiguousConstIterator other) const noexcept {
#ifdef HH_DEBUG
		assert_compatible(other);
#endif
		return ptr > other.ptr;
	}

	constexpr bool operator<=(ContiguousConstIterator other) const noexcept {
#ifdef HH_DEBUG
		assert_compatible(other);
#endif
		return ptr <= other.ptr;
	}

	constexpr bool operator>=(ContiguousConstIterator other) const noexcept {
#ifdef HH_DEBUG
		assert_compatible(other);
#endif
		return ptr >= other.ptr;
	}

#ifdef __cpp_lib_three_way_comparison
	constexpr std::strong_ordering operator<=>(ContiguousConstIterator other) const noexcept {
	#ifdef HH_DEBUG
		assert_compatible(other);
	#endif
		return ptr <=> other.ptr;
	}
#endif

private:
	Ptr ptr;

	constexpr ContiguousConstIterator(Ptr ptr) noexcept :
		ptr(ptr) {
	}

#ifdef HH_DEBUG
	constexpr ContiguousConstIterator(Ptr ptr, const ContainerDebugBase* base) noexcept :
		ptr(ptr) {
	}

	void assert_deref() const noexcept {
		HH_ASSERT(ptr, "cannot dereference value-initialized iterator");
	}

	void assert_increment() const noexcept {
	}

	void assert_decrement() const noexcept {
	}

	void assert_offset(difference_type offset) const noexcept {
	}

	void assert_compatible(const ContiguousConstIterator& other) const noexcept {
	}
#endif

	template<typename, typename, typename, typename>
	friend class ContiguousIterator;

	template<typename>
	friend struct std::pointer_traits;

	template<typename, typename>
	friend class VarArray;

	template<typename, size_t>
	friend class FixedList;
};

template<typename T, typename Diff, typename Ptr, typename ConstPtr>
class ContiguousIterator : public ContiguousConstIterator<T, Diff, Ptr, ConstPtr> {
	using Base = ContiguousConstIterator<T, Diff, Ptr, ConstPtr>;

public:
#ifdef __cpp_lib_concepts
	using iterator_concept = std::contiguous_iterator_tag;
#endif
	using iterator_category = std::random_access_iterator_tag;
	using value_type		= T;
	using difference_type	= Diff;
	using pointer			= Ptr;
	using reference			= T&;

	using Base::Base;

	constexpr reference operator*() const noexcept {
		return const_cast<reference>(Base::operator*());
	}

	constexpr pointer operator->() const noexcept {
#ifdef HH_DEBUG
		this->assert_deref();
#endif
		return this->ptr;
	}

	constexpr ContiguousIterator& operator++() noexcept {
		Base::operator++();
		return *this;
	}

	constexpr ContiguousIterator operator++(int) noexcept {
		auto  old = *this;
		Base::operator++();
		return old;
	}

	constexpr ContiguousIterator& operator--() noexcept {
		Base::operator--();
		return *this;
	}

	constexpr ContiguousIterator operator--(int) noexcept {
		auto  old = *this;
		Base::operator--();
		return old;
	}

	constexpr ContiguousIterator& operator+=(difference_type offset) noexcept {
		Base::operator+=(offset);
		return *this;
	}

	constexpr ContiguousIterator operator+(difference_type offset) const noexcept {
		auto copy = *this;
		copy += offset;
		return copy;
	}

	friend constexpr ContiguousIterator operator+(difference_type offset, ContiguousIterator it) noexcept {
		it += offset;
		return it;
	}

	constexpr ContiguousIterator& operator-=(difference_type offset) noexcept {
		Base::operator-=(offset);
		return *this;
	}

	constexpr difference_type operator-(ContiguousIterator other) const noexcept {
		return Base::operator-(other);
	}

	constexpr ContiguousIterator operator-(difference_type offset) const noexcept {
		auto copy = *this;
		copy -= offset;
		return copy;
	}

	constexpr reference operator[](difference_type offset) const noexcept {
		return const_cast<reference>(Base::operator[](offset));
	}
};

} // namespace hh

template<typename T, typename Diff, typename Ptr, typename ConstPtr>
struct std::pointer_traits<hh::ContiguousConstIterator<T, Diff, Ptr, ConstPtr>> {
	using pointer		  = hh::ContiguousConstIterator<T, Diff, Ptr, ConstPtr>;
	using element_type	  = const typename pointer::value_type;
	using difference_type = typename pointer::difference_type;

	[[nodiscard]] static constexpr element_type* to_address(pointer it) noexcept {
		HH_ASSERT(it.array->data() <= it.ptr && it.ptr <= it.array->data() + it.array->size(), "Iterator is not within a "
																							   "validly addressable range.");
		return std::to_address(it.ptr);
	}
};

template<typename T, typename Diff, typename Ptr, typename ConstPtr>
struct std::pointer_traits<hh::ContiguousIterator<T, Diff, Ptr, ConstPtr>> {
	using pointer		  = hh::ContiguousIterator<T, Diff, Ptr, ConstPtr>;
	using element_type	  = typename pointer::value_type;
	using difference_type = typename pointer::difference_type;

	[[nodiscard]] static constexpr element_type* to_address(pointer it) noexcept {
		HH_ASSERT(it.array->data() <= it.ptr && it.ptr <= it.array->data() + it.array->size(), "Iterator is not within a "
																							   "validly addressable range.");
		return std::to_address(it.ptr);
	}
};
