#pragma once

#include <algorithm>
#include <compare>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
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

namespace hh
{
	template<typename FixedList, bool CONST> class FixedListIterator
	{
		friend FixedList;
		template<typename, bool> friend class FixedListIterator;
		template<typename> friend struct std::pointer_traits;

	public:
		using iterator_concept = std::contiguous_iterator_tag;
		using pointer		   = std::conditional_t<CONST, typename FixedList::const_pointer, typename FixedList::pointer>;
		using reference		   = std::conditional_t<CONST, typename FixedList::const_reference, typename FixedList::reference>;
		using value_type	   = typename FixedList::value_type;
		using difference_type  = typename FixedList::difference_type;

		FixedListIterator() = default;

		operator FixedListIterator<FixedList, true>() const noexcept
		{
#if DEBUG
			return {ptr, list};
#else
			return {ptr};
#endif
		}

		reference operator*() const noexcept
		{
			return *operator->();
		}

		pointer operator->() const noexcept
		{
			HH_ASSERT(list->data() <= ptr && ptr < list->data() + list->size(),
					  "Tried to dereference value-initialized or end iterator.");
			return ptr;
		}

		template<bool CONST2> bool operator==(FixedListIterator<FixedList, CONST2> that) const noexcept
		{
			return ptr == that.ptr;
		}

		template<bool CONST2> bool operator!=(FixedListIterator<FixedList, CONST2> that) const noexcept
		{
			return ptr != that.ptr;
		}

		template<bool CONST2> bool operator<(FixedListIterator<FixedList, CONST2> that) const noexcept
		{
			return ptr < that.ptr;
		}

		template<bool CONST2> bool operator<=(FixedListIterator<FixedList, CONST2> that) const noexcept
		{
			return ptr <= that.ptr;
		}

		template<bool CONST2> bool operator>(FixedListIterator<FixedList, CONST2> that) const noexcept
		{
			return ptr > that.ptr;
		}

		template<bool CONST2> bool operator>=(FixedListIterator<FixedList, CONST2> that) const noexcept
		{
			return ptr >= that.ptr;
		}

		template<bool CONST2> std::strong_ordering operator<=>(FixedListIterator<FixedList, CONST2> that) const noexcept
		{
			return ptr <=> that.ptr;
		}

		FixedListIterator& operator++() noexcept
		{
			return *this += 1;
		}

		FixedListIterator operator++(int) noexcept
		{
			auto old = *this;
			*this += 1;
			return old;
		}

		FixedListIterator& operator--() noexcept
		{
			return *this -= 1;
		}

		FixedListIterator operator--(int) noexcept
		{
			auto old = *this;
			*this -= 1;
			return old;
		}

		FixedListIterator& operator+=(difference_type offset) noexcept
		{
			HH_ASSERT(offset == 0 || ptr, "Cannot offset value-initialized iterator.");
			HH_ASSERT(offset >= 0 || offset >= list->data() - ptr, "Cannot offset list iterator before begin.");
			HH_ASSERT(offset <= 0 || offset <= list->data() + list->size() - ptr, "Cannot offset list iterator past end.");

			ptr += offset;
			return *this;
		}

		FixedListIterator operator+(difference_type offset) const noexcept
		{
			auto old = *this;
			return old += offset;
		}

		friend FixedListIterator operator+(difference_type offset, FixedListIterator iterator) noexcept
		{
			return iterator + offset;
		}

		FixedListIterator& operator-=(difference_type offset) noexcept
		{
			return *this += -offset;
		}

		FixedListIterator operator-(difference_type offset) const noexcept
		{
			auto old = *this;
			return old -= offset;
		}

		template<bool CONST2> difference_type operator-(FixedListIterator<FixedList, CONST2> that) const noexcept
		{
			return ptr - that.ptr;
		}

		reference operator[](difference_type offset) const noexcept
		{
			return *(*this + offset);
		}

	private:
		pointer ptr = nullptr;
#if DEBUG
		FixedList const* list = nullptr;

		FixedListIterator(pointer ptr, FixedList const* list) noexcept : ptr(ptr), list(list)
		{}
#else
		FixedListIterator(pointer ptr) noexcept : ptr(ptr)
		{}
#endif
	};

