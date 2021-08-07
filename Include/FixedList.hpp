#ifndef HH_STATIC_LIST
#define HH_STATIC_LIST

#include <algorithm>
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
	#define HH_ASSERT(condition, message) assert((message, condition))
#endif

#if !DEBUG
	#undef HH_ASSERT
	#define HH_ASSERT(condition, message)
#endif

namespace hh
{
	template<typename T, size_t Capacity> class FixedList
	{
		template<typename V> class Iterator
		{
			friend FixedList;

		public:
			using iterator_concept = std::contiguous_iterator_tag;
			using value_type	   = V;
			using pointer		   = V*;
			using reference		   = V&;
			using difference_type  = std::ptrdiff_t;

			constexpr Iterator() = default;

			constexpr operator Iterator<V const>() const noexcept
			{
				return
				{
					pos,
#if DEBUG
						begin, end
#endif
				};
			}

			constexpr V& operator*() const noexcept
			{
				return *operator->();
			}

			constexpr V* operator->() const noexcept
			{
				HH_ASSERT(begin <= pos && pos < end, "Tried to dereference value-initialized iterator.");
				return pos;
			}

			template<typename U> constexpr bool operator==(Iterator<U> that) const noexcept
			{
				return pos == that.pos;
			}

			template<typename U> constexpr bool operator!=(Iterator<U> that) const noexcept
			{
				return pos != that.pos;
			}

			template<typename U> constexpr bool operator<(Iterator<U> that) const noexcept
			{
				return pos < that.pos;
			}

			template<typename U> constexpr bool operator<=(Iterator<U> that) const noexcept
			{
				return pos <= that.pos;
			}

			template<typename U> constexpr bool operator>(Iterator<U> that) const noexcept
			{
				return pos > that.pos;
			}

			template<typename U> constexpr bool operator>=(Iterator<U> that) const noexcept
			{
				return pos >= that.pos;
			}

			template<typename U> constexpr std::strong_ordering operator<=>(Iterator<U> that) const noexcept
			{
				return pos <=> that.pos;
			}

			constexpr Iterator& operator++() noexcept
			{
				incrementPosition(1);
				return *this;
			}

			constexpr Iterator operator++(int) noexcept
			{
				auto old = *this;
				incrementPosition(1);
				return old;
			}

			constexpr Iterator& operator--() noexcept
			{
				decrementPosition(1);
				return *this;
			}

			constexpr Iterator operator--(int) noexcept
			{
				auto old = *this;
				decrementPosition(1);
				return old;
			}

			constexpr Iterator& operator+=(difference_type offset) noexcept
			{
				incrementPosition(offset);
				return *this;
			}

			constexpr Iterator operator+(difference_type offset) const noexcept
			{
				auto old = *this;
				return old += offset;
			}

			friend constexpr Iterator operator+(difference_type offset, Iterator iterator) noexcept
			{
				return iterator + offset;
			}

			constexpr Iterator& operator-=(difference_type offset) noexcept
			{
				decrementPosition(offset);
				return *this;
			}

			constexpr Iterator operator-(difference_type offset) const noexcept
			{
				auto old = *this;
				return old -= offset;
			}

			template<typename U> constexpr difference_type operator-(Iterator<U> that) const noexcept
			{
				return pos - that.pos;
			}

			constexpr V& operator[](difference_type offset) const noexcept
			{
				return *(*this + offset);
			}

		private:
			V* pos = {};
#ifdef DEBUG
			V* begin = {};
			V* end	 = {};

			constexpr Iterator(V* pos, V* begin, V* end) noexcept : pos(pos), begin(begin), end(end)
			{}
#else
			constexpr Iterator(V* pos) noexcept : pos(pos)
			{}
#endif

			constexpr void incrementPosition(difference_type offset) noexcept
			{
				HH_ASSERT(pos, "Cannot increment value-initialized iterator.");
				HH_ASSERT(pos < end, "Cannot increment list iterator past end.");
				pos += offset;
			}

			constexpr void decrementPosition(difference_type offset) noexcept
			{
				HH_ASSERT(pos, "Cannot decrement value-initialized iterator.");
				HH_ASSERT(begin < pos, "Cannot decrement list iterator before begin.");
				pos -= offset;
			}
		};

	public:
		using value_type			 = T;
		using reference				 = T&;
		using const_reference		 = T const&;
		using pointer				 = T*;
		using const_pointer			 = T const*;
		using size_type				 = std::size_t;
		using difference_type		 = std::ptrdiff_t;
		using iterator				 = Iterator<value_type>;
		using const_iterator		 = Iterator<value_type const>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using count_type			 = std::conditional_t<
			Capacity <= std::numeric_limits<uint8_t>::max() && alignof(T) <= alignof(uint8_t),
			uint8_t,
			std::conditional_t<
				Capacity <= std::numeric_limits<uint16_t>::max() && alignof(T) <= alignof(uint16_t),
				uint16_t,
				std::conditional_t<Capacity <= std::numeric_limits<uint32_t>::max() && alignof(T) <= alignof(uint32_t),
								   uint32_t,
								   uint64_t>>>;

		static constexpr size_type capacity() noexcept
		{
			return Capacity;
		}

		static constexpr size_type max_size() noexcept
		{
			return Capacity;
		}

		FixedList() = default;

		constexpr FixedList(size_type count) noexcept(std::is_nothrow_default_constructible_v<T>) :
			elemCount(static_cast<count_type>(count))
		{
			HH_ASSERT(count <= Capacity, "Requested size exceeded capacity.");
			std::uninitialized_default_construct(begin(), end());
		}

		constexpr FixedList(size_type count, auto const& value) noexcept(std::is_nothrow_constructible_v<T, decltype(value)>) :
			elemCount(static_cast<count_type>(count))
		{
			HH_ASSERT(count <= Capacity, "Requested size exceeded capacity.");
			std::uninitialized_fill(begin(), end(), value);
		}

		template<typename U>
		constexpr FixedList(std::initializer_list<U> init) : elemCount(static_cast<count_type>(init.size()))
		{
			HH_ASSERT(init.size() <= Capacity, "Size of initializer list exceeded capacity.");
			std::uninitialized_copy(init.begin(), init.end(), begin());
		}

		FixedList(FixedList const&) requires std::is_trivially_copy_constructible_v<T> = default;

		constexpr FixedList(FixedList const& that) noexcept(std::is_nothrow_copy_constructible_v<T>) : elemCount(that.elemCount)
		{
			std::uninitialized_copy(that.begin(), that.end(), begin());
		}

		FixedList(FixedList&&) requires std::is_trivially_move_constructible_v<T> = default;

		constexpr FixedList(FixedList&& that) noexcept(std::is_nothrow_move_constructible_v<T>) : elemCount(that.elemCount)
		{
			std::uninitialized_move(that.begin(), that.end(), begin());
		}

		~FixedList() requires std::is_trivially_destructible_v<T> = default;

		constexpr ~FixedList()
		{
			destruct();
		}

		FixedList& operator=(FixedList const&) requires std::is_trivially_copy_assignable_v<T> = default;

		constexpr FixedList& operator=(FixedList const& that) noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			assign(that.begin(), that.end());
			return *this;
		}

