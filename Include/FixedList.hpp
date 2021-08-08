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
			incrementPosition(1);
			return *this;
		}

		FixedListIterator operator++(int) noexcept
		{
			auto old = *this;
			incrementPosition(1);
			return old;
		}

		FixedListIterator& operator--() noexcept
		{
			decrementPosition(1);
			return *this;
		}

		FixedListIterator operator--(int) noexcept
		{
			auto old = *this;
			decrementPosition(1);
			return old;
		}

		FixedListIterator& operator+=(difference_type offset) noexcept
		{
			incrementPosition(offset);
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
			decrementPosition(offset);
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

		void incrementPosition(difference_type offset) noexcept
		{
			HH_ASSERT(pos, "Cannot increment value-initialized iterator.");
			HH_ASSERT(pos < end, "Cannot increment list iterator past end.");
			pos += offset;
		}

		void decrementPosition(difference_type offset) noexcept
		{
			HH_ASSERT(pos, "Cannot decrement value-initialized iterator.");
			HH_ASSERT(begin < pos, "Cannot decrement list iterator before begin.");
			pos -= offset;
		}
	};

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

		FixedList() = default;

		FixedList(size_type count) noexcept(std::is_nothrow_default_constructible_v<T>) :
			elemCount(static_cast<count_type>(count))
		{
			HH_ASSERT(count <= Capacity, "Requested size exceeded capacity.");
			std::uninitialized_default_construct(begin(), end());
		}

		FixedList(size_type count, auto const& value) noexcept(std::is_nothrow_constructible_v<T, decltype(value)>) :
			elemCount(static_cast<count_type>(count))
		{
			HH_ASSERT(count <= Capacity, "Requested size exceeded capacity.");
			std::uninitialized_fill(begin(), end(), value);
		}

		template<typename U> FixedList(std::initializer_list<U> init) : elemCount(static_cast<count_type>(init.size()))
		{
			HH_ASSERT(init.size() <= Capacity, "Size of initializer list exceeded capacity.");
			std::uninitialized_copy(init.begin(), init.end(), begin());
		}

		FixedList(FixedList const&) requires std::is_trivially_copy_constructible_v<T> = default;

		FixedList(FixedList const& that) noexcept(std::is_nothrow_copy_constructible_v<T>) : elemCount(that.elemCount)
		{
			std::uninitialized_copy(that.begin(), that.end(), begin());
		}

		FixedList(FixedList&&) requires std::is_trivially_move_constructible_v<T> = default;

		FixedList(FixedList&& that) noexcept(std::is_nothrow_move_constructible_v<T>) : elemCount(that.elemCount)
		{
			std::uninitialized_move(that.begin(), that.end(), begin());
		}

		~FixedList() requires std::is_trivially_destructible_v<T> = default;

		~FixedList()
		{
			destruct();
		}

		FixedList& operator=(FixedList const&) requires std::is_trivially_copy_assignable_v<T> = default;

		FixedList& operator=(FixedList const& that) noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			assign(that.begin(), that.end());
			return *this;
		}

		FixedList& operator=(FixedList&&) requires std::is_trivially_move_assignable_v<T> = default;

		FixedList& operator=(FixedList&& that) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			destruct();
			std::uninitialized_move(that.begin(), that.end(), begin());
			elemCount = that.elemCount;
			return *this;
		}

		bool operator==(auto const& that) const noexcept
		{
			return size() == std::size(that) && std::equal(begin(), end(), std::begin(that));
		}

		bool operator!=(auto const& that) const noexcept
		{
			return !operator==(that);
		}

		bool operator<(auto const& that) const noexcept
		{
			return std::lexicographical_compare(begin(), end(), std::begin(that), std::end(that));
		}

		bool operator<=(auto const& that) const noexcept
		{
			return !operator>(that);
		}

		bool operator>(auto const& that) const noexcept
		{
			return that < *this;
		}

		bool operator>=(auto const& that) const noexcept
		{
			return !operator<(that);
		}

		auto operator<=>(auto const& that) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), std::begin(that), std::end(that));
		}

		reference operator[](size_type index) noexcept
		{
			return commonSubscript(this, index);
		}

		const_reference operator[](size_type index) const noexcept
		{
			return commonSubscript(this, index);
		}

		reference at(size_type index)
		{
			return commonAt(this, index);
		}

		const_reference at(size_type index) const
		{
			return commonAt(this, index);
		}

		pointer get(size_type index) noexcept
		{
			return commonGet(this, index);
		}

		const_pointer get(size_type index) const noexcept
		{
			return commonGet(this, index);
		}

		reference front() noexcept
		{
			return (*this)[0];
		}

		const_reference front() const noexcept
		{
			return (*this)[0];
		}

		reference back() noexcept
		{
			return (*this)[elemCount - 1];
		}

		const_reference back() const noexcept
		{
			return (*this)[elemCount - 1];
		}

		void assign(size_type count, auto const& value) noexcept(std::is_nothrow_constructible_v<T, decltype(value)>)
		{
			HH_ASSERT(count <= Capacity, "Requested size exceeded capacity.");

			clear();
			while(count--)
				pushUnchecked(value);
		}

		template<typename TInputIt>
		void assign(TInputIt first,
					TInputIt last) noexcept(std::is_nothrow_constructible_v<T, std::iterator_traits<TInputIt>::reference>)
		{
			clear();
			while(first != last)
				emplace_back(*first++);
		}

		template<typename U> void assign(std::initializer_list<U> init) noexcept(std::is_nothrow_constructible_v<T, U>)
		{
			assign(init.begin(), init.end());
		}

		iterator insert(iterator pos, T const& value) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_copy_constructible_v<T>)
		{
			return emplace(pos, value);
		}

		iterator insert(iterator pos, T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			return emplace(pos, std::move(value));
		}

		iterator insert(iterator pos, size_type count, auto const& value) noexcept(
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
		iterator insert(iterator pos, TInputIt first, TInputIt last) noexcept(
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
		iterator insert(iterator pos, std::initializer_list<U> init) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, U>)
		{
			return insert(pos, init.begin(), init.end());
		}

		template<typename... Ts>
		iterator emplace(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			HH_ASSERT(elemCount < Capacity, "List is out of capacity.");
			return makeIterator(this, emplaceUnchecked(pos, std::forward<Ts>(ts)...));
		}

		template<typename... Ts>
		iterator try_emplace(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			if(elemCount < Capacity)
				return makeIterator(this, emplaceUnchecked(pos, std::forward<Ts>(ts)...));

			return end();
		}

		template<typename... Ts> reference emplace_back(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			HH_ASSERT(elemCount < Capacity, "List is out of capacity.");
			return *pushUnchecked(std::forward<Ts>(ts)...);
		}

		template<typename... Ts> iterator try_emplace_back(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			if(elemCount < Capacity)
				return makeIterator(this, pushUnchecked(std::forward<Ts>(ts)...));

			return end();
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
			HH_ASSERT(elemCount, "Cannot pop elements in an empty list.");
			popUnchecked();
		}

		bool try_pop_back() noexcept
		{
			if(empty())
				return false;

			popUnchecked();
			return true;
		}

		iterator erase(iterator pos) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			HH_ASSERT(pos >= begin() && pos < end(), "Cannot erase with an invalid iterator.");

			std::destroy_at(pos.pos);
			moveLeftUnchecked(pos.pos, end().pos, pos.pos);
			--elemCount;
			return pos;
		}

		iterator erase(iterator first, iterator last) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			HH_ASSERT(first <= last && first >= begin() && last <= end(), "Cannot erase an invalid range.");

			std::destroy(first, last);
			moveLeftUnchecked(last.pos, end().pos, first.pos);
			elemCount -= static_cast<count_type>(last - first);
			return first;
		}

		void clear() noexcept
		{
			destruct();
			elemCount = 0;
		}

		void swap(FixedList& that) noexcept(std::is_nothrow_swappable_v<T>)
		{
			std::swap(*this, that);
		}

		[[nodiscard]] bool empty() const noexcept
		{
			return !elemCount;
		}

		size_type size() const noexcept
		{
			return elemCount;
		}

		count_type count() const noexcept
		{
			return elemCount;
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
			return makeIterator(this, data());
		}

		const_iterator begin() const noexcept
		{
			return makeIterator(this, data());
		}

		iterator end() noexcept
		{
			return makeIterator(this, data() + elemCount);
		}

		const_iterator end() const noexcept
		{
			return makeIterator(this, data() + elemCount);
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
		count_type elemCount = 0;
		alignas(T) std::byte storage[sizeof(T) * Capacity];

		static auto& commonSubscript(auto self, size_type index) noexcept
		{
			HH_ASSERT(index < self->elemCount, "Tried to access list out of range.");
			return self->data()[index];
		}

		static auto& commonAt(auto self, size_type index)
		{
			if(index < self->elemCount)
				return self->data()[index];

			throw std::out_of_range("Index into list was out of range.");
		}

		static auto commonGet(auto self, size_type index) noexcept
		{
			if(index < self->elemCount)
				return self->data() + index;

			return static_cast<decltype(self->data())>(nullptr);
		}

		template<typename S>
		static auto makeIterator(S self, auto position) noexcept
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

		template<typename... Ts> pointer pushUnchecked(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			return std::construct_at(data() + elemCount++, std::forward<Ts>(ts)...);
		}

		template<typename... Ts>
		pointer emplaceUnchecked(iterator pos, Ts&&... ts) noexcept(
			std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_constructible_v<T, Ts...>)
		{
			T* endPtr = end().pos;
			moveRightUnchecked(pos.pos, endPtr, endPtr + 1);
			++elemCount;
			return std::construct_at(pos.pos, std::forward<Ts>(ts)...);
		}

		void popUnchecked() noexcept
		{
			std::destroy_at(data() + --elemCount);
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
