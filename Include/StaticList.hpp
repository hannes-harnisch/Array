#ifndef HH_STATIC_LIST
#define HH_STATIC_LIST

#include <algorithm>
#include <iterator>
#include <limits>
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
	template<typename T, size_t Cap> class StaticList
	{
		using count_type = std::conditional_t<
			Cap <= std::numeric_limits<uint8_t>::max() && alignof(T) <= alignof(uint8_t),
			uint8_t,
			std::conditional_t<
				Cap <= std::numeric_limits<uint16_t>::max() && alignof(T) <= alignof(uint16_t),
				uint16_t,
				std::conditional_t<Cap <= std::numeric_limits<uint32_t>::max() && alignof(T) <= alignof(uint32_t),
								   uint32_t,
								   uint64_t>>>;

	public:
		using value_type	  = T;
		using reference		  = T&;
		using const_reference = T const&;
		using pointer		  = T*;
		using const_pointer	  = T const*;
		using size_type		  = std::size_t;
		using difference_type = std::ptrdiff_t;

		static constexpr size_type capacity() noexcept
		{
			return Cap;
		}

		static constexpr size_type max_size() noexcept
		{
			return Cap;
		}

		StaticList() = default;

		StaticList(StaticList const&) requires std::is_trivially_copy_constructible_v<T> = default;

		StaticList(StaticList const& that) noexcept(std::is_nothrow_copy_constructible_v<T>) : count(that.count)
		{
			std::copy_n(that.begin(), count, begin());
		}

		StaticList(StaticList&&) requires std::is_trivially_move_constructible_v<T> = default;

		StaticList(StaticList&& that) noexcept(std::is_nothrow_move_constructible_v<T>) : count(that.count)
		{}

		~StaticList() requires std::is_trivially_destructible_v<T> = default;

		~StaticList()
		{
			destruct();
		}

		StaticList& operator=(StaticList const&) requires std::is_trivially_copy_assignable_v<T> = default;

		StaticList& operator=(StaticList&&) requires std::is_trivially_move_assignable_v<T> = default;

		constexpr auto operator<=>(auto const& that) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), std::begin(that), std::end(that));
		}

		constexpr reference operator[](size_type index) noexcept
		{
			HH_ASSERT(index < count, "Index into list was out of range.");
			return data()[index];
		}

		constexpr const_reference operator[](size_type index) const noexcept
		{
			HH_ASSERT(index < count, "Index into list was out of range.");
			return data()[index];
		}

		constexpr reference at(size_type index)
		{
			if(index < count)
				return data()[index];

			throw std::out_of_range("Index into list was out of range.");
		}

		constexpr const_reference at(size_type index) const
		{
			if(index < count)
				return data()[index];

			throw std::out_of_range("Index into list was out of range.");
		}

		constexpr pointer get(size_type index) noexcept
		{
			if(index < count)
				return data() + index;

			return nullptr;
		}

		constexpr const_pointer get(size_type index) const noexcept
		{
			if(index < count)
				return data() + index;

			return nullptr;
		}

		constexpr reference front() noexcept
		{
			HH_ASSERT(count, "Tried to access empty list.");
			return data()[0];
		}

		constexpr const_reference front() const noexcept
		{
			HH_ASSERT(count, "Tried to access empty list.");
			return data()[0];
		}

		constexpr reference back() noexcept
		{
			HH_ASSERT(count, "Tried to access empty list.");
			return data()[count - 1];
		}

		constexpr const_reference back() const noexcept
		{
			HH_ASSERT(count, "Tried to access empty list.");
			return data()[count - 1];
		}

		template<typename... Ts>
		constexpr reference emplace_back(Ts&&... ts) noexcept(std::is_nothrow_constructible_v<T, Ts...>)
		{
			HH_ASSERT(count < Cap, "List is out of space.");
			return *new(data() + count++) T(std::forward<Ts>(ts)...);
		}

		constexpr void push_back(T const& element) noexcept(std::is_nothrow_copy_constructible_v<T>)
		{
			emplace_back(element);
		}

		constexpr void push_back(T&& element) noexcept(std::is_nothrow_move_constructible_v<T>)
		{
			emplace_back(std::move(element));
		}

		constexpr void clear() noexcept
		{
			destruct();
			count = 0;
		}

		constexpr void swap(StaticList& that) noexcept(std::is_nothrow_invocable_v<std::swap, StaticList&, StaticList&>)
		{
			std::swap(*this, that);
		}

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return !count;
		}

		constexpr size_type size() const noexcept
		{
			return count;
		}

		constexpr pointer data() noexcept
		{
			return reinterpret_cast<T*>(storage);
		}

		constexpr const_pointer data() const noexcept
		{
			return reinterpret_cast<T const*>(storage);
		}

	private:
		template<typename V> class Iterator
		{
			friend StaticList;

		public:
#ifdef __cpp_lib_concepts
			using iterator_concept = std::contiguous_iterator_tag;
#else
			using iterator_category = std::random_access_iterator_tag;
#endif
			using value_type	  = V;
			using pointer		  = V*;
			using reference		  = V&;
			using difference_type = std::ptrdiff_t;

			constexpr Iterator() = default;

			constexpr V& operator*() const noexcept
			{
				return *pos;
			}

			constexpr V* operator->() const noexcept
			{
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

			template<typename U> constexpr bool operator>(Iterator<U> that) const noexcept
			{
				return pos > that.pos;
			}

			template<typename U> constexpr bool operator>=(Iterator<U> that) const noexcept
			{
				return pos >= that.pos;
			}

			template<typename U> constexpr bool operator<(Iterator<U> that) const noexcept
			{
				return pos < that.pos;
			}

			template<typename U> constexpr bool operator<=(Iterator<U> that) const noexcept
			{
				return pos <= that.pos;
			}

#ifdef __cpp_lib_three_way_comparison
			template<typename U> constexpr auto operator<=>(Iterator<U> that) const noexcept
			{
				return pos <=> that.pos;
			}
#endif

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

			constexpr V& operator[](difference_type index) const noexcept
			{
				return *(*this + index);
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
				HH_ASSERT(pos < end, "Cannot increment list iterator past end.");
				pos += offset;
			}

			constexpr void decrementPosition(difference_type offset) noexcept
			{
				HH_ASSERT(begin < pos, "Cannot decrement list iterator before begin.");
				pos -= offset;
			}
		};

	public:
		using iterator				 = Iterator<value_type>;
		using const_iterator		 = Iterator<value_type const>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		constexpr iterator begin() noexcept
		{
#ifdef DEBUG
			return {data(), data(), data() + count};
#else
			return {data()};
#endif
		}

		constexpr const_iterator begin() const noexcept
		{
#ifdef DEBUG
			return {data(), data(), data() + count};
#else
			return {data()};
#endif
		}

		constexpr iterator end() noexcept
		{
#ifdef DEBUG
			auto endPos = data() + count;
			return {endPos, data(), endPos};
#else
			return {data() + count};
#endif
		}

		constexpr const_iterator end() const noexcept
		{
#ifdef DEBUG
			auto endPos = data() + count;
			return {endPos, data(), endPos};
#else
			return {data() + count};
#endif
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
		alignas(T) char storage[sizeof(T) * Cap];
		count_type count = 0;

		constexpr void destruct() noexcept
		{
			if constexpr(!std::is_trivially_destructible_v<T>)
				for(T& element : *this)
					element.~T();
		}
	};
}

#endif /* HH_STATIC_LIST */
