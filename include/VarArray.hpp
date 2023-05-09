#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifndef HH_ASSERT
	#include <cassert>
	#define HH_ASSERT(condition, message) assert((condition) && (message))
#endif

#if !DEBUG
	#undef HH_ASSERT
	#define HH_ASSERT(condition, message)
#endif

namespace hh {

template<typename Array, bool CONST>
class ArrayIterator {
	friend Array;

	template<typename, bool>
	friend class ArrayIterator;

	template<typename>
	friend struct std::pointer_traits;

public:
#ifdef __cpp_lib_concepts
	using iterator_concept = std::contiguous_iterator_tag;
#endif
	using iterator_category = std::random_access_iterator_tag;
	using pointer			= std::conditional_t<CONST, typename Array::const_pointer, typename Array::pointer>;
	using reference			= std::conditional_t<CONST, typename Array::const_reference, typename Array::reference>;
	using value_type		= typename Array::value_type;
	using difference_type	= typename Array::difference_type;

	constexpr ArrayIterator() = default;

	constexpr operator ArrayIterator<Array, true>() const noexcept {
#if DEBUG
		return {ptr, array};
#else
		return {ptr};
#endif
	}

	constexpr reference operator*() const noexcept {
		return *operator->();
	}

	constexpr pointer operator->() const noexcept {
		HH_ASSERT(array->data() <= ptr && ptr < array->data() + array->size(), "Tried to dereference value-initialized or end "
																			   "iterator.");
		return ptr;
	}

	template<bool CONST2>
	constexpr bool operator==(ArrayIterator<Array, CONST2> that) const noexcept {
		return ptr == that.ptr;
	}

	template<bool CONST2>
	constexpr bool operator!=(ArrayIterator<Array, CONST2> that) const noexcept {
		return ptr != that.ptr;
	}

	template<bool CONST2>
	constexpr bool operator<(ArrayIterator<Array, CONST2> that) const noexcept {
		return ptr < that.ptr;
	}

	template<bool CONST2>
	constexpr bool operator<=(ArrayIterator<Array, CONST2> that) const noexcept {
		return ptr <= that.ptr;
	}

	template<bool CONST2>
	constexpr bool operator>(ArrayIterator<Array, CONST2> that) const noexcept {
		return ptr > that.ptr;
	}

	template<bool CONST2>
	constexpr bool operator>=(ArrayIterator<Array, CONST2> that) const noexcept {
		return ptr >= that.ptr;
	}

#ifdef __cpp_lib_three_way_comparison
	template<bool CONST2>
	constexpr std::strong_ordering operator<=>(ArrayIterator<Array, CONST2> that) const noexcept {
		return ptr <=> that.ptr;
	}
#endif

	constexpr ArrayIterator& operator++() noexcept {
		return *this += 1;
	}

	constexpr ArrayIterator operator++(int) noexcept {
		auto old = *this;
		*this += 1;
		return old;
	}

	constexpr ArrayIterator& operator--() noexcept {
		return *this -= 1;
	}

	constexpr ArrayIterator operator--(int) noexcept {
		auto old = *this;
		*this -= 1;
		return old;
	}

	constexpr ArrayIterator& operator+=(difference_type offset) noexcept {
		HH_ASSERT(offset == 0 || ptr, "Cannot offset value-initialized iterator.");
		HH_ASSERT(offset >= 0 || offset >= array->data() - ptr, "Cannot offset list iterator before begin.");
		HH_ASSERT(offset <= 0 || offset <= array->data() + array->size() - ptr, "Cannot offset list iterator past end.");

		ptr += offset;
		return *this;
	}

	constexpr ArrayIterator operator+(difference_type offset) const noexcept {
		auto old = *this;
		return old += offset;
	}

	friend constexpr ArrayIterator operator+(difference_type offset, ArrayIterator iterator) noexcept {
		return iterator + offset;
	}

	constexpr ArrayIterator& operator-=(difference_type offset) noexcept {
		return *this += -offset;
	}

	constexpr ArrayIterator operator-(difference_type offset) const noexcept {
		auto old = *this;
		return old -= offset;
	}

	template<bool CONST2>
	constexpr difference_type operator-(ArrayIterator<Array, CONST2> that) const noexcept {
		return ptr - that.ptr;
	}

	constexpr reference operator[](difference_type offset) const noexcept {
		return *(*this + offset);
	}

private:
	pointer ptr = nullptr;
#if DEBUG
	const Array* array = nullptr;

