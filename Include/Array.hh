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

		Array(T data[], size_t count) noexcept : count {count}, array {data}
		{}

		Array(size_t count) : Array {new T[count], count}
		{}

		template<typename U> Array(size_t count, U&& initialValue) : Array {count}
		{
			fill(std::forward<U>(initialValue));
		}

		template<typename U> Array(size_t count, std::initializer_list<U> initialValues) : Array {count}
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

		[[nodiscard]] T& operator[](size_t index) noexcept
		{
			assert(("Index into array was out of range.", index < count));
			return array[index];
		}

		[[nodiscard]] const T& operator[](size_t index) const noexcept
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

		[[nodiscard]] T& at(size_t index)
		{
			if(index < count)
				return array[index];
			throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] const T& at(size_t index) const
		{
			if(index < count)
				return array[index];
			throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] T& front() noexcept
		{
			return array[0];
		}

		[[nodiscard]] const T& front() const noexcept
		{
			return array[0];
		}

		[[nodiscard]] T& back() noexcept
		{
			return array[count - 1];
		}

		[[nodiscard]] const T& back() const noexcept
		{
			return array[count - 1];
		}

		[[nodiscard]] bool empty() const noexcept
		{
			return !count;
		}

		[[nodiscard]] size_t size() const noexcept
		{
			return count;
		}

		[[nodiscard]] constexpr size_t max_size() const noexcept
		{
			return std::numeric_limits<size_type>::max();
		}

		[[nodiscard]] T* data() noexcept
		{
			return array;
		}

		[[nodiscard]] const T* data() const noexcept
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
		template<typename V> class Iterator
		{
			friend Array;

		public:
			using iterator_category = std::contiguous_iterator_tag;
			using value_type		= V;
			using difference_type	= std::ptrdiff_t;
			using pointer			= V*;
			using reference			= V&;

			Iterator() noexcept = default;

			[[nodiscard]] V& operator*() const noexcept
			{
				return *pos;
			}

			[[nodiscard]] V* operator->() const noexcept
			{
				return pos;
			}

			template<typename U> [[nodiscard]] bool operator==(Iterator<U> other) const noexcept
			{
				return pos == other.pos;
			}

			template<typename U> [[nodiscard]] bool operator!=(Iterator<U> other) const noexcept
			{
				return pos != other.pos;
			}

			template<typename U> [[nodiscard]] bool operator>(Iterator<U> other) const noexcept
			{
				return pos > other.pos;
			}

			template<typename U> [[nodiscard]] bool operator>=(Iterator<U> other) const noexcept
			{
				return pos >= other.pos;
			}

			template<typename U> [[nodiscard]] bool operator<(Iterator<U> other) const noexcept
			{
				return pos < other.pos;
			}

			template<typename U> [[nodiscard]] bool operator<=(Iterator<U> other) const noexcept
			{
				return pos <= other.pos;
			}

			template<typename U> [[nodiscard]] auto operator<=>(Iterator<U> other) const noexcept
			{
				return pos <=> other.pos;
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

			Iterator& operator+=(ptrdiff_t offset) noexcept
			{
				incrementPosition(offset);
				return *this;
			}

			[[nodiscard]] Iterator operator+(ptrdiff_t offset) const noexcept
			{
				auto old {*this};
				return old += offset;
			}

			friend [[nodiscard]] Iterator operator+(ptrdiff_t offset, Iterator iterator) noexcept
			{
				return iterator + offset;
			}

			Iterator& operator-=(ptrdiff_t offset) noexcept
			{
				decrementPosition(offset);
				return *this;
			}

			[[nodiscard]] Iterator operator-(ptrdiff_t offset) const noexcept
			{
				auto old {*this};
				return old -= offset;
			}

			template<typename U> [[nodiscard]] ptrdiff_t operator-(Iterator<U> other) const noexcept
			{
				return pos - other.pos;
			}

			[[nodiscard]] V& operator[](size_t index) const noexcept
			{
				return *(*this + index);
			}

		private:
			V* pos {};

#ifndef NDEBUG
			V* begin {};
			V* end {};

			Iterator(V* pos, V* begin, V* end) noexcept : pos {pos}, begin {begin}, end {end}
			{}
#else
			Iterator(V* pos) noexcept : pos {pos}
			{}
#endif

			void incrementPosition(ptrdiff_t offset) noexcept
			{
				assert(("Cannot increment iterator past end.", pos < end));
				pos += offset;
			}

			void decrementPosition(ptrdiff_t offset) noexcept
			{
				assert(("Cannot decrement iterator before begin.", begin < pos));
				pos -= offset;
			}
		};

	public:
		using iterator				 = Iterator<T>;
		using const_iterator		 = Iterator<const T>;
		using reverse_iterator		 = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		[[nodiscard]] iterator begin() noexcept
		{
#ifndef NDEBUG
			return {array, array, array + count};
#else
			return {array};
#endif
		}

		[[nodiscard]] const_iterator begin() const noexcept
		{
#ifndef NDEBUG
			return {array, array, array + count};
#else
			return {array};
#endif
		}

		[[nodiscard]] iterator end() noexcept
		{
#ifndef NDEBUG
			T* endPos {array + count};
			return {endPos, array, endPos};
#else
			return {array + count};
#endif
		}

		[[nodiscard]] const_iterator end() const noexcept
		{
#ifndef NDEBUG
			T* endPos {array + count};
			return {endPos, array, endPos};
#else
			return {array + count};
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
		size_t count {};
		T* array {};
	};
}
