#pragma once

#include "Common.hpp"
#include "ContiguousIterator.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hh {

// A variable-length fixed-size dynamically allocated array.
template<typename T, typename Allocator = std::allocator<T>>
class VarArray :
#ifndef NDEBUG
	public ContainerDebugBase,
#endif
	private Allocator {
	using AllocTraits = std::allocator_traits<Allocator>;

public:
	using value_type	  = T;
	using allocator_type  = Allocator;
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

	// Creates an empty array.
	constexpr VarArray() noexcept(std::is_nothrow_default_constructible_v<Allocator>) :
		Allocator(),
		ptr(nullptr),
		count(0) {
	}

	// Creates an empty array with the given allocator.
	constexpr explicit VarArray(const Allocator& allocator) noexcept :
		Allocator(allocator),
		ptr(nullptr),
		count(0) {
	}

	constexpr explicit VarArray(size_type count, const Allocator& allocator = Allocator()) :
		Allocator(allocator),
		ptr(AllocTraits::allocate(get_alloc(), count)),
		count(count) {
		allocator_value_initialize_n(get_alloc(), ptr, count, std::to_address(ptr), static_cast<size_t>(count));
	}

	template<typename U>
	constexpr VarArray(size_type count, const U& value, const Allocator& allocator = Allocator()) :
		Allocator(allocator),
		ptr(AllocTraits::allocate(get_alloc(), count)),
		count(count) {
		allocator_fill_initialize_n(get_alloc(), ptr, count, std::to_address(ptr), value, static_cast<size_t>(count));
	}

	constexpr VarArray(size_type count, std::initializer_list<T> initializers, const Allocator& allocator = Allocator()) :
		Allocator(allocator),
		ptr(AllocTraits::allocate(get_alloc(), count)),
		count(count) {
		const size_t init_count = std::min(initializers.size(), static_cast<size_t>(count));
		Allocator&	 alloc		= get_alloc();

		T* dst = allocator_copy_initialize_n<T>(alloc, ptr, initializers.begin(), count, init_count);

		const size_t rest = static_cast<size_t>(count) - init_count;
		allocator_value_initialize_n(alloc, ptr, count, dst, rest);
	}

	template<typename U>
	constexpr VarArray(size_type				count,
					   std::initializer_list<T> initializers,
					   const U&					fallback,
					   const Allocator&			allocator = Allocator()) :
		Allocator(allocator),
		ptr(AllocTraits::allocate(get_alloc(), count)),
		count(count) {
		const size_t init_count = std::min(initializers.size(), static_cast<size_t>(count));
		Allocator&	 alloc		= get_alloc();

		T* dst = allocator_copy_initialize_n<T>(alloc, ptr, initializers.begin(), count, init_count);

		const size_t rest = static_cast<size_t>(count) - init_count;
		allocator_fill_initialize_n(alloc, ptr, count, dst, fallback, rest);
	}

	constexpr VarArray(std::initializer_list<T> initializers, const Allocator& allocator = Allocator()) :
		Allocator(allocator),
		ptr(AllocTraits::allocate(get_alloc(), initializers.size())),
		count(initializers.size()) {
		allocator_copy_initialize_n<T>(get_alloc(), ptr, initializers.begin(), count, static_cast<size_t>(count));
	}

	constexpr VarArray(const VarArray& other) :
#ifndef NDEBUG
		ContainerDebugBase(other),
#endif
		Allocator(AllocTraits::select_on_container_copy_construction(other.get_alloc())),
		ptr(AllocTraits::allocate(get_alloc(), other.count)),
		count(other.count) {
		allocator_copy_initialize_n<T>(get_alloc(), ptr, other.ptr, count, static_cast<size_t>(count));
	}

	constexpr VarArray(VarArray&& other) noexcept :
#ifndef NDEBUG
		ContainerDebugBase(std::move(other)),
#endif
		Allocator(std::move(other.get_alloc())),
		ptr(other.ptr),
		count(other.count) {
		other.ptr	= nullptr;
		other.count = 0;
	}

	constexpr ~VarArray() {
		delete_data();
	}

	constexpr VarArray& operator=(const VarArray& other) {
		if (this == &other)
			return *this;

#ifndef NDEBUG
		ContainerDebugBase::operator=(other);
#endif

		if constexpr (AllocTraits::propagate_on_container_copy_assignment::value) {
			get_alloc() = other.get_alloc();
		}

		if (count == other.count) {
			T*			 dst = std::to_address(ptr);
			const T*	 src = std::to_address(other.ptr);
			const size_t n	 = static_cast<size_t>(count);
			if constexpr (std::is_trivially_copy_assignable_v<T>) {
				std::memcpy(dst, src, sizeof(T) * n);
			} else {
				T* const end = dst + n;
				for (; dst != end; ++dst, ++src)
					*dst = *src;
			}
		} else {
			delete_data();

			Allocator& alloc = get_alloc();

			ptr	  = AllocTraits::allocate(alloc, other.count);
			count = other.count;
			allocator_copy_initialize_n<T>(alloc, ptr, other.ptr, count, static_cast<size_t>(count));
		}
		return *this;
	}

	constexpr VarArray& operator=(VarArray&& other) noexcept {
#ifndef NDEBUG
		ContainerDebugBase::operator=(std::move(other));
#endif

		if constexpr (AllocTraits::propagate_on_container_move_assignment::value) {
			get_alloc() = std::move(other.get_alloc());
		}

		delete_data();
		ptr	  = other.ptr;
		count = other.count;

		other.ptr	= nullptr;
		other.count = 0;
		return *this;
	}

	constexpr bool operator==(const VarArray& other) const noexcept {
		return count == other.count && std::equal(ptr, ptr + count, other.ptr);
	}

	constexpr bool operator!=(const VarArray& other) const noexcept {
		return !(*this == other);
	}

	constexpr bool operator<(const VarArray& other) const noexcept {
		return std::lexicographical_compare(ptr, ptr + count, other.ptr, other.ptr + other.count);
	}

	constexpr bool operator>(const VarArray& other) const noexcept {
		return other < *this;
	}

	constexpr bool operator<=(const VarArray& other) const noexcept {
		return !(*this > other);
	}

	constexpr bool operator>=(const VarArray& other) const noexcept {
		return !(*this < other);
	}

#ifdef __cpp_lib_three_way_comparison
	constexpr auto operator<=>(const VarArray& other) const noexcept {
		return std::lexicographical_compare_three_way(ptr, ptr + count, other.ptr, other.ptr + other.count);
	}
#endif

	constexpr T& operator[](size_type index) noexcept {
		HH_ASSERT(index < count, "index out of range");
		return ptr[index];
	}

	constexpr const T& operator[](size_type index) const noexcept {
		HH_ASSERT(index < count, "index out of range");
		return ptr[index];
	}

	constexpr T& at(size_type index) {
		if (index < count)
			return ptr[index];

		throw std::out_of_range("index out of range");
	}

	constexpr const T& at(size_type index) const {
		if (index < count)
			return ptr[index];

		throw std::out_of_range("index out of range");
	}

	constexpr pointer get(size_type index) noexcept {
		if (index < count)
			return ptr + index;

		return nullptr;
	}

	constexpr const_pointer get(size_type index) const noexcept {
		if (index < count)
			return ptr + index;

		return nullptr;
	}

	constexpr T& front() noexcept {
		HH_ASSERT(count != 0, "can't access front of empty array");
		return ptr[0];
	}

	constexpr const T& front() const noexcept {
		HH_ASSERT(count != 0, "can't access front of empty array");
		return ptr[0];
	}

	constexpr T& back() noexcept {
		HH_ASSERT(count != 0, "can't access back of empty array");
		return ptr[count - 1];
	}

	constexpr const T& back() const noexcept {
		HH_ASSERT(count != 0, "can't access back of empty array");
		return ptr[count - 1];
	}

	constexpr size_type size() const noexcept {
		return count;
	}

	constexpr size_type max_size() const noexcept {
		return std::min(AllocTraits::max_size(get_alloc()),
						static_cast<size_type>(std::numeric_limits<difference_type>::max()));
	}

	[[nodiscard]] constexpr bool empty() const noexcept {
		return count == 0;
	}

	constexpr pointer data() noexcept {
		return ptr;
	}

	constexpr const_pointer data() const noexcept {
		return ptr;
	}

	constexpr allocator_type get_allocator() const noexcept {
		return static_cast<allocator_type>(get_alloc());
	}

	template<typename U>
	constexpr void fill(const U& value) noexcept {
		std::fill(begin(), end(), value);
	}

	constexpr void reset() noexcept {
		delete_data();
		ptr	  = nullptr;
		count = 0;
	}

	constexpr void swap(VarArray& other) noexcept {
		if constexpr (AllocTraits::propagate_on_container_swap::value) {
			std::swap(get_alloc(), other.get_alloc());
		}

		std::swap(ptr, other.ptr);
		std::swap(count, other.count);
	}

	friend constexpr void swap(VarArray& left, VarArray& right) noexcept {
		left.swap(right);
	}

	constexpr iterator begin() noexcept {
#ifndef NDEBUG
		return {ptr, this};
#else
		return {ptr};
#endif
	}

	constexpr const_iterator begin() const noexcept {
#ifndef NDEBUG
		return {ptr, this};
#else
		return {ptr};
#endif
	}

	constexpr iterator end() noexcept {
#ifndef NDEBUG
		return {ptr + count, this};
#else
		return {ptr + count};
#endif
	}

	constexpr const_iterator end() const noexcept {
#ifndef NDEBUG
		return {ptr + count, this};
#else
		return {ptr + count};
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
	pointer	  ptr;
	size_type count;

	constexpr Allocator& get_alloc() noexcept {
		return static_cast<Allocator&>(*this);
	}

	constexpr const Allocator& get_alloc() const noexcept {
		return static_cast<const Allocator&>(*this);
	}

	constexpr void delete_data() noexcept {
		if constexpr (!std::is_trivially_destructible_v<T>) {
			T* const begin = std::to_address(ptr);
			T*		 it	   = begin + count;
			while (it != begin) {
				--it;
				AllocTraits::destroy(get_alloc(), it);
			}
		}
		AllocTraits::deallocate(get_alloc(), ptr, count);
	}
};

} // namespace hh