	// Dynamic array with compile-time fixed capacity. Uses no internal dynamic allocation.
	template<typename T, size_t CAPACITY> class FixedList
	{
	public:
		static_assert(!std::is_const_v<T>, "Const value types are not supported. Make the fixed list itself const instead.");

		using value_type	  = T;
		using reference		  = T&;
		using const_reference = T const&;
		using pointer		  = T*;
		using const_pointer	  = T const*;
		using size_type		  = std::size_t;
		using difference_type = std::ptrdiff_t;

		using count_type = std::conditional_t<
			CAPACITY <= std::numeric_limits<uint8_t>::max() && alignof(T) <= alignof(uint8_t),
			uint8_t,
			std::conditional_t<CAPACITY <= std::numeric_limits<uint16_t>::max() && alignof(T) <= alignof(uint16_t),
							   uint16_t,
							   std::conditional_t<alignof(T) <= alignof(uint32_t), uint32_t, uint64_t>>>;

		using iterator				 = FixedListIterator<FixedList, false>;
		using const_iterator		 = FixedListIterator<FixedList, true>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static size_t capacity() noexcept
		{
			return CAPACITY;
		}

		static size_t max_size() noexcept
		{
			return CAPACITY;
		}

		// Creates an empty fixed list.
		FixedList() = default;

		// Creates a fixed list filled with count default-constructed elements.
		FixedList(size_t count) noexcept(std::is_nothrow_default_constructible_v<T>) :
			elem_count(static_cast<count_type>(count))
		{
			HH_ASSERT(count <= CAPACITY, "Requested size exceeded capacity.");

			std::uninitialized_default_construct(begin(), end());
		}

		// Creates a fixed list filled with count instances of the specified value.
		FixedList(size_t count, T const& value) noexcept(std::is_nothrow_copy_constructible_v<T>) :
			elem_count(static_cast<count_type>(count))
		{
			HH_ASSERT(count <= CAPACITY, "Requested size exceeded capacity.");

			std::uninitialized_fill(begin(), end(), value);
		}

		// Creates a fixed list filled with the elements from the range between first and last (exclusive).
		template<std::forward_iterator It>
		FixedList(It first, It last) : elem_count(static_cast<count_type>(std::distance(first, last)))
		{
			HH_ASSERT(std::distance(first, last) <= CAPACITY, "Size of range exceeded capacity.");

			std::uninitialized_copy(first, last, begin());
		}

		// Creates a fixed list filled with the elements from the range between first and last (exclusive).
		template<std::input_iterator It> FixedList(It first, It last) : elem_count(static_cast<count_type>(CAPACITY))
		{
			auto it = std::uninitialized_copy(first, last, begin());

			elem_count = static_cast<count_type>(it - begin());
		}

		// Creates a fixed list from an initializer list.
		FixedList(std::initializer_list<T> init) noexcept(std::is_nothrow_copy_constructible_v<T>) :
			elem_count(static_cast<count_type>(init.size()))
		{
			HH_ASSERT(init.size() <= CAPACITY, "Size of initializer list exceeded capacity.");

			std::uninitialized_copy(init.begin(), init.end(), begin());
		}

		FixedList(FixedList const&) requires std::is_trivially_copy_constructible_v<T>
		= default;

		FixedList(FixedList const& that) noexcept(std::is_nothrow_copy_constructible_v<T>) : elem_count(that.elem_count)
		{
			std::uninitialized_copy(that.begin(), that.end(), begin());
		}

		FixedList(FixedList&&) requires std::is_trivially_move_constructible_v<T>
		= default;

		FixedList(FixedList&& that) noexcept(std::is_nothrow_move_constructible_v<T>) : elem_count(that.elem_count)
		{
			std::uninitialized_move(that.begin(), that.end(), begin());
		}

		~FixedList() requires std::is_trivially_destructible_v<T>
		= default;

		~FixedList()
		{
			destruct();
		}

		FixedList& operator=(FixedList const&) requires std::is_trivially_copy_assignable_v<T>
		= default;

		// Destroys all existing elements, then copy-constructs the elements from the assigned list.
		FixedList& operator=(FixedList const& that) noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			assign(that.begin(), that.end());
			return *this;
		}

		FixedList& operator=(FixedList&&) requires std::is_trivially_move_assignable_v<T>
		= default;

