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
	#define HH_ASSERT(condition, message) assert((condition) && (message))
#endif

#if !DEBUG
	#undef HH_ASSERT
	#define HH_ASSERT(condition, message)
#endif

namespace hh
{
	template<typename V> class ArrayIterator
	{
		template<typename, typename> friend class Array;
		template<typename> friend class ArrayIterator;
		template<typename> friend struct std::pointer_traits;

	public:
#ifdef __cpp_lib_concepts
		using iterator_concept = std::contiguous_iterator_tag;
#endif
		using iterator_category = std::random_access_iterator_tag;
		using value_type		= V;
		using pointer			= V*;
		using reference			= V&;
		using difference_type	= std::ptrdiff_t;

		constexpr ArrayIterator() = default;

		constexpr operator ArrayIterator<V const>() const noexcept
		{
			return
			{
				pos,
#if DEBUG
					begin, end,
#endif
			};
		}

		constexpr V& operator*() const noexcept
		{
			return *operator->();
		}

		constexpr V* operator->() const noexcept
		{
			HH_ASSERT(pos, "Tried to dereference value-initialized iterator.");
			return pos;
		}

		template<typename U> constexpr bool operator==(ArrayIterator<U> that) const noexcept
		{
			return pos == that.pos;
		}

		template<typename U> constexpr bool operator!=(ArrayIterator<U> that) const noexcept
		{
			return pos != that.pos;
		}

		template<typename U> constexpr bool operator>(ArrayIterator<U> that) const noexcept
		{
			return pos > that.pos;
		}

		template<typename U> constexpr bool operator>=(ArrayIterator<U> that) const noexcept
		{
			return pos >= that.pos;
		}

		template<typename U> constexpr bool operator<(ArrayIterator<U> that) const noexcept
		{
			return pos < that.pos;
		}

		template<typename U> constexpr bool operator<=(ArrayIterator<U> that) const noexcept
		{
			return pos <= that.pos;
		}

#ifdef __cpp_lib_three_way_comparison
		template<typename U> constexpr auto operator<=>(ArrayIterator<U> that) const noexcept
		{
			return pos <=> that.pos;
		}
#endif

		constexpr ArrayIterator& operator++() noexcept
		{
			incrementPosition(1);
			return *this;
		}

		constexpr ArrayIterator operator++(int) noexcept
		{
			auto old = *this;
			incrementPosition(1);
			return old;
		}

		constexpr ArrayIterator& operator--() noexcept
		{
			decrementPosition(1);
			return *this;
		}

		constexpr ArrayIterator operator--(int) noexcept
		{
			auto old = *this;
			decrementPosition(1);
			return old;
		}

		constexpr ArrayIterator& operator+=(difference_type offset) noexcept
		{
			incrementPosition(offset);
			return *this;
		}

		constexpr ArrayIterator operator+(difference_type offset) const noexcept
		{
			auto old = *this;
			return old += offset;
		}

		friend constexpr ArrayIterator operator+(difference_type offset, ArrayIterator iterator) noexcept
		{
			return iterator + offset;
		}

		constexpr ArrayIterator& operator-=(difference_type offset) noexcept
		{
			decrementPosition(offset);
			return *this;
		}

		constexpr ArrayIterator operator-(difference_type offset) const noexcept
		{
			auto old = *this;
			return old -= offset;
		}

		template<typename U> constexpr difference_type operator-(ArrayIterator<U> that) const noexcept
		{
			return pos - that.pos;
		}

		constexpr V& operator[](difference_type offset) const noexcept
		{
			return *(*this + offset);
		}

	private:
		V* pos = {};
#if DEBUG
		V* begin = {};
		V* end	 = {};

		constexpr ArrayIterator(V* pos, V* begin, V* end) noexcept : pos(pos), begin(begin), end(end)
		{}
#else
		constexpr ArrayIterator(V* pos) noexcept : pos(pos)
		{}
#endif

		constexpr void incrementPosition(difference_type offset) noexcept
		{
			HH_ASSERT(pos, "Cannot increment value-initialized iterator.");
			HH_ASSERT(pos < end, "Cannot increment array iterator past end.");
			pos += offset;
		}

		constexpr void decrementPosition(difference_type offset) noexcept
		{
			HH_ASSERT(pos, "Cannot decrement value-initialized iterator.");
			HH_ASSERT(begin < pos, "Cannot decrement array iterator before begin.");
			pos -= offset;
		}
	};

