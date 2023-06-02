#pragma once

#include <iterator>
#include <memory>
#include <type_traits>

#ifdef __cpp_lib_three_way_comparison
	#include <compare>
#endif

#ifndef HH_ASSERT
	#include <cassert>
	#define HH_ASSERT(condition, message) assert((condition) && (message))
#endif

#ifndef NDEBUG
	#include <shared_mutex>
#else
	#undef HH_ASSERT
	#define HH_ASSERT(condition, message)
#endif

namespace hh {

#ifndef NDEBUG
class ContainerDebugBase {
	friend class IteratorDebugBase;

	static inline std::shared_mutex debug_iterator_lock;

	mutable IteratorDebugBase* head = nullptr;

protected:
	constexpr ContainerDebugBase() noexcept = default;

	constexpr ContainerDebugBase(const ContainerDebugBase&) noexcept :
		head(nullptr) {
	}

	constexpr ContainerDebugBase(ContainerDebugBase&& other) noexcept :
		head(other.head) {
		other.head = nullptr;
		update_iterators();
	}

	constexpr ~ContainerDebugBase() {
		invalidate_iterators();
	}

	constexpr ContainerDebugBase& operator=(const ContainerDebugBase&) noexcept {
		invalidate_iterators();
		head = nullptr;
		return *this;
	}

	constexpr ContainerDebugBase& operator=(ContainerDebugBase&& other) noexcept {
		invalidate_iterators();
		head	   = other.head;
		other.head = nullptr;
		update_iterators();
		return *this;
	}

	constexpr void invalidate_iterators() const noexcept {
	#ifdef __cpp_lib_is_constant_evaluated
		if (std::is_constant_evaluated())
			invalidate_iterators_no_lock();
		else
	#endif
		{
			debug_iterator_lock.lock();
			invalidate_iterators_no_lock();
			debug_iterator_lock.unlock();
		}
	}

	constexpr void update_iterators() const noexcept {
	#ifdef __cpp_lib_is_constant_evaluated
		if (std::is_constant_evaluated())
			update_iterators_no_lock();
		else
	#endif
		{
			debug_iterator_lock.lock();
			update_iterators_no_lock();
			debug_iterator_lock.unlock();
		}
	}

	constexpr void invalidate_iterators_no_lock() const noexcept;
	constexpr void update_iterators_no_lock() const noexcept;
};

class IteratorDebugBase {
	friend ContainerDebugBase;

protected:
	struct Range {
		const void* begin;
		const void* end;
	};

	using RangeGetter = Range (*)(const ContainerDebugBase&) noexcept;

	const ContainerDebugBase* parent	   = nullptr;
	RangeGetter				  range_getter = nullptr;
	IteratorDebugBase*		  next		   = nullptr;

	constexpr IteratorDebugBase() noexcept = default;

	template<typename C>
	constexpr IteratorDebugBase(const C* container) noexcept :
		parent(container),
		range_getter(get_range_from<C>) {
		adopt_current_parent();
	}

	constexpr IteratorDebugBase(const IteratorDebugBase& other) noexcept :
		parent(other.parent),
		range_getter(other.range_getter) {
		adopt_current_parent();
	}

	constexpr ~IteratorDebugBase() {
		orphan_this();
	}

	constexpr IteratorDebugBase& operator=(const IteratorDebugBase& other) noexcept {
		if (parent != other.parent) {
			orphan_this();
			if (other.parent == nullptr) {
				parent		 = nullptr;
				range_getter = nullptr;
				next		 = nullptr;
			} else {
				parent		 = other.parent;
				range_getter = other.range_getter;
				adopt_current_parent();
			}
		}
		return *this;
	}

	template<typename T>
	constexpr const T* get_begin() const noexcept {
		HH_ASSERT(parent, "iterator has been invalidated");
		const void* begin = range_getter(*parent).begin;
		return static_cast<const T*>(begin);
	}

	template<typename T>
	constexpr const T* get_end() const noexcept {
		HH_ASSERT(parent, "iterator has been invalidated");
		const void* end = range_getter(*parent).end;
		return static_cast<const T*>(end);
	}

private:
	template<typename C>
	static constexpr Range get_range_from(const ContainerDebugBase& base) noexcept {
		const C& cont = static_cast<const C&>(base);
		return {
			cont.data(),
			cont.data() + cont.size(),
		};
	}

