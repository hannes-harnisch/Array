#ifndef HH_ARRAY
#define HH_ARRAY

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifndef HH_ASSERT
	#include <cassert>
	#define HH_ASSERT(condition, message) assert((message, condition))
#endif

namespace hh
{
	template<typename T, typename Allocator = std::allocator<T>> class [[nodiscard]] Array
	{
		using AllocTraits = std::allocator_traits<Allocator>;

	public:
		using value_type	  = typename AllocTraits::value_type;
		using reference		  = value_type&;
		using const_reference = value_type const&;
		using pointer		  = typename AllocTraits::pointer;
		using const_pointer	  = typename AllocTraits::const_pointer;
		using size_type		  = typename AllocTraits::size_type;
		using difference_type = typename AllocTraits::difference_type;

		constexpr Array() noexcept = default;

		constexpr Array(size_type const count) : arr(allocate(count)), count(count)
		{
			if constexpr(std::is_default_constructible_v<value_type>)
			{
				Allocator alloc;
				for(auto& element : *this)
					AllocTraits::construct(alloc, &element);
			}
		}

		template<typename... Ts> constexpr Array(size_type const count, Ts&&... ts) : arr(allocate(count)), count(count)
		{
			Allocator alloc;
			for(auto& element : *this)
				AllocTraits::construct(alloc, &element, ts...);
		}

		template<typename U>
		constexpr Array(size_type const count, std::initializer_list<U> initializers) : arr(allocate(count)), count(count)
		{
			HH_ASSERT(count >= initializers.size(), "Size of initializer list exceeds array size.");

			Allocator alloc;
			auto element = begin();
			for(auto&& init : initializers)
				AllocTraits::construct(alloc, &*element++, std::move(init));

			if constexpr(std::is_default_constructible_v<value_type>)
				for(auto rest = begin() + initializers.size(); rest != end(); ++rest)
					AllocTraits::construct(alloc, &*rest);
		}

		template<typename U, typename V>
		constexpr Array(size_type const count, std::initializer_list<U> initializers, V&& fallback) : Array(count, initializers)
		{
			Allocator alloc;
			for(auto element = begin() + initializers.size(); element != end(); ++element)
				AllocTraits::construct(alloc, &*element, fallback);
		}

		constexpr Array(Array const& that) : arr(allocate(that.count)), count(that.count)
		{
			Allocator alloc;
			auto other = that.begin();
			for(auto& element : *this)
				AllocTraits::construct(alloc, &element, *other++);
		}

		constexpr Array(Array&& that) noexcept
		{
			swap(that);
		}

#ifdef __cpp_constexpr_dynamic_alloc
		constexpr
#endif
			~Array()
		{
			destruct();
		}

		constexpr Array& operator=(Array that) noexcept
		{
			swap(that);
			return *this;
		}

		[[nodiscard]] constexpr reference operator[](size_type const index) noexcept
		{
			HH_ASSERT(index < count, "Index into array was out of range.");
			return arr[index];
		}

		[[nodiscard]] constexpr const_reference operator[](size_type const index) const noexcept
		{
			HH_ASSERT(index < count, "Index into array was out of range.");
			return arr[index];
		}

		template<typename C> [[nodiscard]] constexpr bool operator==(C const& that) const noexcept
		{
			return size() == std::size(that) && std::equal(begin(), end(), std::begin(that));
		}

		template<typename C> [[nodiscard]] constexpr bool operator!=(C const& that) const noexcept
		{
			return !operator==(that);
		}

		template<typename C> [[nodiscard]] constexpr bool operator<(C const& that) const noexcept
		{
			return std::lexicographical_compare(begin(), end(), std::begin(that), std::end(that));
		}

		template<typename C> [[nodiscard]] constexpr bool operator>(C const& that) const noexcept
		{
			return that < *this;
		}

		template<typename C> [[nodiscard]] constexpr bool operator<=(C const& that) const noexcept
		{
			return !operator>(that);
		}

		template<typename C> [[nodiscard]] constexpr bool operator>=(C const& that) const noexcept
		{
			return !operator<(that);
		}

#ifdef __cpp_lib_three_way_comparison
		[[nodiscard]] constexpr auto operator<=>(auto const& that) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), std::begin(that), std::end(that));
		}
