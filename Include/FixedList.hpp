#ifndef HH_STATIC_LIST
#define HH_STATIC_LIST

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
	template<typename V> class FixedListIterator
	{
		template<typename> friend class FixedListIterator;
		template<typename, unsigned> friend class FixedList;
		template<typename> friend struct std::pointer_traits;

	public:
		using iterator_concept = std::contiguous_iterator_tag;
		using value_type	   = V;
		using pointer		   = V*;
		using reference		   = V&;
		using difference_type  = std::ptrdiff_t;

		FixedListIterator() = default;

		operator FixedListIterator<V const>() const noexcept
		{
			return
			{
				pos,
#if DEBUG
					begin, end
#endif
			};
		}

		V& operator*() const noexcept
		{
			return *operator->();
		}

		V* operator->() const noexcept
		{
			HH_ASSERT(begin <= pos && pos < end, "Tried to dereference value-initialized or end iterator.");
			return pos;
		}

		template<typename U> bool operator==(FixedListIterator<U> that) const noexcept
		{
			return pos == that.pos;
		}

		template<typename U> bool operator!=(FixedListIterator<U> that) const noexcept
		{
			return pos != that.pos;
		}

		template<typename U> bool operator<(FixedListIterator<U> that) const noexcept
		{
			return pos < that.pos;
		}

		template<typename U> bool operator<=(FixedListIterator<U> that) const noexcept
		{
			return pos <= that.pos;
		}

		template<typename U> bool operator>(FixedListIterator<U> that) const noexcept
		{
			return pos > that.pos;
		}

		template<typename U> bool operator>=(FixedListIterator<U> that) const noexcept
		{
			return pos >= that.pos;
		}

		template<typename U> std::strong_ordering operator<=>(FixedListIterator<U> that) const noexcept
		{
			return pos <=> that.pos;
		}

		FixedListIterator& operator++() noexcept
		{
			increment_position(1);
			return *this;
		}

		FixedListIterator operator++(int) noexcept
		{
			auto old = *this;
			increment_position(1);
			return old;
		}

		FixedListIterator& operator--() noexcept
		{
			decrement_position(1);
			return *this;
		}

		FixedListIterator operator--(int) noexcept
		{
			auto old = *this;
			decrement_position(1);
			return old;
		}

		FixedListIterator& operator+=(difference_type offset) noexcept
		{
			increment_position(offset);
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
			decrement_position(offset);
			return *this;
		}

		FixedListIterator operator-(difference_type offset) const noexcept
		{
			auto old = *this;
			return old -= offset;
		}

		template<typename U> difference_type operator-(FixedListIterator<U> that) const noexcept
		{
			return pos - that.pos;
		}

		V& operator[](difference_type offset) const noexcept
		{
			return *(*this + offset);
		}

	private:
		V* pos = {};
#ifdef DEBUG
		V* begin = {};
		V* end	 = {};

		FixedListIterator(V* pos, V* begin, V* end) noexcept : pos(pos), begin(begin), end(end)
		{}
#else
		FixedListIterator(V* pos) noexcept : pos(pos)
		{}
#endif

		void increment_position(difference_type offset) noexcept
		{
			HH_ASSERT(pos, "Cannot increment value-initialized iterator.");
			HH_ASSERT(pos < end, "Cannot increment list iterator past end.");
			pos += offset;
		}

		void decrement_position(difference_type offset) noexcept
		{
			HH_ASSERT(pos, "Cannot decrement value-initialized iterator.");
			HH_ASSERT(begin < pos, "Cannot decrement list iterator before begin.");
			pos -= offset;
		}
	};

	// Automatically allocated dynamic array with a maximum size.
	template<typename T, unsigned Capacity> class FixedList
	{
	public:
		using value_type	  = T;
		using reference		  = T&;
		using const_reference = T const&;
		using pointer		  = T*;
		using const_pointer	  = T const*;
		using size_type		  = std::size_t;
		using difference_type = std::ptrdiff_t;

		using count_type = std::conditional_t<
			Capacity <= std::numeric_limits<uint8_t>::max() && alignof(T) <= alignof(uint8_t),
			uint8_t,
			std::conditional_t<Capacity <= std::numeric_limits<uint16_t>::max() && alignof(T) <= alignof(uint16_t),
							   uint16_t,
							   std::conditional_t<alignof(T) <= alignof(uint32_t), uint32_t, uint64_t>>>;

		using iterator				 = FixedListIterator<value_type>;
		using const_iterator		 = FixedListIterator<value_type const>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static size_type capacity() noexcept
		{
			return Capacity;
		}

		static size_type max_size() noexcept
		{
			return Capacity;
		}

		// Creates an empty fixed list.
		FixedList() = default;

		// Creates a fixed list filled with count default-constructed elements.
		FixedList(size_type count) noexcept(std::is_nothrow_default_constructible_v<T>) :
			elem_count(static_cast<count_type>(count))
		{
			HH_ASSERT(count <= Capacity, "Requested size exceeded capacity.");

			std::uninitialized_default_construct(begin(), end());
		}

		// Creates a fixed list filled with count instances of the specified value.
		FixedList(size_type count, auto const& value) noexcept(std::is_nothrow_constructible_v<T, decltype(value)>) :
			elem_count(static_cast<count_type>(count))
		{
			HH_ASSERT(count <= Capacity, "Requested size exceeded capacity.");

			std::uninitialized_fill(begin(), end(), value);
		}

		// Creates a fixed list filled with the elements from the range between first and last (exclusive).
		template<typename TInputIt>
		FixedList(TInputIt first, TInputIt last) noexcept(
			std::is_nothrow_constructible_v<T, typename std::iterator_traits<TInputIt>::reference>) :
			elem_count(static_cast<count_type>(std::distance(first, last)))
		{
			HH_ASSERT(std::distance(first, last) <= Capacity, "Size of range exceeded capacity.");

			std::uninitialized_copy(first, last, begin());
		}

		// Creates a fixed list from an initializer list.
		template<typename U>
		FixedList(std::initializer_list<U> init) noexcept(std::is_nothrow_constructible_v<T, U const&>) :
			elem_count(static_cast<count_type>(init.size()))
		{
			HH_ASSERT(init.size() <= Capacity, "Size of initializer list exceeded capacity.");

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
		reference operator[](size_type index) noexcept
		{
			return common_subscript(this, index);
		}

		// Retrieves the element at the specified index. Out-of-bounds access is asserted on if DEBUG is defined, otherwise the
		// behavior is undefined.
		const_reference operator[](size_type index) const noexcept
		{
			return common_subscript(this, index);
		}

		// Retrieves the element at the specified index. Out-of-bounds access throws std::out_of_range.
		reference at(size_type index)
		{
			return common_at(this, index);
		}

		// Retrieves the element at the specified index. Out-of-bounds access throws std::out_of_range.
		const_reference at(size_type index) const
		{
			return common_at(this, index);
		}

		// Retrieves a pointer to the element at the specified index. Out-of-bounds access returns a nullptr.
		pointer get(size_type index) noexcept
		{
			return common_get(this, index);
		}

		// Retrieves a pointer to the element at the specified index. Out-of-bounds access returns a nullptr.
		const_pointer get(size_type index) const noexcept
		{
			return common_get(this, index);
		}

		// Retrieves the first element. Calling it on an empty list is asserted on when DEBUG is defined, otherwise the behavior
		// is undefined.
		reference front() noexcept
		{
			return (*this)[0];
		}

		// Retrieves the first element. Calling it on an empty list is asserted on when DEBUG is defined, otherwise the behavior
		// is undefined.
		const_reference front() const noexcept
		{
			return (*this)[0];
		}

		// Retrieves the last element. Calling it on an empty list is asserted on when DEBUG is defined, otherwise the behavior
		// is undefined.
		reference back() noexcept
		{
			return (*this)[elem_count - 1];
		}

		// Retrieves the last element. Calling it on an empty list is asserted on when DEBUG is defined, otherwise the behavior
		// is undefined.
		const_reference back() const noexcept
		{
			return (*this)[elem_count - 1];
		}

		// Replaces all elements in the container with count instances of the specified value. The container remains unaffected
		// when an exception is thrown during element construction. If the specified amount exceeds the capacity, an assert
		// occurs when DEBUG is defined, otherwise the behavior is undefined.
		void assign(size_type count, auto const& value) noexcept(std::is_nothrow_constructible_v<T, decltype(value)>)
		{
			HH_ASSERT(count <= Capacity, "Requested size exceeded capacity.");

			clear();
			while(count--)
				push_unchecked(value);
		}

		// Replaces all elements in the container with the elements from the range between first and last (exclusive). The
		// container remains unaffected when an exception is thrown during element construction. If the size of the range
		// exceeds the capacity, an assert occurs when DEBUG is defined, otherwise the behavior is undefined.
		template<typename TInputIt>
		void assign(TInputIt first, TInputIt last) noexcept(
			std::is_nothrow_constructible_v<T, typename std::iterator_traits<TInputIt>::reference>)
		{
			clear();
			while(first != last)
				emplace_back(*first++);
		}

		// Replaces all elements in the container with the elements from the initializer list. The container remains unaffected
		// when an exception is thrown during element construction. If the size of the range exceeds the capacity, an assert
		// occurs when DEBUG is defined, otherwise the behavior is undefined.
		template<typename U> void assign(std::initializer_list<U> init) noexcept(std::is_nothrow_constructible_v<T, U>)
		{
			assign(init.begin(), init.end());
		}

		// Inserts the specified value before the element at pos and returns an iterator to the inserted element. The container
		// remains unaffected when an exception is thrown during construction of the element. If the container is out of
		// capacity, an assert occurs when DEBUG is defined, otherwise the behavior is undefined.
		template<typename U>
		iterator insert(iterator pos,
						U&& value) noexcept(std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, U&&>)
		{
			return emplace(pos, std::forward<U>(value));
		}

		// Inserts the specified value before the element at pos and returns the end iterator if the container is full,
		// otherwise an iterator to the inserted element. The container remains unaffected when an exception is thrown during
		// construction of the element.
		template<typename U>
		[[nodiscard]] iterator try_insert(iterator pos, U&& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, U&&>)
		{
			return try_emplace(pos, std::forward<U>(value));
		}

		// Inserts count instances of the specified value before the element at pos and returns pos if count is zero, otherwise
		// an iterator to the first inserted element. The container remains unaffected when an exception is thrown during
		// element construction. If the container is out of capacity, an assert occurs when DEBUG is defined, otherwise the
		// behavior is undefined.
		iterator insert(iterator pos, size_type count, auto const& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, decltype(value)>)
		{
			HH_ASSERT(elem_count + count <= Capacity, "List is out of capacity.");

			return insert_unchecked(pos, count, value);
		}

		// Inserts count instances of the specified value before the element at pos and returns the end iterator if the
		// container is full, otherwise pos if count is zero, otherwise an iterator to the first inserted element. The container
		// remains unaffected when an exception is thrown during element construction.
		[[nodiscard]] iterator try_insert(iterator pos, size_type count, auto const& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, decltype(value)>)
		{
			if(elem_count + count > Capacity)
				return end();

			return insert_unchecked(pos, count, value);
		}

		// Inserts elements from the range between first and last (exclusive) before the element at pos and returns pos if first
		// == last, otherwise an iterator to the first inserted element. The container remains unaffected when an exception is
		// thrown during element construction. If the container is out of capacity, an assert occurs when DEBUG is defined,
		// otherwise the behavior is undefined.
		template<typename TInputIt>
		iterator insert(iterator pos, TInputIt first, TInputIt last) noexcept(
			std::is_nothrow_move_constructible_v<T>&&
				std::is_nothrow_constructible_v<T, typename std::iterator_traits<TInputIt>::reference>)
		{
			auto dist = std::distance(first, last);
			HH_ASSERT(elem_count + dist <= Capacity, "List is out of capacity.");

			return insert_unchecked(pos, first, dist);
		}

		// Inserts elements from the range between first and last (exclusive) before the element at pos and returns the end
		// iterator if the container is full, otherwise pos if first == last, otherwise an iterator to the first inserted
		// element. The container remains unaffected when an exception is thrown during element construction.
		template<typename TInputIt>
		[[nodiscard]] iterator try_insert(iterator pos, TInputIt first, TInputIt last) noexcept(
			std::is_nothrow_move_constructible_v<T>&&
				std::is_nothrow_constructible_v<T, typename std::iterator_traits<TInputIt>::reference>)
		{
			auto dist = std::distance(first, last);

			if(elem_count + dist > Capacity)
				return end();

			return insert_unchecked(pos, first, dist);
		}

		// Inserts elements from the initializer list before the element at pos and returns pos if the initializer list is
		// empty, otherwise an iterator to the first inserted element. The container remains unaffected when an exception is
		// thrown during element construction. If the container is out of capacity, an assert occurs when DEBUG is defined,
		// otherwise the behavior is undefined.
		template<typename U>
		iterator insert(iterator pos, std::initializer_list<U> init) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, U>)
		{
			return insert(pos, init.begin(), init.end());
		}

		// Inserts elements from the initializer list before the element at pos and returns the end iterator if the container is
		// full, otherwise pos if the initializer list is empty, otherwise an iterator to the first inserted element. The
		// container remains unaffected when an exception is thrown during element construction.
		template<typename U>
		[[nodiscard]] iterator try_insert(iterator pos, std::initializer_list<U> init) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, U>)
		{
			return try_insert(pos, init.begin(), init.end());
		}

		// Constructs a new element from the specified arguments before the element at pos and returns an iterator to the
		// new element. The container remains unaffected when an exception is thrown during element construction. If the
		// container is out of capacity, an assert occurs when DEBUG is defined, otherwise the behavior is undefined.
		template<typename... Ts>
		iterator emplace(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			HH_ASSERT(elem_count < Capacity, "List is out of capacity.");

			return make_iterator(this, emplace_unchecked(pos, std::forward<Ts>(ts)...));
		}

		// Constructs a new element from the specified arguments before the element at pos and returns the end iterator if the
		// container is full, otherwise an iterator to the new element. The container remains unaffected if an exception is
		// thrown during element construction.
		template<typename... Ts>
		[[nodiscard]] iterator try_emplace(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			if(elem_count == Capacity)
				return end();

			return make_iterator(this, emplace_unchecked(pos, std::forward<Ts>(ts)...));
		}

		// Appends a new element constructed from the specified arguments and returns an iterator to it. The container remains
		// unaffected if an exception is thrown.
		template<typename... Ts> reference emplace_back(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			HH_ASSERT(elem_count < Capacity, "List is out of capacity.");

			return *push_unchecked(std::forward<Ts>(ts)...);
		}

		// Appends a new element constructed from the specified arguments and returns the end iterator if the container is full,
		// otherwise an iterator to the new element. The container remains unaffected if an exception is thrown.
		template<typename... Ts>
		[[nodiscard]] iterator try_emplace_back(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			if(elem_count == Capacity)
				return end();

			return make_iterator(this, push_unchecked(std::forward<Ts>(ts)...));
		}

		void push_back(T const& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			emplace_back(value);
		}

		void push_back(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			emplace_back(std::move(value));
		}

		void pop_back() noexcept
		{
			HH_ASSERT(elem_count, "Cannot pop elements in an empty list.");
			pop_unchecked();
		}

		[[nodiscard]] bool try_pop_back() noexcept
		{
			if(empty())
				return false;

			pop_unchecked();
			return true;
		}

		iterator erase(iterator pos) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			HH_ASSERT(pos >= begin() && pos < end(), "Cannot erase with an invalid iterator.");

			std::destroy_at(pos.pos);
			move_left_unchecked(pos.pos, end().pos, pos.pos);
			--elem_count;
			return pos;
		}

		iterator erase(iterator first, iterator last) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			HH_ASSERT(first <= last && first >= begin() && last <= end(), "Cannot erase an invalid range.");

			std::destroy(first, last);
			move_left_unchecked(last.pos, end().pos, first.pos);
			elem_count -= static_cast<count_type>(last - first);
			return first;
		}

		void clear() noexcept
		{
			destruct();
			elem_count = 0;
		}

		void swap(FixedList& that) noexcept(std::is_nothrow_swappable_v<T>)
		{
			std::swap(*this, that);
		}

		[[nodiscard]] bool empty() const noexcept
		{
			return !elem_count;
		}

		bool full() const noexcept
		{
			return elem_count == Capacity;
		}

		size_type size() const noexcept
		{
			return elem_count;
		}

		count_type count() const noexcept
		{
			return elem_count;
		}

		pointer data() noexcept
		{
			return reinterpret_cast<pointer>(storage);
		}

		const_pointer data() const noexcept
		{
			return reinterpret_cast<const_pointer>(storage);
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
		alignas(T) std::byte storage[sizeof(T) * Capacity];

		static auto& common_subscript(auto self, size_type index) noexcept
		{
			HH_ASSERT(index < self->elem_count, "Tried to access list out of range.");
			return self->data()[index];
		}

		static auto& common_at(auto self, size_type index)
		{
			if(index < self->elem_count)
				return self->data()[index];

			throw std::out_of_range("Index into list was out of range.");
		}

		static auto common_get(auto self, size_type index) noexcept
		{
			if(index < self->elem_count)
				return self->data() + index;

			return static_cast<decltype(self->data())>(nullptr);
		}

		template<typename S>
		static auto make_iterator([[maybe_unused]] S self, auto position) noexcept
			-> std::conditional_t<std::is_const_v<std::remove_pointer_t<S>>, const_iterator, iterator>
		{
#ifdef DEBUG
			auto data = self->data();
			return {position, data, data + self->elem_count};
#else
			return {position};
#endif
		}

		static void move_left_unchecked(T* src_begin, T* src_end, T* dst_begin)
		{
			while(src_begin != src_end)
				std::construct_at(dst_begin++, std::move(*src_begin++));
		}

		static void move_right_unchecked(T* src_begin, T* src_end, T* dst_end)
		{
			while(src_begin != src_end)
				std::construct_at(--dst_end, std::move(*--src_end));
		}

		template<typename... Ts> pointer push_unchecked(Ts&&... ts)
		{
			auto ptr = std::construct_at(data() + elem_count, std::forward<Ts>(ts)...);
			++elem_count;
			return ptr;
		}

		template<typename... Ts> pointer emplace_unchecked(iterator pos, Ts&&... ts)
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
				move_left_unchecked(pos_ptr + 1, end_ptr, pos_ptr);
				throw;
			}
		}

		iterator insert_unchecked(iterator pos, size_type const count, auto const& value)
		{
			auto pos_ptr = std::to_address(pos);
			auto end_ptr = std::to_address(end());
			move_right_unchecked(pos_ptr, end_ptr, end_ptr + count);

			try
			{
				auto counter = count;
				while(counter--)
					std::construct_at(pos_ptr++, value);

				elem_count += count;
				return pos;
			}
			catch(...)
			{
				auto orig_pos_ptr = std::to_address(pos);
				std::destroy(orig_pos_ptr, pos_ptr);
				move_left_unchecked(orig_pos_ptr + count, end_ptr + count, orig_pos_ptr);
				throw;
			}
		}

		template<typename TInputIt> iterator insert_unchecked(iterator pos, TInputIt first, auto const dist)
		{
			auto pos_ptr = std::to_address(pos);
			auto end_ptr = std::to_address(end());
			move_right_unchecked(pos_ptr, end_ptr, end_ptr + dist);

			try
			{
				auto distance = dist;
				while(distance--)
					std::construct_at(pos_ptr++, *first++);

				elem_count += dist;
				return pos;
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
	template<typename T> struct pointer_traits<hh::FixedListIterator<T>>
	{
		using pointer		  = hh::FixedListIterator<T>;
		using element_type	  = typename pointer::value_type;
		using difference_type = typename pointer::difference_type;

		[[nodiscard]] static constexpr element_type* to_address(pointer iter) noexcept
		{
			HH_ASSERT(iter.begin <= iter.pos && iter.pos <= iter.end, "Iterator is not within a validly addressable range.");
			return std::to_address(iter.pos);
		}
	};
}

#endif /* HH_STATIC_LIST */
