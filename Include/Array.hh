#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace ARRAY_NAMESPACE
{
	template<typename T> class Array
	{
	public:
		using value_type	  = T;
		using reference		  = T&;
		using const_reference = const T&;
		using pointer		  = T*;
		using const_pointer	  = const T*;
		using difference_type = std::ptrdiff_t;
		using size_type		  = std::size_t;

		Array() noexcept = default;

		Array(value_type data[], size_type size) noexcept : count {size}, array {data}
		{}

		Array(size_type size) : Array {new value_type[size], size}
		{}

		template<typename U> Array(size_type size, U&& initialValue) : Array {size}
		{
			fill(std::forward<U>(initialValue));
		}

		template<typename U> Array(size_type size, std::initializer_list<U> initialValues) : Array {size}
		{
			assert(("Size of initializer list exceeds array size.", count >= initialValues.size()));
			std::copy_n(initialValues.begin(), initialValues.size(), array);
		}

		Array(const Array& other) : Array {other.count}
		{
			std::copy_n(other.array, count, array);
		}

		Array(Array&& other) noexcept : Array {}
		{
			swap(other);
		}

		~Array()
		{
			delete[] array;
		}

		Array& operator=(Array other) noexcept
		{
			swap(other);
			return *this;
		}

		[[nodiscard]] reference operator[](size_type index) noexcept
		{
			assert(("Index into array was out of range.", index < count));
			return array[index];
		}

		[[nodiscard]] const_reference operator[](size_type index) const noexcept
		{
			assert(("Index into array was out of range.", index < count));
			return array[index];
		}

		[[nodiscard]] bool operator==(const Array& other) const noexcept
		{
			if(count != other.count)
				return false;
			// std::equal not used here because of faulty MSVC implementation
			auto otherElement {other.begin()};
			for(auto&& element : *this)
				if(element != *otherElement++)
					return false;
			return true;
		}

		[[nodiscard]] bool operator!=(const Array& other) const noexcept
		{
			return !operator==(other);
		}

		[[nodiscard]] bool operator<(const Array& other) const noexcept
		{
			return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
		}

		[[nodiscard]] bool operator>(const Array& other) const noexcept
		{
			return other < *this;
		}

		[[nodiscard]] bool operator<=(const Array& other) const noexcept
		{
			return !operator>(other);
		}

		[[nodiscard]] bool operator>=(const Array& other) const noexcept
		{
			return !operator<(other);
		}

		[[nodiscard]] auto operator<=>(const Array& other) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end());
		}

		[[nodiscard]] reference at(size_type index)
		{
			if(index < count)
				return array[index];
			throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] const_reference at(size_type index) const
		{
			if(index < count)
				return array[index];
			throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] reference front() noexcept
		{
			return array[0];
		}

		[[nodiscard]] const_reference front() const noexcept
		{
			return array[0];
		}

		[[nodiscard]] reference back() noexcept
		{
			return array[count - 1];
		}

		[[nodiscard]] const_reference back() const noexcept
		{
			return array[count - 1];
		}

		[[nodiscard]] bool empty() const noexcept
		{
			return !count;
		}

		[[nodiscard]] size_type size() const noexcept
		{
			return count;
		}

		[[nodiscard]] constexpr size_type max_size() const noexcept
		{
			return std::numeric_limits<size_type>::max();
		}

		[[nodiscard]] pointer data() noexcept
		{
			return array;
		}

		[[nodiscard]] const_pointer data() const noexcept
		{
			return array;
		}

		void swap(Array& other) noexcept
		{
			std::swap(count, other.count);
			std::swap(array, other.array);
		}

		friend void swap(Array& left, Array& right) noexcept
		{
			left.swap(right);
		}

		template<typename U> void fill(U&& value) noexcept
		{
			std::fill(begin(), end(), std::forward<U>(value));
		}

	private:
		template<typename QualifiedT> class Iterator
		{
			friend Array;

		public:
			using iterator_category = std::contiguous_iterator_tag;
			using value_type		= QualifiedT;
			using difference_type	= Array::difference_type;
			using pointer			= QualifiedT*;
			using reference			= QualifiedT&;

			Iterator() noexcept = default;

			[[nodiscard]] reference operator*() const noexcept
			{
				return *Position;
			}

			[[nodiscard]] pointer operator->() const noexcept
			{
				return Position;
			}

			template<typename U> [[nodiscard]] bool operator==(Iterator<U> other) const noexcept
			{
				return Position == other.Position;
			}

			template<typename U> [[nodiscard]] bool operator!=(Iterator<U> other) const noexcept
			{
				return Position != other.Position;
			}

			template<typename U> [[nodiscard]] bool operator>(Iterator<U> other) const noexcept
			{
				return Position > other.Position;
			}

			template<typename U> [[nodiscard]] bool operator>=(Iterator<U> other) const noexcept
			{
				return Position >= other.Position;
			}

			template<typename U> [[nodiscard]] bool operator<(Iterator<U> other) const noexcept
			{
				return Position < other.Position;
			}

			template<typename U> [[nodiscard]] bool operator<=(Iterator<U> other) const noexcept
			{
				return Position <= other.Position;
			}

			template<typename U> [[nodiscard]] auto operator<=>(Iterator<U> other) const noexcept
			{
				return Position <=> other.Position;
			}

			Iterator& operator++() noexcept
			{
				incrementPosition(1);
				return *this;
			}

			Iterator operator++(int) noexcept
			{
				auto old {*this};
				incrementPosition(1);
				return old;
			}

			Iterator& operator--() noexcept
			{
				decrementPosition(1);
				return *this;
			}

			Iterator operator--(int) noexcept
			{
				auto old {*this};
				decrementPosition(1);
				return old;
			}

			Iterator& operator+=(difference_type offset) noexcept
			{
				incrementPosition(offset);
				return *this;
			}

			[[nodiscard]] Iterator operator+(difference_type offset) const noexcept
			{
				auto copy {*this};
				return copy += offset;
			}

			friend [[nodiscard]] Iterator operator+(difference_type offset, Iterator iterator) noexcept
			{
				return iterator + offset;
			}

			Iterator& operator-=(difference_type offset) noexcept
			{
				decrementPosition(offset);
				return *this;
			}

			[[nodiscard]] Iterator operator-(difference_type offset) const noexcept
			{
				auto copy {*this};
				return copy -= offset;
			}

			template<typename U> [[nodiscard]] difference_type operator-(Iterator<U> other) const noexcept
			{
				return Position - other.Position;
			}

			[[nodiscard]] reference operator[](size_type index) const noexcept
			{
				return *(*this + index);
			}

		private:
			pointer Position {};

#ifndef NDEBUG
			pointer Begin {};
			pointer End {};

			Iterator(pointer pos, pointer begin, pointer end) noexcept : Position {pos}, Begin {begin}, End {end}
			{}
#else
			Iterator(pointer pos) noexcept : Position {pos}
			{}
#endif

			void incrementPosition(difference_type offset) noexcept
			{
				assert(("Cannot increment iterator past end.", Position < End));
				Position += offset;
			}

			void decrementPosition(difference_type offset) noexcept
			{
				assert(("Cannot decrement iterator before begin.", Begin < Position));
				Position -= offset;
			}
		};

	public:
		using iterator				 = Iterator<value_type>;
		using const_iterator		 = Iterator<const value_type>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		[[nodiscard]] iterator begin() noexcept
		{
#ifndef NDEBUG
			return {array, array, array + count};
#else
			return {Data};
#endif
		}

		[[nodiscard]] const_iterator begin() const noexcept
		{
#ifndef NDEBUG
			return {array, array, array + count};
#else
			return {Data};
#endif
		}

		[[nodiscard]] iterator end() noexcept
		{
#ifndef NDEBUG
			pointer endPos {array + count};
			return {endPos, array, endPos};
#else
			return {Data + Size};
#endif
		}

		[[nodiscard]] const_iterator end() const noexcept
		{
#ifndef NDEBUG
			pointer endPos {array + count};
			return {endPos, array, endPos};
#else
			return {Data + Size};
#endif
		}

		[[nodiscard]] reverse_iterator rbegin() noexcept
		{
			return reverse_iterator {end()};
		}

		[[nodiscard]] const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator {end()};
		}

		[[nodiscard]] reverse_iterator rend() noexcept
		{
			return reverse_iterator {begin()};
		}

		[[nodiscard]] const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator {begin()};
		}

		[[nodiscard]] const_iterator cbegin() const noexcept
		{
			return begin();
		}

		[[nodiscard]] const_iterator cend() const noexcept
		{
			return end();
		}

		[[nodiscard]] const_reverse_iterator crbegin() const noexcept
		{
			return rbegin();
		}

		[[nodiscard]] const_reverse_iterator crend() const noexcept
		{
			return rend();
		}

	private:
		size_type count {};
		pointer array {};
	};
}