		FixedList& operator=(FixedList&&) requires std::is_trivially_move_assignable_v<T> = default;

		constexpr FixedList& operator=(FixedList&& that) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			destruct();
			std::uninitialized_move(that.begin(), that.end(), begin());
			elemCount = that.elemCount;
			return *this;
		}

		constexpr bool operator==(auto const& that) const noexcept
		{
			return size() == std::size(that) && std::equal(begin(), end(), std::begin(that));
		}

		constexpr bool operator!=(auto const& that) const noexcept
		{
			return !operator==(that);
		}

		constexpr bool operator<(auto const& that) const noexcept
		{
			return std::lexicographical_compare(begin(), end(), std::begin(that), std::end(that));
		}

		constexpr bool operator<=(auto const& that) const noexcept
		{
			return !operator>(that);
		}

		constexpr bool operator>(auto const& that) const noexcept
		{
			return that < *this;
		}

		constexpr bool operator>=(auto const& that) const noexcept
		{
			return !operator<(that);
		}

		constexpr auto operator<=>(auto const& that) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), std::begin(that), std::end(that));
		}

		constexpr reference operator[](size_type index) noexcept
		{
			return commonSubscript(this, index);
		}

		constexpr const_reference operator[](size_type index) const noexcept
		{
			return commonSubscript(this, index);
		}

		constexpr reference at(size_type index)
		{
			return commonAt(this, index);
		}

		constexpr const_reference at(size_type index) const
		{
			return commonAt(this, index);
		}

		constexpr pointer get(size_type index) noexcept
		{
			return commonGet(this, index);
		}

		constexpr const_pointer get(size_type index) const noexcept
		{
			return commonGet(this, index);
		}

		constexpr reference front() noexcept
		{
			return (*this)[0];
		}

		constexpr const_reference front() const noexcept
		{
			return (*this)[0];
		}

		constexpr reference back() noexcept
		{
			return (*this)[elemCount - 1];
		}

		constexpr const_reference back() const noexcept
		{
			return (*this)[elemCount - 1];
		}

		constexpr void assign(size_type count, auto const& value) noexcept(std::is_nothrow_constructible_v<T, decltype(value)>)
		{
			HH_ASSERT(count <= Capacity, "Requested size exceeded capacity.");

			clear();
			while(count--)
				pushUnchecked(value);
		}

		template<typename TInputIt>
		constexpr void assign(TInputIt first, TInputIt last) noexcept(
			std::is_nothrow_constructible_v<T, std::iterator_traits<TInputIt>::reference>)
		{
			clear();
			while(first != last)
				emplace_back(*first++);
		}

		template<typename U>
		constexpr void assign(std::initializer_list<U> init) noexcept(std::is_nothrow_constructible_v<T, U>)
		{
			assign(init.begin(), init.end());
		}

		constexpr iterator insert(iterator pos, auto const& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, decltype(value)>)
		{
			return emplace(pos, value);
		}

		constexpr iterator insert(iterator pos, auto&& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, decltype(value)>)
		{
			return emplace(pos, std::move(value));
		}

		constexpr iterator insert(iterator pos, size_type count, auto const& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, decltype(value)>)
		{
			HH_ASSERT(elemCount + count <= Capacity, "List is out of capacity.");

			moveRightUnchecked(pos.pos, end().pos, end().pos + count);
			auto ptr = pos.pos;
			while(count--)
			{
				std::construct_at(ptr++, value);
				++elemCount;
			}
			return pos;
		}

		template<typename TInputIt>
		constexpr iterator insert(iterator pos, TInputIt first, TInputIt last) noexcept(
			std::is_nothrow_move_constructible_v<T>&&
				std::is_nothrow_constructible_v<T, std::iterator_traits<TInputIt>::reference>)
		{
			auto dist = last - first;
			HH_ASSERT(elemCount + dist <= Capacity, "List is out of capacity.");

			moveRightUnchecked(pos.pos, end().pos, end().pos + dist);
			auto ptr = pos.pos;
			while(first != last)
			{
				std::construct_at(ptr++, *first++);
				++elemCount;
			}
			return pos;
		}

		template<typename U>
		constexpr iterator insert(iterator pos, std::initializer_list<U> init) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, U>)
		{
			return insert(pos, init.begin(), init.end());
		}

		template<typename... Ts>
		constexpr iterator emplace(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			HH_ASSERT(elemCount < Capacity, "List is out of capacity.");
			return makeIterator(this, emplaceUnchecked(pos, std::forward<Ts>(ts)...));
		}

		template<typename... Ts>
		constexpr iterator try_emplace(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			if(elemCount < Capacity)
				return makeIterator(this, emplaceUnchecked(pos, std::forward<Ts>(ts)...));

			return end();
		}

		template<typename... Ts>
		constexpr reference emplace_back(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			HH_ASSERT(elemCount < Capacity, "List is out of capacity.");
			return *pushUnchecked(std::forward<Ts>(ts)...);
		}

		template<typename... Ts>
		constexpr iterator try_emplace_back(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			if(elemCount < Capacity)
				return makeIterator(this, pushUnchecked(std::forward<Ts>(ts)...));

			return end();
		}

		constexpr void push_back(T const& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			emplace_back(value);
		}

		constexpr void push_back(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			emplace_back(std::move(value));
		}

		constexpr void pop_back() noexcept
		{
			HH_ASSERT(elemCount, "Cannot pop elements in an empty list.");
			popUnchecked();
		}

		constexpr bool try_pop_back() noexcept
		{
			if(empty())
				return false;

			popUnchecked();
			return true;
		}

		constexpr iterator erase(iterator pos) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			HH_ASSERT(pos >= begin() && pos < end(), "Cannot erase with an invalid iterator.");

			std::destroy_at(pos.pos);
			moveLeftUnchecked(pos.pos, end().pos, pos.pos);
			--elemCount;
			return pos;
		}

		constexpr iterator erase(iterator first, iterator last) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			HH_ASSERT(first <= last && first >= begin() && last <= end(), "Cannot erase an invalid range.");

			std::destroy(first, last);
			moveLeftUnchecked(last.pos, end().pos, first.pos);
			elemCount -= static_cast<count_type>(last - first);
			return first;
		}

		constexpr void clear() noexcept
		{
			destruct();
			elemCount = 0;
		}

		constexpr void swap(FixedList& that) noexcept(std::is_nothrow_swappable_v<T>)
		{
			std::swap(*this, that);
		}

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return !elemCount;
		}

		constexpr size_type size() const noexcept
		{
			return elemCount;
		}

		constexpr count_type count() const noexcept
		{
			return elemCount;
		}

		constexpr pointer data() noexcept
		{
			return reinterpret_cast<pointer>(storage);
		}

		constexpr const_pointer data() const noexcept
		{
			return reinterpret_cast<const_pointer>(storage);
		}

		constexpr iterator begin() noexcept
		{
			return makeIterator(this, data());
		}

		constexpr const_iterator begin() const noexcept
		{
			return makeIterator(this, data());
		}

		constexpr iterator end() noexcept
		{
			return makeIterator(this, data() + elemCount);
		}

		constexpr const_iterator end() const noexcept
		{
			return makeIterator(this, data() + elemCount);
		}

		constexpr reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(end());
		}

		constexpr const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		constexpr reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin());
		}

		constexpr const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}

		constexpr const_iterator cbegin() const noexcept
		{
			return begin();
		}

		constexpr const_iterator cend() const noexcept
		{
			return end();
		}

		constexpr const_reverse_iterator crbegin() const noexcept
		{
			return rbegin();
		}

		constexpr const_reverse_iterator crend() const noexcept
		{
			return rend();
		}

	private:
		count_type elemCount = 0;
		alignas(T) std::byte storage[sizeof(T) * Capacity];

		static constexpr auto& commonSubscript(auto self, size_type index) noexcept
		{
			HH_ASSERT(index < self->elemCount, "Tried to access list out of range.");
			return self->data()[index];
		}

		static constexpr auto& commonAt(auto self, size_type index)
		{
			if(index < self->elemCount)
				return self->data()[index];

			throw std::out_of_range("Index into list was out of range.");
		}

		static constexpr auto commonGet(auto self, size_type index) noexcept
		{
			if(index < self->elemCount)
				return self->data() + index;

			return static_cast<decltype(self->data())>(nullptr);
		}

		template<typename S>
		static constexpr auto makeIterator(S self, auto position) noexcept
			-> std::conditional_t<std::is_const_v<std::remove_pointer_t<S>>, const_iterator, iterator>
		{
#ifdef DEBUG
			return {position, self->data(), self->data() + self->elemCount};
#else
			return {position};
#endif
		}

		static void moveLeftUnchecked(T* srcBegin, T* srcEnd, T* dstBegin) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			while(srcBegin != srcEnd)
				std::construct_at(dstBegin++, std::move(*srcBegin++));
		}

		static void moveRightUnchecked(T* srcBegin, T* srcEnd, T* dstEnd) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			while(srcBegin != srcEnd)
				std::construct_at(--dstEnd, std::move(*--srcEnd));
		}

		template<typename... Ts> constexpr pointer pushUnchecked(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			return std::construct_at(data() + elemCount++, std::forward<Ts>(ts)...);
		}

		template<typename... Ts>
		constexpr pointer emplaceUnchecked(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			T* endPtr = end().pos;
			moveRightUnchecked(pos.pos, endPtr, endPtr + 1);
			++elemCount;
			return std::construct_at(pos.pos, std::forward<Ts>(ts)...);
		}

		constexpr void popUnchecked() noexcept
		{
			std::destroy_at(data() + --elemCount);
		}

		constexpr void destruct() noexcept
		{
			std::destroy(begin(), end());
		}
	};
}

#endif /* HH_STATIC_LIST */
