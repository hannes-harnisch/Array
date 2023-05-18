#pragma once

#include "ContiguousIterator.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hh {

template<typename T, typename AllocatorType = std::allocator<T>>
class VarArray
#ifdef HH_DEBUG
	: public ContainerDebugBase
#endif
{
	using Allocator	  = typename std::allocator_traits<AllocatorType>::template rebind_alloc<T>;
	using AllocTraits = std::allocator_traits<Allocator>;

public:
	using value_type	  = T;
	using allocator_type  = AllocatorType;
	using pointer		  = typename AllocTraits::pointer;
	using const_pointer	  = typename AllocTraits::const_pointer;
	using reference		  = T&;
	using const_reference = const T&;
	using size_type		  = typename AllocTraits::size_type;
	using difference_type = typename AllocTraits::difference_type;

	using iterator				 = ContiguousIterator<T, difference_type, pointer, const_pointer>;
	using const_iterator		 = ContiguousConstIterator<T, difference_type, pointer, const_pointer>;
	using reverse_iterator		 = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	constexpr VarArray() noexcept(std::is_nothrow_default_constructible_v<Allocator>) = default;

	constexpr explicit VarArray(const Allocator& allocator) noexcept :
		alloc {allocator} {
	}

	constexpr explicit VarArray(size_type count, const Allocator& allocator = Allocator()) :
		alloc {allocator},
		count(count) {
		const pointer storage = AllocTraits::allocate(alloc, count);

		if constexpr (std::is_trivially_default_constructible_v<T>) // TODO, construct method?
			std::memset(storage, 0, sizeof(T) * count);
		else {
			pointer		  dst = storage;
			const pointer end = dst + count;
			try {
				for (; dst != end; ++dst)
					AllocTraits::construct(alloc, dst);
			} catch (...) {
				T* const begin = end - count;
				while (dst != begin) {
					--dst;
					AllocTraits::destroy(alloc, dst);
				}
				AllocTraits::deallocate(alloc, storage, count);
				throw;
			}
		}
		alloc.storage = storage;
	}

	constexpr VarArray(size_type count, const T& value, const Allocator& allocator = Allocator()) :
		alloc {allocator},
		count(count) {
		T* storage = AllocTraits::allocate(alloc, count);

		if constexpr (std::is_trivially_copyable_v<T> && sizeof(T) == 1) {
			int byte;
			std::memcpy(&byte, &value, 1);
			std::memset(storage, byte, sizeof(T) * count);
		} else {
			T*		 dst = storage;
			T* const end = dst + count;
			try {
				for (; dst != end; ++dst)
					AllocTraits::construct(alloc, dst, value);
			} catch (...) {
				T* const begin = end - count;
				while (dst != begin) {
					--dst;
					AllocTraits::destroy(alloc, dst);
				}
				AllocTraits::deallocate(alloc, storage, count);
				throw;
			}
		}
		alloc.storage = storage;
	}

	constexpr VarArray(size_type count, std::initializer_list<T> initializers, const Allocator& allocator = Allocator()) :
		alloc {allocator},
		count(count) {
		HH_ASSERT(count >= initializers.size(), "Size of initializer list exceeds array size.");

		alloc.storage = AllocTraits::allocate(alloc, count);
		auto element  = begin();
		for (auto init : initializers)
			AllocTraits::construct(alloc, &*element++, std::move(init));

		auto offset = static_cast<typename iterator::difference_type>(initializers.size());
		for (auto rest = begin() + offset; rest != end(); ++rest)
			AllocTraits::construct(alloc, &*rest);
	}

	template<typename U, typename V>
	constexpr VarArray(size_type				count,
					   std::initializer_list<U> initializers,
					   const V&					fallback,
					   const Allocator&			allocator = Allocator()) :
		VarArray(count, initializers, allocator) {
		auto offset = static_cast<typename iterator::difference_type>(initializers.size());
		for (auto element = begin() + offset; element != end(); ++element)
			AllocTraits::construct(alloc, &*element, fallback);
	}

	constexpr VarArray(const VarArray& that) :
		alloc {AllocTraits::select_on_container_copy_construction(that.alloc)},
		count(that.count) {
		alloc.storage = AllocTraits::allocate(alloc, count);
		auto other	  = that.begin();
		for (auto& element : *this)
			AllocTraits::construct(alloc, &element, *other++);
	}

	constexpr VarArray(VarArray&& that) noexcept :
		alloc {std::move(that.alloc)},
		count(that.count) {
		alloc.storage	   = that.alloc.storage;
		that.alloc.storage = {};
	}

	constexpr ~VarArray() {
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
		return !(*this == that);
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
		return !(*this > that);
	}

	template<typename C>
	constexpr bool operator>=(const C& that) const noexcept {
		return !(*this < that);
	}

#ifdef __cpp_lib_three_way_comparison
	constexpr auto operator<=>(const auto& that) const noexcept {
		return std::lexicographical_compare_three_way(begin(), end(), std::begin(that), std::end(that));
	}
#endif

	constexpr T& operator[](size_type index) noexcept {
		HH_ASSERT(index < count, "index out of range");
		return alloc.storage[index];
	}

	constexpr const T& operator[](size_type index) const noexcept {
		HH_ASSERT(index < count, "index out of range");
		return alloc.storage[index];
	}

	constexpr T& at(size_type index) {
		if (index < count)
			return alloc.storage[index];

		throw std::out_of_range("index out of range");
	}

	constexpr const T& at(size_type index) const {
		if (index < count)
			return alloc.storage[index];

		throw std::out_of_range("index out of range");
	}

	constexpr pointer get(size_type index) noexcept {
		if (index < count)
			return alloc.storage + index;

		return nullptr;
	}

	constexpr const_pointer get(size_type index) const noexcept {
		if (index < count)
			return alloc.storage + index;

		return nullptr;
	}

	constexpr T& front() noexcept {
		HH_ASSERT(count != 0, "can't access front of empty array");
		return alloc.storage[0];
	}

	constexpr const T& front() const noexcept {
		HH_ASSERT(count != 0, "can't access front of empty array");
		return alloc.storage[0];
	}

	constexpr T& back() noexcept {
		HH_ASSERT(count != 0, "can't access back of empty array");
		return alloc.storage[count - 1];
	}

	constexpr const T& back() const noexcept {
		HH_ASSERT(count != 0, "can't access back of empty array");
		return alloc.storage[count - 1];
	}

	constexpr size_type size() const noexcept {
		return count;
	}

	static constexpr size_type max_size() noexcept {
		return std::numeric_limits<size_type>::max();
	}

	[[nodiscard]] constexpr bool empty() const noexcept {
		return count == 0;
	}

	constexpr pointer data() noexcept {
		return alloc.storage;
	}

	constexpr const_pointer data() const noexcept {
		return alloc.storage;
	}

	template<typename U>
	constexpr void fill(const U& value) noexcept {
		std::fill(begin(), end(), value);
	}

	constexpr void reset() noexcept {
		destruct();
		alloc.storage = {};
		count		  = 0;
	}

	constexpr void swap(VarArray& that) noexcept {
		std::swap(alloc, that.alloc);
		std::swap(count, that.count);
	}

	friend constexpr void swap(VarArray& left, VarArray& right) noexcept {
		left.swap(right);
	}

	constexpr iterator begin() noexcept {
#ifdef HH_DEBUG
		return {alloc.storage, *this};
#else
		return {alloc.storage};
#endif
	}

	constexpr const_iterator begin() const noexcept {
#ifdef HH_DEBUG
		return {alloc.storage, *this};
#else
		return {alloc.storage};
#endif
	}

	constexpr iterator end() noexcept {
#ifdef HH_DEBUG
		return {alloc.storage + count, *this};
#else
		return {alloc.storage + count};
#endif
	}

	constexpr const_iterator end() const noexcept {
#ifdef HH_DEBUG
		return {alloc.storage + count, *this};
#else
		return {alloc.storage + count};
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
		VarArray::pointer storage = {};
	} alloc;

	size_type count = {};

	constexpr void destruct() noexcept {
		if constexpr (!std::is_trivially_destructible_v<value_type>)
			for (auto& element : *this)
				AllocTraits::destroy(alloc, &element);

		if (data())
			AllocTraits::deallocate(alloc, data(), count);
	}
};

} // namespace hh