	constexpr ArrayIterator(pointer ptr, const Array* arr) noexcept :
		ptr(ptr),
		array(arr) {
	}
#else
	constexpr ArrayIterator(pointer ptr) noexcept :
		ptr(ptr) {
	}
#endif
};

template<typename T, typename Allocator = std::allocator<T>>
class VarArray {
	using AllocTraits = std::allocator_traits<Allocator>;

public:
	using value_type			 = typename AllocTraits::value_type;
	using reference				 = value_type&;
	using const_reference		 = const value_type&;
	using pointer				 = typename AllocTraits::pointer;
	using const_pointer			 = typename AllocTraits::const_pointer;
	using size_type				 = typename AllocTraits::size_type;
	using difference_type		 = typename AllocTraits::difference_type;
	using allocator_type		 = Allocator;
	using iterator				 = ArrayIterator<VarArray, false>;
	using const_iterator		 = ArrayIterator<VarArray, true>;
	using reverse_iterator		 = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static constexpr size_type max_size() noexcept {
		return std::numeric_limits<size_type>::max();
	}

	constexpr VarArray() = default;

	constexpr explicit VarArray(const Allocator& alloc) :
		alloc_pair {alloc} {
	}

	constexpr VarArray(size_type count, const Allocator& alloc = Allocator()) :
		alloc_pair {alloc, allocate(count)},
		count(count) {
		if constexpr (!std::is_trivially_default_constructible_v<T>)
			for (auto& element : *this)
				AllocTraits::construct(alloc_pair, &element);
	}

	template<typename U>
	constexpr VarArray(size_type count, const U& value, const Allocator& alloc = Allocator()) :
		alloc_pair {alloc, allocate(count)},
		count(count) {
		for (auto& element : *this)
			AllocTraits::construct(alloc_pair, &element, value);
	}

	template<typename U>
	constexpr VarArray(size_type count, std::initializer_list<U> initializers, const Allocator& alloc = Allocator()) :
		alloc_pair {alloc, allocate(count)},
		count(count) {
		HH_ASSERT(count >= initializers.size(), "Size of initializer list exceeds array size.");

		auto element = begin();
		for (auto init : initializers)
			AllocTraits::construct(alloc_pair, &*element++, std::move(init));

		auto offset = static_cast<typename iterator::difference_type>(initializers.size());
		for (auto rest = begin() + offset; rest != end(); ++rest)
			AllocTraits::construct(alloc_pair, &*rest);
	}

	template<typename U, typename V>
	constexpr VarArray(size_type				count,
					   std::initializer_list<U> initializers,
					   const V&					fallback,
					   const Allocator&			alloc = Allocator()) :
		VarArray(count, initializers, alloc) {
		auto offset = static_cast<typename iterator::difference_type>(initializers.size());
		for (auto element = begin() + offset; element != end(); ++element)
			AllocTraits::construct(alloc_pair, &*element, fallback);
	}

	constexpr VarArray(const VarArray& that) :
		alloc_pair {AllocTraits::select_on_container_copy_construction(that.alloc_pair.first()), allocate(that.count)},
		count(that.count) {
		auto other = that.begin();
		for (auto& element : *this)
			AllocTraits::construct(alloc_pair, &element, *other++);
	}

	constexpr VarArray(VarArray&& that) noexcept :
		alloc_pair {std::move(that.alloc_pair.first()), std::exchange(that.alloc_pair.array, {})},
		count(that.count) {
	}

#ifdef __cpp_constexpr_dynamic_alloc
	constexpr
#endif
		~VarArray() {
		destruct();
	}

	constexpr VarArray& operator=(VarArray that) noexcept {
		swap(that);
		return *this;
	}

	template<typename C>
	constexpr bool operator==(const C& that) const noexcept {
		return size() == std::size(that) && std::equal(begin(), end(), std::begin(that));
	}

	template<typename C>
	constexpr bool operator!=(const C& that) const noexcept {
		return !operator==(that);
	}

	template<typename C>
	constexpr bool operator<(const C& that) const noexcept {
		return std::lexicographical_compare(begin(), end(), std::begin(that), std::end(that));
	}

	template<typename C>
	constexpr bool operator>(const C& that) const noexcept {
		return that < *this;
	}

	template<typename C>
	constexpr bool operator<=(const C& that) const noexcept {
		return !operator>(that);
	}

	template<typename C>
	constexpr bool operator>=(const C& that) const noexcept {
		return !operator<(that);
	}

#ifdef __cpp_lib_three_way_comparison
	constexpr auto operator<=>(const auto& that) const noexcept {
		return std::lexicographical_compare_three_way(begin(), end(), std::begin(that), std::end(that));
	}
#endif

	constexpr reference operator[](size_type index) noexcept {
		return common_subscript(*this, index);
	}

	constexpr const_reference operator[](size_type index) const noexcept {
		return common_subscript(*this, index);
	}

	constexpr reference at(size_type index) {
		return common_at(*this, index);
	}

	constexpr const_reference at(size_type index) const {
		return common_at(*this, index);
	}

	constexpr pointer get(size_type index) noexcept {
		return common_get(*this, index);
	}