		// Destroys all existing elements, then move-constructs the elements from the assigned list.
		FixedList& operator=(FixedList&& that) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			destruct();
			elem_count = that.elem_count;
			std::uninitialized_move(that.begin(), that.end(), begin());
			return *this;
		}

		// Compares the list with any container, by comparing all elements individually.
		bool operator==(auto const& that) const noexcept
		{
			return size() == std::size(that) && std::equal(begin(), end(), std::begin(that));
		}

		// Compares the list with any container, by comparing all elements individually.
		bool operator!=(auto const& that) const noexcept
		{
			return !operator==(that);
		}

		// Lexicographically compares the list with any container, by comparing all elements individually.
		bool operator<(auto const& that) const noexcept
		{
			return std::lexicographical_compare(begin(), end(), std::begin(that), std::end(that));
		}

		// Lexicographically compares the list with any container, by comparing all elements individually.
		bool operator<=(auto const& that) const noexcept
		{
			return !operator>(that);
		}

		// Lexicographically compares the list with any container, by comparing all elements individually.
		bool operator>(auto const& that) const noexcept
		{
			return that < *this;
		}

		// Lexicographically compares the list with any container, by comparing all elements individually.
		bool operator>=(auto const& that) const noexcept
		{
			return !operator<(that);
		}

		// Lexicographically compares the list with any container, by comparing all elements individually.
		auto operator<=>(auto const& that) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), std::begin(that), std::end(that));
		}

		// Retrieves the element at the specified index. Out-of-bounds access is asserted on if DEBUG is defined, otherwise the
		// behavior is undefined.
		T& operator[](size_t index) noexcept
		{
			return common_subscript(this, index);
		}

		// Retrieves the element at the specified index. Out-of-bounds access is asserted on if DEBUG is defined, otherwise the
		// behavior is undefined.
		T const& operator[](size_t index) const noexcept
		{
			return common_subscript(this, index);
		}

		// Retrieves the element at the specified index. Out-of-bounds access throws std::out_of_range.
		T& at(size_t index)
		{
			return common_at(this, index);
		}

		// Retrieves the element at the specified index. Out-of-bounds access throws std::out_of_range.
		T const& at(size_t index) const
		{
			return common_at(this, index);
		}

		// Retrieves a pointer to the element at the specified index. Out-of-bounds access returns a nullptr.
		T* get(size_t index) noexcept
		{
			return common_get(this, index);
		}

		// Retrieves a pointer to the element at the specified index. Out-of-bounds access returns a nullptr.
		T const* get(size_t index) const noexcept
		{
			return common_get(this, index);
		}

		// Retrieves the first element. Calling it on an empty list is asserted on when DEBUG is defined, otherwise the behavior
		// is undefined.
		T& front() noexcept
		{
			return (*this)[0];
		}

		// Retrieves the first element. Calling it on an empty list is asserted on when DEBUG is defined, otherwise the behavior
		// is undefined.
		T const& front() const noexcept
		{
			return (*this)[0];
		}

		// Retrieves the last element. Calling it on an empty list is asserted on when DEBUG is defined, otherwise the behavior
		// is undefined.
		T& back() noexcept
		{
			return (*this)[elem_count - 1];
		}

		// Retrieves the last element. Calling it on an empty list is asserted on when DEBUG is defined, otherwise the behavior
		// is undefined.
		T const& back() const noexcept
		{
			return (*this)[elem_count - 1];
		}

		// Returns a pointer to the first element of the array.
		T* data() noexcept
		{
			return std::launder(reinterpret_cast<T*>(storage));
		}

		// Returns a pointer to the first element of the array.
		T const* data() const noexcept
		{
			return std::launder(reinterpret_cast<T const*>(storage));
		}

		// Indicates whether the container is empty.
		[[nodiscard]] bool empty() const noexcept
		{
			return !elem_count;
		}

		// Indicates whether the container is out of capacity.
		bool full() const noexcept
		{
			return elem_count == CAPACITY;
		}

		// Returns the amount of elements in the container.
		size_t size() const noexcept
		{
			return elem_count;
		}

		// Returns the amount of elements in the container as the type that the container uses internally to track the count.
		count_type count() const noexcept
		{
			return elem_count;
		}

		// Replaces all elements in the container with count instances of the specified value. The container remains empty if an
		// exception is thrown during element construction. If the specified amount exceeds the capacity, an assert occurs when
		// DEBUG is defined, otherwise the behavior is undefined.
		void assign(size_t count, T const& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			HH_ASSERT(count <= CAPACITY, "Requested size exceeded capacity.");

			destruct();
			elem_count = static_cast<count_type>(count);
			try
			{
				std::uninitialized_fill_n(begin(), count, value);
			}
			catch(...)
			{
				elem_count = 0;
				throw;
			}
		}

		// Replaces all elements in the container with the elements from the range between first and last (exclusive). The
		// container remains empty if an exception is thrown during element construction. If the size of the range exceeds the
		// capacity, an assert occurs when DEBUG is defined, otherwise the behavior is undefined.
		template<std::forward_iterator It> It assign(It first, It last)
		{
			auto dist = std::distance(first, last);
			HH_ASSERT(dist <= CAPACITY, "List does not have enough capacity.");

			destruct();
			elem_count = static_cast<count_type>(dist);
			try
			{
				std::uninitialized_copy(first, last, begin());
				return last;
			}
			catch(...)
			{
				elem_count = 0;
				throw;
			}
		}

		// Replaces all elements in the container with the elements from the range between first and last (exclusive). The
		// container remains empty if an exception is thrown during element construction. If the size of the range exceeds the
		// capacity, an assert occurs when DEBUG is defined, otherwise the behavior is undefined.
		template<std::input_iterator It> It assign(It first, It last)
		{
			clear();
			try
			{
				while(first != last)
				{
					emplace_back(*first);
					++first;
				}
				return first;
			}
			catch(...)
			{
				clear();
				throw;
			}
		}

		// Replaces all elements in the container with the elements from the initializer list. The container remains empty if an
		// exception is thrown during element construction. If the size of the range exceeds the capacity, an assert occurs when
		// DEBUG is defined, otherwise the behavior is undefined.
		void assign(std::initializer_list<T> init) noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			assign(init.begin(), init.end());
		}

		// Inserts the specified value before the element at pos and returns an iterator to the inserted element. The container
		// remains unaffected if an exception is thrown during construction of the element. If the container is out of capacity,
		// an assert occurs when DEBUG is defined, otherwise the behavior is undefined.
		template<typename U>
		iterator insert(iterator pos,
						U&& value) noexcept(std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, U&&>)
		{
			return emplace(pos, std::forward<U>(value));
		}

		// Tries to insert the specified value before the element at pos and returns the end iterator if the container is full,
		// otherwise an iterator to the inserted element. The container remains unaffected if an exception is thrown during
		// construction of the element.
		template<typename U>
		[[nodiscard]] iterator try_insert(iterator pos, U&& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, U&&>)
		{
			return try_emplace(pos, std::forward<U>(value));
		}

		// Inserts count instances of the specified value before the element at pos and returns pos if count is zero, otherwise
		// an iterator to the first inserted element. The container remains unaffected if an exception is thrown during element
		// construction. If the container is out of capacity, an assert occurs when DEBUG is defined, otherwise the behavior is
		// undefined.
		iterator insert(iterator pos, size_t count, T const& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_copy_constructible_v<T>)
		{
			HH_ASSERT(elem_count + count <= CAPACITY, "List is out of capacity.");

			return insert_count_unchecked(pos, count, value);
		}

		// Tries to insert count instances of the specified value before the element at pos and returns the end iterator if the
		// container would run out of capacity, otherwise pos if count is zero, otherwise an iterator to the first inserted
		// element. The container remains unaffected if an exception is thrown during element construction.
		[[nodiscard]] iterator try_insert(iterator pos, size_t count, T const& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_copy_constructible_v<T>)
		{
			if(elem_count + count > CAPACITY)
				return end();

			return insert_count_unchecked(pos, count, value);
		}

		// Inserts elements from the range between first and last (exclusive) before the element at pos and returns pos if first
		// == last, otherwise an iterator to the first inserted element. The container remains unaffected if an exception is
		// thrown during element construction. If the container is out of capacity, an assert occurs when DEBUG is defined,
		// otherwise the behavior is undefined.
		template<std::forward_iterator It> iterator insert(iterator pos, It first, It last)
		{
			auto dist = std::distance(first, last);
			HH_ASSERT(elem_count + dist <= CAPACITY, "List is out of capacity.");

			return insert_range_unchecked(pos, first, dist);
		}

		// Inserts elements from the range between first and last (exclusive) before the element at pos and returns pos if first
		// == last, otherwise an iterator to the first inserted element. The container remains unaffected if an exception is
		// thrown during element construction. If the container is out of capacity, an assert occurs when DEBUG is defined,
		// otherwise the behavior is undefined.
		template<std::input_iterator It> iterator insert(iterator pos, It first, It last)
		{
			auto pos_ptr = std::to_address(pos);
			try
			{
				while(first != last)
				{
					auto end_ptr = std::to_address(end());
					move_right_unchecked(pos_ptr, end_ptr, end_ptr + 1);

					++elem_count;
					std::construct_at(pos_ptr++, *first);
					++first;
				}
				return make_iterator(this, std::to_address(pos));
			}
			catch(...)
			{
				auto orig_pos_ptr = std::to_address(pos);
				std::destroy(orig_pos_ptr, pos_ptr);
				move_left_unchecked(pos_ptr, std::to_address(end()), orig_pos_ptr);
				elem_count -= static_cast<count_type>(pos_ptr - orig_pos_ptr);
				throw;
			}
		}

		// Tries to insert elements from the range between first and last (exclusive) before the element at pos and returns the
		// end iterator if the container would run out of capacity, otherwise pos if first == last, otherwise an iterator to the
		// first inserted element. The container remains unaffected if an exception is thrown during element construction.
		template<std::forward_iterator It> [[nodiscard]] iterator try_insert(iterator pos, It first, It last)
		{
			auto dist = std::distance(first, last);

			if(elem_count + dist > CAPACITY)
				return end();

			return insert_range_unchecked(pos, first, dist);
		}

		// Tries to insert elements from the range between first and last (exclusive) before the element at pos and returns the
		// end iterator if the container would run out of capacity, otherwise pos if first == last, otherwise an iterator to the
		// first inserted element. The container remains unaffected if an exception is thrown during element construction.
		template<std::input_iterator It> [[nodiscard]] iterator try_insert(iterator pos, It first, It last)
		{
			auto pos_ptr = std::to_address(pos);
			try
			{
				while(first != last)
				{
					if(elem_count == CAPACITY)
					{
						erase(pos, make_iterator(this, pos_ptr));
						return end();
					}

					auto end_ptr = std::to_address(end());
					move_right_unchecked(pos_ptr, end_ptr, end_ptr + 1);

					++elem_count;
					std::construct_at(pos_ptr++, *first);
					++first;
				}
				return make_iterator(this, std::to_address(pos));
			}
			catch(...)
			{
				auto orig_pos_ptr = std::to_address(pos);
				std::destroy(orig_pos_ptr, pos_ptr);
				move_left_unchecked(pos_ptr, std::to_address(end()), orig_pos_ptr);
				elem_count -= static_cast<count_type>(pos_ptr - orig_pos_ptr);
				throw;
			}
		}

		// Inserts elements from the initializer list before the element at pos and returns pos if the initializer list is
		// empty, otherwise an iterator to the first inserted element. The container remains unaffected if an exception is
		// thrown during element construction. If the container is out of capacity, an assert occurs when DEBUG is defined,
		// otherwise the behavior is undefined.
		iterator insert(iterator pos, std::initializer_list<T> init) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_copy_constructible_v<T>)
		{
			return insert(pos, init.begin(), init.end());
		}

		// Tries to insert elements from the initializer list before the element at pos and returns the end iterator if the
		// container would run out of capacity, otherwise pos if the initializer list is empty, otherwise an iterator to the
		// first inserted element. The container remains unaffected if an exception is thrown during element construction.
		[[nodiscard]] iterator try_insert(iterator pos, std::initializer_list<T> init) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_copy_constructible_v<T>)
		{
			return try_insert(pos, init.begin(), init.end());
		}

		// Constructs a new element from the specified arguments before the element at pos and returns an iterator to the
		// new element. The container remains unaffected if an exception is thrown during element construction. If the container
		// is out of capacity, an assert occurs when DEBUG is defined, otherwise the behavior is undefined.
		template<typename... Ts>
		iterator emplace(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			HH_ASSERT(elem_count < CAPACITY, "List is out of capacity.");

			return make_iterator(this, emplace_unchecked(pos, std::forward<Ts>(ts)...));
		}

		// Tries to construct a new element from the specified arguments before the element at pos and returns the end iterator
		// if the container is full, otherwise an iterator to the new element. The container remains unaffected if an exception
		// is thrown during element construction.
		template<typename... Ts>
		[[nodiscard]] iterator try_emplace(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			if(elem_count == CAPACITY)
				return end();

			return make_iterator(this, emplace_unchecked(pos, std::forward<Ts>(ts)...));
		}

		// Appends a new element constructed from the specified arguments and returns an iterator to it. The container remains
		// unaffected if an exception is thrown.
		template<typename... Ts> T& emplace_back(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			HH_ASSERT(elem_count < CAPACITY, "List is out of capacity.");

			return *push_unchecked(std::forward<Ts>(ts)...);
		}

		// Appends a new element constructed from the specified arguments and returns nullptr if the container is full,
		// otherwise a pointer to the new element. The container remains unaffected if an exception is thrown.
		template<typename... Ts>
		[[nodiscard]] T* try_emplace_back(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			if(elem_count == CAPACITY)
				return nullptr;

			return push_unchecked(std::forward<Ts>(ts)...);
		}

		// Removes the last element from the container. If the container is empty, an assert occurs when DEBUG is defined,
		// otherwise the behavior is undefined.
		void pop_back() noexcept
		{
			HH_ASSERT(elem_count, "Cannot pop elements in an empty list.");
			pop_unchecked();
		}

		// Attempts to remove the last element from the container. Returns false if the container was empty, otherwise true.
		[[nodiscard]] bool try_pop_back() noexcept
		{
			if(empty())
				return false;

			pop_unchecked();
			return true;
		}

		// Removes the element at pos and returns an iterator to the element that came after it.
		iterator erase(iterator pos) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			HH_ASSERT(pos >= begin() && pos < end(), "Cannot erase with an invalid iterator.");

			auto pos_ptr = std::to_address(pos);
			std::destroy_at(pos_ptr);
			move_left_unchecked(pos_ptr + 1, std::to_address(end()), pos_ptr);
			--elem_count;
			return make_iterator(this, pos_ptr);
		}

		// Removes all elements in the specified range and returns an iterator to the element that came after the last removed
		// element.
		iterator erase(iterator first, iterator last) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			HH_ASSERT(first <= last && first >= begin() && last <= end(), "Cannot erase an invalid range.");

			auto first_ptr = std::to_address(first);
			auto last_ptr  = std::to_address(last);
			std::destroy(first_ptr, last_ptr);

			move_left_unchecked(last_ptr, std::to_address(end()), first_ptr);
			elem_count -= static_cast<count_type>(last_ptr - first_ptr);
			return make_iterator(this, first_ptr);
		}

		// Removes all elements from the container.
		void clear() noexcept
		{
			destruct();
			elem_count = 0;
		}

		// Swaps two containers.
		void swap(FixedList& that) noexcept(std::is_nothrow_swappable_v<T>)
		{
			std::swap(*this, that);
		}

		iterator begin() noexcept
		{
			return make_iterator(this, data());
		}

		const_iterator begin() const noexcept
		{
			return make_iterator(this, data());
		}

		iterator end() noexcept
		{
			return make_iterator(this, data() + elem_count);
		}

		const_iterator end() const noexcept
		{
			return make_iterator(this, data() + elem_count);
		}

		reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(end());
		}

		const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin());
		}

		const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}

		const_iterator cbegin() const noexcept
		{
			return begin();
		}

		const_iterator cend() const noexcept
		{
			return end();
		}

		const_reverse_iterator crbegin() const noexcept
		{
			return rbegin();
		}

		const_reverse_iterator crend() const noexcept
		{
			return rend();
		}

	private:
		count_type elem_count = 0;
		alignas(T) char storage[sizeof(T) * CAPACITY];

		static auto& common_subscript(auto self, size_t index) noexcept
		{
			HH_ASSERT(index < self->elem_count, "Tried to access list out of range.");
			return self->data()[index];
		}

		static auto& common_at(auto self, size_t index)
		{
			if(index < self->elem_count)
				return self->data()[index];

			throw std::out_of_range("Index into list was out of range.");
		}

		static auto common_get(auto self, size_t index) noexcept
		{
			if(index < self->elem_count)
				return self->data() + index;

			return static_cast<decltype(self->data())>(nullptr);
		}

		template<typename S>
		static auto make_iterator([[maybe_unused]] S self, auto position) noexcept
			-> std::conditional_t<std::is_const_v<std::remove_pointer_t<S>>, const_iterator, iterator>
		{
#if DEBUG
			return {position, self};
#else
			return {position};
#endif
		}

		static void move_left_unchecked(T* src_begin, T* src_end, T* dst_begin)
		{
			if constexpr(std::is_trivially_move_constructible_v<T>)
				std::memmove(dst_begin, src_begin, (src_end - src_begin) * sizeof(T));
			else
				while(src_begin != src_end)
					std::construct_at(dst_begin++, std::move(*src_begin++));
		}

		static void move_right_unchecked(T* src_begin, T* src_end, T* dst_end)
		{
			if constexpr(std::is_trivially_move_constructible_v<T>)
			{
				auto count = src_end - src_begin;
				std::memmove(dst_end - count, src_begin, count * sizeof(T));
			}
			else
				while(src_begin != src_end)
					std::construct_at(--dst_end, std::move(*--src_end));
		}

		template<typename... Ts> T* push_unchecked(Ts&&... ts)
		{
			auto ptr = std::construct_at(data() + elem_count, std::forward<Ts>(ts)...);
			++elem_count;
			return ptr;
		}

		template<typename... Ts> T* emplace_unchecked(iterator pos, Ts&&... ts)
		{
			auto pos_ptr = std::to_address(pos);
			auto end_ptr = std::to_address(end());
			move_right_unchecked(pos_ptr, end_ptr, end_ptr + 1);

			try
			{
				auto ptr = std::construct_at(pos_ptr, std::forward<Ts>(ts)...);
				++elem_count;
				return ptr;
			}
			catch(...)
			{
				move_left_unchecked(pos_ptr + 1, end_ptr + 1, pos_ptr);
				throw;
			}
		}

		iterator insert_count_unchecked(iterator pos, size_t const count, T const& value)
		{
			auto pos_ptr = std::to_address(pos);
			auto end_ptr = std::to_address(end());
			move_right_unchecked(pos_ptr, end_ptr, end_ptr + count);

			try
			{
				auto counter = count;
				while(counter--)
					std::construct_at(pos_ptr++, value);

				elem_count += static_cast<count_type>(count);
				return make_iterator(this, std::to_address(pos));
			}
			catch(...)
			{
				auto orig_pos_ptr = std::to_address(pos);
				std::destroy(orig_pos_ptr, pos_ptr);
				move_left_unchecked(orig_pos_ptr + count, end_ptr + count, orig_pos_ptr);
				throw;
			}
		}

		template<std::forward_iterator It>
		iterator insert_range_unchecked(iterator pos, It first, typename std::iterator_traits<It>::difference_type const dist)
		{
			auto pos_ptr = std::to_address(pos);
			auto end_ptr = std::to_address(end());
			move_right_unchecked(pos_ptr, end_ptr, end_ptr + dist);

			try
			{
				auto distance = dist;
				while(distance--)
					std::construct_at(pos_ptr++, *first++);

				elem_count += static_cast<count_type>(dist);
				return make_iterator(this, std::to_address(pos));
			}
			catch(...)
			{
				auto orig_pos_ptr = std::to_address(pos);
				std::destroy(orig_pos_ptr, pos_ptr);
				move_left_unchecked(orig_pos_ptr + dist, end_ptr + dist, orig_pos_ptr);
				throw;
			}
		}

		void pop_unchecked() noexcept
		{
			std::destroy_at(data() + --elem_count);
		}

		void destruct() noexcept
		{
			std::destroy(begin(), end());
		}
	};
}

namespace std
{
	template<typename FixedList> struct pointer_traits<hh::FixedListIterator<FixedList, true>>
	{
		using pointer		  = hh::FixedListIterator<FixedList, true>;
		using element_type	  = typename pointer::value_type const;
		using difference_type = typename pointer::difference_type;

		[[nodiscard]] static constexpr element_type* to_address(pointer it) noexcept
		{
			HH_ASSERT(it.list->data() <= it.ptr && it.ptr <= it.list->data() + it.list->size(),
					  "Iterator is not within a validly addressable range.");
			return it.ptr;
		}
	};

	template<typename FixedList> struct pointer_traits<hh::FixedListIterator<FixedList, false>>
	{
		using pointer		  = hh::FixedListIterator<FixedList, false>;
		using element_type	  = typename pointer::value_type;
		using difference_type = typename pointer::difference_type;

		[[nodiscard]] static constexpr element_type* to_address(pointer it) noexcept
		{
			HH_ASSERT(it.list->data() <= it.ptr && it.ptr <= it.list->data() + it.list->size(),
					  "Iterator is not within a validly addressable range.");
			return it.ptr;
		}
	};
}
