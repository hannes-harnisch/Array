#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace hh
{
	template<typename T, typename Alloc = std::allocator<T>> class Array
	{
		using AllocTraits = std::allocator_traits<Alloc>;

	public:
		using value_type	  = typename AllocTraits::value_type;
		using reference		  = value_type&;
		using const_reference = const value_type&;
		using pointer		  = typename AllocTraits::pointer;
		using const_pointer	  = typename AllocTraits::const_pointer;
		using size_type		  = typename AllocTraits::size_type;
		using difference_type = typename AllocTraits::difference_type;

		constexpr Array() noexcept = default;

		constexpr Array(value_type arr[], size_type count) noexcept : arr(arr), count(count)
		{}

		constexpr Array(size_type count) : Array(allocate(count), count)
		{
			if constexpr(std::is_default_constructible_v<value_type>)
			{
				Alloc alloc;
				for(auto& element : *this)
					AllocTraits::construct(alloc, &element);
			}
		}

		template<typename... Ts> constexpr Array(size_type count, Ts&&... ts) : Array(allocate(count), count)
		{
			Alloc alloc;
			for(auto& element : *this)
				AllocTraits::construct(alloc, &element, std::forward<Ts>(ts)...);
		}

		template<typename U>
		constexpr Array(size_type count, std::initializer_list<U> initializers) : Array(allocate(count), count)
		{
			assert(("Size of initializer list exceeds array size.", count >= initializers.size()));

			Alloc alloc;
			auto element = begin();
			for(auto&& init : initializers)
				AllocTraits::construct(alloc, &*element++, init);

			if constexpr(std::is_default_constructible_v<value_type>)
				for(auto rest = begin() + initializers.size(); rest != end(); ++rest)
					AllocTraits::construct(alloc, &*rest);
		}

		template<typename U, typename V>
		constexpr Array(size_type count, std::initializer_list<U> initializers, V&& fallback) : Array(count, initializers)
		{
			Alloc alloc;
			for(auto element = begin() + initializers.size(); element != end(); ++element)
				AllocTraits::construct(alloc, &*element, std::forward<V>(fallback));
		}

		constexpr Array(const Array& that) : Array(allocate(that.count), that.count)
		{
			Alloc alloc;
			auto other = that.begin();
			for(auto& element : *this)
				AllocTraits::construct(alloc, &element, *other++);
		}

		constexpr Array(Array&& that) noexcept : Array()
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

		[[nodiscard]] constexpr reference operator[](size_type index) noexcept
		{
			assert(("Index into array was out of range.", index < count));
			return arr[index];
		}

		[[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept
		{
			assert(("Index into array was out of range.", index < count));
			return arr[index];
		}

		[[nodiscard]] constexpr bool operator==(const Array& that) const noexcept
		{
			return count == that.count && std::equal(begin(), end(), that.begin());
		}

		[[nodiscard]] constexpr bool operator!=(const Array& that) const noexcept
		{
			return !operator==(that);
		}

		[[nodiscard]] constexpr bool operator<(const Array& that) const noexcept
		{
			return std::lexicographical_compare(begin(), end(), that.begin(), that.end());
		}

		[[nodiscard]] constexpr bool operator>(const Array& that) const noexcept
		{
			return that < *this;
		}

		[[nodiscard]] constexpr bool operator<=(const Array& that) const noexcept
		{
			return !operator>(that);
		}

		[[nodiscard]] constexpr bool operator>=(const Array& that) const noexcept
		{
			return !operator<(that);
		}

#ifdef __cpp_lib_three_way_comparison
		[[nodiscard]] constexpr auto operator<=>(const Array& that) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), that.begin(), that.end());
		}
#endif

		[[nodiscard]] constexpr reference at(size_type index)
		{
			if(index < count)
				return arr[index];
			else
				throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] constexpr const_reference at(size_type index) const
		{
			if(index < count)
				return arr[index];
			else
				throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] constexpr pointer get(size_type index) noexcept
		{
			if(index < count)
				return arr + index;
			else
				return nullptr;
		}

		[[nodiscard]] constexpr const_pointer get(size_type index) const noexcept
		{
			if(index < count)
				return arr + index;
			else
				return nullptr;
		}

		[[nodiscard]] constexpr reference front() noexcept
		{
			return arr[0];
		}

		[[nodiscard]] constexpr const_reference front() const noexcept
		{
			return arr[0];
		}

		[[nodiscard]] constexpr reference back() noexcept
		{
			return arr[count - 1];
		}

		[[nodiscard]] constexpr const_reference back() const noexcept
		{
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
			std::fill(begin(), end(), std::forward<U>(value));
		}

		[[nodiscard]] constexpr pointer release() noexcept
		{
			return std::exchange(arr, pointer());
		}

		constexpr void reset(pointer resetValue = pointer()) noexcept
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
		template<typename V> class Iterator
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

			template<typename U> [[nodiscard]] constexpr bool operator==(Iterator<U> that) const noexcept
			{
				return pos == that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator!=(Iterator<U> that) const noexcept
			{
				return pos != that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator>(Iterator<U> that) const noexcept
			{
				return pos > that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator>=(Iterator<U> that) const noexcept
			{
				return pos >= that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator<(Iterator<U> that) const noexcept
			{
				return pos < that.pos;
			}

			template<typename U> [[nodiscard]] constexpr bool operator<=(Iterator<U> that) const noexcept
			{
				return pos <= that.pos;
			}

#ifdef __cpp_lib_three_way_comparison
			template<typename U> [[nodiscard]] constexpr auto operator<=>(Iterator<U> that) const noexcept
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

			[[nodiscard]] constexpr Iterator operator+(difference_type offset) const noexcept
			{
				auto old = *this;
				return old += offset;
			}

			[[nodiscard]] friend constexpr Iterator operator+(difference_type offset, Iterator iterator) noexcept
			{
				return iterator + offset;
			}

			constexpr Iterator& operator-=(difference_type offset) noexcept
			{
				decrementPosition(offset);
				return *this;
			}

			[[nodiscard]] constexpr Iterator operator-(difference_type offset) const noexcept
			{
				auto old = *this;
				return old -= offset;
			}

			template<typename U> [[nodiscard]] constexpr difference_type operator-(Iterator<U> that) const noexcept
			{
				return pos - that.pos;
			}

			[[nodiscard]] constexpr V& operator[](size_type index) const noexcept
			{
				return *(*this + index);
			}

		private:
			V* pos {};
#ifndef NDEBUG
			V* begin {};
			V* end {};

			constexpr Iterator(V* pos, V* begin, V* end) noexcept : pos(pos), begin(begin), end(end)
			{}
#else
			constexpr Iterator(V* pos) noexcept : pos(pos)
			{}
#endif

			constexpr void incrementPosition(difference_type offset) noexcept
			{
				assert(("Cannot increment iterator past end.", pos < end));
				pos += offset;
			}

			constexpr void decrementPosition(difference_type offset) noexcept
			{
				assert(("Cannot decrement iterator before begin.", begin < pos));
				pos -= offset;
			}
		};

	public:
		using iterator				 = Iterator<value_type>;
		using const_iterator		 = Iterator<const value_type>;
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

		static constexpr pointer allocate(size_type count)
		{
			Alloc alloc;
			return AllocTraits::allocate(alloc, count);
		}

		constexpr void destruct() noexcept
		{
			Alloc alloc;

			if constexpr(!std::is_trivially_destructible_v<value_type>)
				for(auto& element : *this)
					AllocTraits::destroy(alloc, &element);

			AllocTraits::deallocate(alloc, arr, count);
		}
	};
}