	template<typename T, typename Allocator = std::allocator<T>> class Array
	{
		using AllocTraits = std::allocator_traits<Allocator>;

	public:
		using value_type			 = typename AllocTraits::value_type;
		using reference				 = value_type&;
		using const_reference		 = value_type const&;
		using pointer				 = typename AllocTraits::pointer;
		using const_pointer			 = typename AllocTraits::const_pointer;
		using size_type				 = typename AllocTraits::size_type;
		using difference_type		 = typename AllocTraits::difference_type;
		using iterator				 = ArrayIterator<value_type>;
		using const_iterator		 = ArrayIterator<value_type const>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static constexpr size_type max_size() noexcept
		{
			return std::numeric_limits<size_type>::max();
		}

		constexpr Array() = default;

		constexpr Array(size_type count) : arr(allocate(count)), count(count)
		{
			if constexpr(std::is_default_constructible_v<value_type>)
			{
				Allocator alloc;

				for(auto& element : *this)
					AllocTraits::construct(alloc, &element);
			}
		}

		template<typename... Ts> constexpr Array(size_type count, Ts&&... ts) : arr(allocate(count)), count(count)
		{
			Allocator alloc;

			for(auto& element : *this)
				AllocTraits::construct(alloc, &element, ts...);
		}

		template<typename U>
		constexpr Array(size_type count, std::initializer_list<U> initializers) : arr(allocate(count)), count(count)
		{
			HH_ASSERT(count >= initializers.size(), "Size of initializer list exceeds array size.");

			Allocator alloc;

			auto element = begin();
			for(auto&& init : initializers)
				AllocTraits::construct(alloc, &*element++, std::move(init));

			if constexpr(std::is_default_constructible_v<value_type>)
				for(auto rest = begin() + static_cast<iterator::difference_type>(initializers.size()); rest != end(); ++rest)
					AllocTraits::construct(alloc, &*rest);
		}

		template<typename U, typename V>
		constexpr Array(size_type count, std::initializer_list<U> initializers, V&& fallback) : Array(count, initializers)
		{
			Allocator alloc;

			for(auto element = begin() + static_cast<iterator::difference_type>(initializers.size()); element != end();
				++element)
				AllocTraits::construct(alloc, &*element, fallback);
		}

		constexpr Array(Array const& that) : arr(allocate(that.count)), count(that.count)
		{
			Allocator alloc;

			auto other = that.begin();
			for(auto& element : *this)
				AllocTraits::construct(alloc, &element, *other++);
		}

		constexpr Array(Array&& that) noexcept : arr(that.release()), count(that.count)
		{}

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

		template<typename C> constexpr bool operator==(C const& that) const noexcept
		{
			return size() == std::size(that) && std::equal(begin(), end(), std::begin(that));
		}

		template<typename C> constexpr bool operator!=(C const& that) const noexcept
		{
			return !operator==(that);
		}

		template<typename C> constexpr bool operator<(C const& that) const noexcept
		{
			return std::lexicographical_compare(begin(), end(), std::begin(that), std::end(that));
		}

		template<typename C> constexpr bool operator>(C const& that) const noexcept
		{
			return that < *this;
		}

		template<typename C> constexpr bool operator<=(C const& that) const noexcept
		{
			return !operator>(that);
		}

		template<typename C> constexpr bool operator>=(C const& that) const noexcept
		{
			return !operator<(that);
		}

#ifdef __cpp_lib_three_way_comparison
		constexpr auto operator<=>(auto const& that) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), std::begin(that), std::end(that));
		}
#endif

		constexpr reference operator[](size_type index) noexcept
		{
			return commonSubscript(*this, index);
		}

		constexpr const_reference operator[](size_type index) const noexcept
		{
			return commonSubscript(*this, index);
		}

		constexpr reference at(size_type index)
		{
			return commonAt(*this, index);
		}

		constexpr const_reference at(size_type index) const
		{
			return commonAt(*this, index);
		}

		constexpr pointer get(size_type index) noexcept
		{
			return commonGet(*this, index);
		}

		constexpr const_pointer get(size_type index) const noexcept
		{
			return commonGet(*this, index);
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
			return (*this)[count - 1];
		}

		constexpr const_reference back() const noexcept
		{
			return (*this)[count - 1];
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
			return arr;
		}

		constexpr const_pointer data() const noexcept
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

		constexpr void reset(pointer resetValue = {}) noexcept
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

		constexpr iterator begin() noexcept
		{
#if DEBUG
			return {arr, arr, arr + count};
#else
			return {arr};
#endif
		}

		constexpr const_iterator begin() const noexcept
		{
#if DEBUG
			return {arr, arr, arr + count};
#else
			return {arr};
#endif
		}

		constexpr iterator end() noexcept
		{
#if DEBUG
			auto endPos = arr + count;
			return {endPos, arr, endPos};
#else
			return {arr + count};
#endif
		}

		constexpr const_iterator end() const noexcept
		{
#if DEBUG
			auto endPos = arr + count;
			return {endPos, arr, endPos};
#else
			return {arr + count};
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
		pointer	  arr	= {};
		size_type count = {};

		static constexpr pointer allocate(size_type count)
		{
			Allocator alloc;
			return AllocTraits::allocate(alloc, count);
		}

		template<typename U> static constexpr auto& commonSubscript(U& self, size_type index) noexcept
		{
			HH_ASSERT(index < self.count, "Tried to access list out of range.");
			return self.arr[index];
		}

		template<typename U> static constexpr auto& commonAt(U& self, size_type index)
		{
			if(index < self.count)
				return self.arr[index];

			throw std::out_of_range("Index into list was out of range.");
		}

		template<typename U> static constexpr auto commonGet(U& self, size_type index) noexcept
		{
			if(index < self.count)
				return self.arr + index;

			return static_cast<decltype(self.arr)>(nullptr);
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

namespace std
{
	template<typename T> struct pointer_traits<hh::ArrayIterator<T>>
	{
		using pointer		  = hh::ArrayIterator<T>;
		using element_type	  = typename pointer::value_type;
		using difference_type = typename pointer::difference_type;

		[[nodiscard]] static constexpr element_type* to_address(pointer iter) noexcept
		{
			HH_ASSERT(iter.begin <= iter.pos && iter.pos <= iter.end, "Iterator is not within a validly addressable range.");
			return std::to_address(iter.pos);
		}
	};
}

#endif /* HH_ARRAY */