	constexpr void adopt_current_parent() noexcept {
	#ifdef __cpp_lib_is_constant_evaluated
		if (std::is_constant_evaluated())
			adopt_current_parent_no_lock();
		else
	#endif
		{
			ContainerDebugBase::debug_iterator_lock.lock();
			adopt_current_parent_no_lock();
			ContainerDebugBase::debug_iterator_lock.unlock();
		}
	}

	constexpr void adopt_current_parent_no_lock() noexcept {
		next		 = parent->head;
		parent->head = this;
	}

	constexpr void orphan_this() noexcept {
	#ifdef __cpp_lib_is_constant_evaluated
		if (std::is_constant_evaluated())
			orphan_this_no_lock();
		else
	#endif
		{
			ContainerDebugBase::debug_iterator_lock.lock();
			orphan_this_no_lock();
			ContainerDebugBase::debug_iterator_lock.unlock();
		}
	}

	constexpr void orphan_this_no_lock() noexcept {
		if (parent == nullptr)
			return;

		IteratorDebugBase** it = &parent->head;
		while (*it != this)
			it = &(*it)->next;

		*it = next;
	}
};

constexpr void ContainerDebugBase::invalidate_iterators_no_lock() const noexcept {
	for (auto it = head; it != nullptr; it = it->next)
		it->parent = nullptr;
}

constexpr void ContainerDebugBase::update_iterators_no_lock() const noexcept {
	for (auto it = head; it != nullptr; it = it->next)
		it->parent = this;
}
#endif

template<typename T, typename Diff, typename Ptr, typename ConstPtr>
class ContiguousConstIterator
#ifndef NDEBUG
	: private IteratorDebugBase
#endif
{
	Ptr ptr;

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

	constexpr const T& operator*() const noexcept {
#ifndef NDEBUG
		assert_deref();
#endif
		return *ptr;
	}

	constexpr pointer operator->() const noexcept {
#ifndef NDEBUG
		assert_deref();
#endif
		return ptr;
	}

	constexpr ContiguousConstIterator& operator++() noexcept {
#ifndef NDEBUG
		assert_increment();
#endif
		++ptr;
		return *this;
	}

	constexpr ContiguousConstIterator operator++(int) noexcept {
		ContiguousConstIterator old = *this;
#ifndef NDEBUG
		assert_increment();
#endif
		++ptr;
		return old;
	}

	constexpr ContiguousConstIterator& operator--() noexcept {
#ifndef NDEBUG
		assert_decrement();
#endif
		--ptr;
		return *this;
	}

	constexpr ContiguousConstIterator operator--(int) noexcept {
		ContiguousConstIterator old = *this;
#ifndef NDEBUG
		assert_decrement();
#endif
		--ptr;
		return old;
	}

	constexpr ContiguousConstIterator& operator+=(difference_type offset) noexcept {
#ifndef NDEBUG
		assert_offset(offset);
#endif
		ptr += offset;
		return *this;
	}

	constexpr ContiguousConstIterator operator+(difference_type offset) const noexcept {
#ifndef NDEBUG
		assert_offset(offset);
#endif
		ContiguousConstIterator copy = *this;
		copy.ptr += offset;
		return copy;
	}

	friend constexpr ContiguousConstIterator operator+(difference_type offset, ContiguousConstIterator it) noexcept {
#ifndef NDEBUG
		it.assert_offset(offset);
#endif
		it.ptr += offset;
		return it;
	}

	constexpr ContiguousConstIterator& operator-=(difference_type offset) noexcept {
#ifndef NDEBUG
		assert_offset(-offset);
#endif
		ptr -= offset;
		return *this;
	}

	constexpr ContiguousConstIterator operator-(difference_type offset) const noexcept {
#ifndef NDEBUG
		assert_offset(-offset);
#endif
		ContiguousConstIterator copy = *this;
		copy.ptr -= offset;
		return copy;
	}

	constexpr difference_type operator-(ContiguousConstIterator other) const noexcept {
#ifndef NDEBUG
		assert_compatible(other);
#endif
		return ptr - other.ptr;
	}

	constexpr const T& operator[](difference_type offset) const noexcept {
#ifndef NDEBUG
		assert_offset(offset);
#endif
		return ptr[offset];
	}

	constexpr bool operator==(ContiguousConstIterator other) const noexcept {
#ifndef NDEBUG
		assert_compatible(other);
#endif
		return ptr == other.ptr;
	}

	constexpr bool operator!=(ContiguousConstIterator other) const noexcept {
#ifndef NDEBUG
		assert_compatible(other);
#endif
		return ptr != other.ptr;
	}

	constexpr bool operator<(ContiguousConstIterator other) const noexcept {
#ifndef NDEBUG
		assert_compatible(other);
#endif
		return ptr < other.ptr;
	}

	constexpr bool operator>(ContiguousConstIterator other) const noexcept {
#ifndef NDEBUG
		assert_compatible(other);
#endif
		return ptr > other.ptr;
	}

	constexpr bool operator<=(ContiguousConstIterator other) const noexcept {
#ifndef NDEBUG
		assert_compatible(other);
#endif
		return ptr <= other.ptr;
	}

	constexpr bool operator>=(ContiguousConstIterator other) const noexcept {
#ifndef NDEBUG
		assert_compatible(other);
#endif
		return ptr >= other.ptr;
	}

#ifdef __cpp_lib_three_way_comparison
	constexpr std::strong_ordering operator<=>(ContiguousConstIterator other) const noexcept {
	#ifndef NDEBUG
		assert_compatible(other);
	#endif
		return ptr <=> other.ptr;
	}
#endif

private:
	constexpr ContiguousConstIterator(Ptr ptr) noexcept :
		ptr(ptr) {
	}

#ifndef NDEBUG
	template<typename T>
	constexpr ContiguousConstIterator(Ptr ptr, const T* container) noexcept :
		IteratorDebugBase(container),
		ptr(ptr) {
	}

	constexpr void assert_deref() const noexcept {
		HH_ASSERT(ptr, "can't dereference value-initialized iterator");
		HH_ASSERT(parent, "can't dereference invalidated iterator");
		HH_ASSERT(get_begin<T>() <= ptr && ptr < get_end<T>(), "can't dereference out-of-range iterator");
	}

	constexpr void assert_increment() const noexcept {
		HH_ASSERT(ptr, "can't increment value-initialized iterator");
		HH_ASSERT(ptr < get_end<T>(), "can't increment iterator past end");
	}

	constexpr void assert_decrement() const noexcept {
		HH_ASSERT(ptr, "can't decrement value-initialized iterator");
		HH_ASSERT(get_begin<T>() < ptr, "can't decrement iterator before begin");
	}

	constexpr void assert_offset(difference_type offset) const noexcept {
		HH_ASSERT(offset == 0 || ptr, "can't offset value-initialized iterator");

		if (offset < 0)
			HH_ASSERT(offset >= get_begin<T>() - ptr, "can't offset iterator before begin");

		if (offset > 0)
			HH_ASSERT(offset <= get_end<T>() - ptr, "can't offset iterator after end");
	}

	constexpr void assert_compatible(const ContiguousConstIterator& other) const noexcept {
		HH_ASSERT(parent == other.parent, "iterators do not belong to the same container instance");
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

	constexpr T& operator*() const noexcept {
		return const_cast<T&>(Base::operator*());
	}

	constexpr pointer operator->() const noexcept {
#ifndef NDEBUG
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

	constexpr T& operator[](difference_type offset) const noexcept {
		return const_cast<T&>(Base::operator[](offset));
	}
};

} // namespace hh

template<typename T, typename Diff, typename Ptr, typename ConstPtr>
struct std::pointer_traits<hh::ContiguousConstIterator<T, Diff, Ptr, ConstPtr>> {
	using pointer		  = hh::ContiguousConstIterator<T, Diff, Ptr, ConstPtr>;
	using element_type	  = const T;
	using difference_type = Diff;

	[[nodiscard]] static constexpr const T* to_address(pointer it) noexcept {
		HH_ASSERT(it.get_begin<T>() <= it.ptr && it.ptr <= it.get_end<T>(), "iterator is not within a validly addressable "
																			"range");
		return std::to_address(it.ptr);
	}
};

template<typename T, typename Diff, typename Ptr, typename ConstPtr>
struct std::pointer_traits<hh::ContiguousIterator<T, Diff, Ptr, ConstPtr>> {
	using pointer		  = hh::ContiguousIterator<T, Diff, Ptr, ConstPtr>;
	using element_type	  = T;
	using difference_type = Diff;

	[[nodiscard]] static constexpr T* to_address(pointer it) noexcept {
		HH_ASSERT(it.get_begin<T>() <= it.ptr && it.ptr <= it.get_end<T>(), "iterator is not within a validly addressable "
																			"range");
		return std::to_address(it.ptr);
	}
};
