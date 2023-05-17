#pragma once

#include "ContiguousIterator.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hh {

template<typename T, typename Allocator = std::allocator<T>>
class VarArray
#ifdef HH_DEBUG
	: private ContainerDebugBase
#endif
{
	friend IteratorDebugBase;

	using AllocTraits = std::allocator_traits<Allocator>;

public:
	using value_type			 = T;
	using allocator_type		 = Allocator;
	using pointer				 = typename AllocTraits::pointer;
	using const_pointer			 = typename AllocTraits::const_pointer;
	using reference				 = T&;
	using const_reference		 = const T&;
	using size_type				 = typename AllocTraits::size_type;
	using difference_type		 = typename AllocTraits::difference_type;
	using iterator				 = ContiguousIterator<T, difference_type, pointer, const_pointer>;
	using const_iterator		 = ContiguousConstIterator<T, difference_type, pointer, const_pointer>;
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
#ifdef HH_DEBUG
		return {alloc_pair.array, *this};
#else
		return {alloc_pair.array};
#endif
	}

	constexpr const_iterator begin() const noexcept {
#ifdef HH_DEBUG
		return {alloc_pair.array, *this};
#else
		return {alloc_pair.array};
#endif
	}

	constexpr iterator end() noexcept {
#ifdef HH_DEBUG
		return {alloc_pair.array + count, *this};
#else
		return {alloc_pair.array + count};
#endif
	}

	constexpr const_iterator end() const noexcept {
#ifdef HH_DEBUG
		return {alloc_pair.array + count, *this};
#else
		return {alloc_pair.array + count};
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