#endif

		[[nodiscard]] constexpr reference at(size_type const index)
		{
			if(index < count)
				return arr[index];

			throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] constexpr const_reference at(size_type const index) const
		{
			if(index < count)
				return arr[index];

			throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] constexpr pointer get(size_type const index) noexcept
		{
			if(index < count)
				return arr + index;

			return {};
		}

		[[nodiscard]] constexpr const_pointer get(size_type const index) const noexcept
		{
			if(index < count)
				return arr + index;

			return {};
		}

		[[nodiscard]] constexpr reference front() noexcept
		{
			HH_ASSERT(count, "Tried to access element of empty array.");
			return arr[0];
		}

		[[nodiscard]] constexpr const_reference front() const noexcept
		{
			HH_ASSERT(count, "Tried to access element of empty array.");
			return arr[0];
		}

		[[nodiscard]] constexpr reference back() noexcept
		{
			HH_ASSERT(count, "Tried to access element of empty array.");
			return arr[count - 1];
		}

		[[nodiscard]] constexpr const_reference back() const noexcept
		{
			HH_ASSERT(count, "Tried to access element of empty array.");
			return arr[count - 1];
		}

		[[nodiscard]] constexpr bool empty() const noexcept
		{
			return !count;
		}

		[[nodiscard]] constexpr size_type size() const noexcept
		{
			return count;
		}

		[[nodiscard]] static constexpr size_type max_size() noexcept
		{
			return std::numeric_limits<size_type>::max();
		}

		[[nodiscard]] constexpr pointer data() noexcept
		{
			return arr;
		}

		[[nodiscard]] constexpr const_pointer data() const noexcept
		{
			return arr;
		}

		template<typename U> constexpr void fill(U&& value) noexcept
		{
			std::fill(begin(), end(), value);
		}

		[[nodiscard]] constexpr pointer release() noexcept
		{
			return std::exchange(arr, {});
		}

		constexpr void reset(pointer const resetValue = {}) noexcept
		{
			destruct();
			arr = resetValue;
		}

		constexpr void swap(Array& that) noexcept
		{
			std::swap(count, that.count);
			std::swap(arr, that.arr);
		}

		friend constexpr void swap(Array& left, Array& right) noexcept
		{
			left.swap(right);
		}

	private:
		template<typename V> class [[nodiscard]] Iterator
		{
			friend Array;

		public:
#ifdef __cpp_lib_concepts
			using iterator_concept = std::contiguous_iterator_tag;
#else
			using iterator_category = std::random_access_iterator_tag;
#endif
			using value_type	  = V;
			using pointer		  = V*;
			using reference		  = V&;
			using difference_type = typename AllocTraits::difference_type;

			constexpr Iterator() noexcept = default;

			[[nodiscard]] constexpr V& operator*() const noexcept
			{
				return *pos;
			}

			[[nodiscard]] constexpr V* operator->() const noexcept
			{
				return pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator==(Iterator<U> const that) const noexcept
			{
				return pos == that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator!=(Iterator<U> const that) const noexcept
			{
				return pos != that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator>(Iterator<U> const that) const noexcept
			{
				return pos > that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator>=(Iterator<U> const that) const noexcept
			{
				return pos >= that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator<(Iterator<U> const that) const noexcept
			{
				return pos < that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator<=(Iterator<U> const that) const noexcept
			{
				return pos <= that.pos;
			}

#ifdef __cpp_lib_three_way_comparison
			template<typename U> [[nodiscard]] constexpr auto operator<=>(Iterator<U> const that) const noexcept
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

			constexpr Iterator& operator+=(difference_type const offset) noexcept
			{
				incrementPosition(offset);
				return *this;
			}

			[[nodiscard]] constexpr Iterator operator+(difference_type const offset) const noexcept
			{
				auto old = *this;
				return old += offset;
			}

			[[nodiscard]] friend constexpr Iterator operator+(difference_type const offset, Iterator const iterator) noexcept
			{
				return iterator + offset;
			}

			constexpr Iterator& operator-=(difference_type const offset) noexcept
			{
				decrementPosition(offset);
				return *this;
			}

			[[nodiscard]] constexpr Iterator operator-(difference_type const offset) const noexcept
			{
				auto old = *this;
				return old -= offset;
			}

			template<typename U> [[nodiscard]] constexpr difference_type operator-(Iterator<U> const that) const noexcept
			{
				return pos - that.pos;
			}

			[[nodiscard]] constexpr V& operator[](size_type const index) const noexcept
			{
				return *(*this + index);
			}

		private:
			V* pos {};
#ifndef NDEBUG
			V* begin {};
			V* end {};

			constexpr Iterator(V* const pos, V* const begin, V* const end) noexcept : pos(pos), begin(begin), end(end)
			{}
#else
			constexpr Iterator(V* const pos) noexcept : pos(pos)
			{}
#endif

			constexpr void incrementPosition(difference_type const offset) noexcept
			{
				HH_ASSERT(pos < end, "Cannot increment array iterator past end.");
				pos += offset;
			}

			constexpr void decrementPosition(difference_type const offset) noexcept
			{
				HH_ASSERT(begin < pos, "Cannot decrement array iterator before begin.");
				pos -= offset;
			}
		};

	public:
		using iterator				 = Iterator<value_type>;
		using const_iterator		 = Iterator<value_type const>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		[[nodiscard]] constexpr iterator begin() noexcept
		{
#ifndef NDEBUG
			return {arr, arr, arr + count};
#else
			return {arr};
#endif
		}

		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
#ifndef NDEBUG
			return {arr, arr, arr + count};
#else
			return {arr};
#endif
		}

		[[nodiscard]] constexpr iterator end() noexcept
		{
#ifndef NDEBUG
			pointer endPos = arr + count;
			return {endPos, arr, endPos};
#else
			return {arr + count};
#endif
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
#ifndef NDEBUG
			pointer endPos = arr + count;
			return {endPos, arr, endPos};
#else
			return {arr + count};
#endif
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(end());
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		[[nodiscard]] constexpr reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin());
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept
		{
			return begin();
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept
		{
			return end();
		}

		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
		{
			return rbegin();
		}

		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
		{
			return rend();
		}

	private:
		pointer arr {};
		size_type count {};

		static constexpr pointer allocate(size_type const count)
		{
			Allocator alloc;
			return AllocTraits::allocate(alloc, count);
		}

		constexpr void destruct() noexcept
		{
			Allocator alloc;

			if constexpr(!std::is_trivially_destructible_v<value_type>)
				for(auto& element : *this)
					AllocTraits::destroy(alloc, &element);

			AllocTraits::deallocate(alloc, arr, count);
		}
	};
}

#endif /* HH_ARRAY */