	constexpr const_pointer get(size_type index) const noexcept {
		return common_get(*this, index);
	}

	constexpr reference front() noexcept {
		return (*this)[0];
	}

	constexpr const_reference front() const noexcept {
		return (*this)[0];
	}

	constexpr reference back() noexcept {
		return (*this)[count - 1];
	}

	constexpr const_reference back() const noexcept {
		return (*this)[count - 1];
	}

	[[nodiscard]] constexpr bool empty() const noexcept {
		return !count;
	}

	constexpr size_type size() const noexcept {
		return count;
	}

	constexpr pointer data() noexcept {
		return alloc_pair.array;
	}

	constexpr const_pointer data() const noexcept {
		return alloc_pair.array;
	}

	template<typename U>
	constexpr void fill(const U& value) noexcept {
		std::fill(begin(), end(), value);
	}

	constexpr void reset() noexcept {
		destruct();
		alloc_pair.array = {};
		count			 = 0;
	}

	constexpr void swap(VarArray& that) noexcept {
		std::swap(alloc_pair, that.alloc_pair);
		std::swap(count, that.count);
	}

	friend constexpr void swap(VarArray& left, VarArray& right) noexcept {
		left.swap(right);
	}

	constexpr iterator begin() noexcept {
#if DEBUG
		return {data(), this};
#else
		return {data()};
#endif
	}

	constexpr const_iterator begin() const noexcept {
#if DEBUG
		return {data(), this};
#else
		return {data()};
#endif
	}

	constexpr iterator end() noexcept {
#if DEBUG
		return {data() + count, this};
#else
		return {data() + count};
#endif
	}

	constexpr const_iterator end() const noexcept {
#if DEBUG
		return {data() + count, this};
#else
		return {data() + count};
#endif
	}

	constexpr reverse_iterator rbegin() noexcept {
		return reverse_iterator(end());
	}

	constexpr const_reverse_iterator rbegin() const noexcept {
		return const_reverse_iterator(end());
	}

	constexpr reverse_iterator rend() noexcept {
		return reverse_iterator(begin());
	}

	constexpr const_reverse_iterator rend() const noexcept {
		return const_reverse_iterator(begin());
	}

	constexpr const_iterator cbegin() const noexcept {
		return begin();
	}

	constexpr const_iterator cend() const noexcept {
		return end();
	}

	constexpr const_reverse_iterator crbegin() const noexcept {
		return rbegin();
	}

	constexpr const_reverse_iterator crend() const noexcept {
		return rend();
	}

private:
	struct : Allocator {
		VarArray::pointer array = {};

		Allocator& first() noexcept {
			return *this;
		}

		const Allocator& first() const noexcept {
			return *this;
		}

	} alloc_pair;

	size_type count = {};

	constexpr pointer allocate(size_type units) {
		return AllocTraits::allocate(alloc_pair, units);
	}

	template<typename U>
	static constexpr auto& common_subscript(U& self, size_type index) noexcept {
		HH_ASSERT(index < self.count, "Tried to access list out of range.");
		return self.data()[index];
	}

	template<typename U>
	static constexpr auto& common_at(U& self, size_type index) {
		if (index < self.count)
			return self.data()[index];

		throw std::out_of_range("Index into list was out of range.");
	}

	template<typename U>
	static constexpr auto common_get(U& self, size_type index) noexcept {
		if (index < self.count)
			return self.data() + index;

		return static_cast<decltype(self.data())>(nullptr);
	}

	constexpr void destruct() noexcept {
		if constexpr (!std::is_trivially_destructible_v<value_type>)
			for (auto& element : *this)
				AllocTraits::destroy(alloc_pair, &element);

		if (data())
			AllocTraits::deallocate(alloc_pair, data(), count);
	}
};

} // namespace hh

template<typename Array>
struct std::pointer_traits<hh::ArrayIterator<Array, true>> {
	using pointer		  = hh::ArrayIterator<Array, true>;
	using element_type	  = typename pointer::value_type const;
	using difference_type = typename pointer::difference_type;

	[[nodiscard]] static constexpr element_type* to_address(pointer it) noexcept {
		HH_ASSERT(it.array->data() <= it.ptr && it.ptr <= it.array->data() + it.array->size(), "Iterator is not within a "
																							   "validly addressable range.");
		return it.ptr;
	}
};

template<typename Array>
struct std::pointer_traits<hh::ArrayIterator<Array, false>> {
	using pointer		  = hh::ArrayIterator<Array, false>;
	using element_type	  = typename pointer::value_type;
	using difference_type = typename pointer::difference_type;

	[[nodiscard]] static constexpr element_type* to_address(pointer it) noexcept {
		HH_ASSERT(it.array->data() <= it.ptr && it.ptr <= it.array->data() + it.array->size(), "Iterator is not within a "
																							   "validly addressable range.");
		return it.ptr;
	}
};
