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
		Array() noexcept = default;

		Array(T data[], size_t size) noexcept : Size {size}, Data {data}
		{}

		Array(size_t size) : Array {new T[size], size}
		{}

		template<typename U> Array(size_t size, const U& initialValue) : Array {size}
		{
			fill(initialValue);
		}

		template<typename U> Array(size_t size, std::initializer_list<U> initialValues) : Array {size}
		{
			assert(("Size of initializer list exceeds array size.", Size >= initialValues.size()));

			auto element {begin()};
			for(const U& value : initialValues)
				*element++ = value;
		}

		Array(const Array& other) : Array {other.Size}
		{
			auto otherElement {other.begin()};
			for(T& element : *this)
				element = *otherElement++;
		}

		Array(Array&& other) noexcept : Array {std::exchange(other.Data, nullptr), other.Size}
		{}

		~Array()
		{
			delete[] Data;
		}

		Array& operator=(const Array& other)
		{
			assert(("Attempted to assign array to itself.", this != &other));

			delete[] Data;
			Size = other.Size;
			Data = new T[Size];

			auto otherElement {other.begin()};
			for(T& element : *this)
				element = *otherElement++;

			return *this;
		}

		Array& operator=(Array&& other) noexcept
		{
			assert(("Attempted to assign array to itself.", this != &other));

			Size = other.Size;
			std::swap(Data, other.Data);
			return *this;
		}

		[[nodiscard]] T& operator[](size_t index) noexcept
		{
			assert(("Index into array was out of range.", index < Size));
			return Data[index];
		}

		[[nodiscard]] const T& operator[](size_t index) const noexcept
		{
			assert(("Index into array was out of range.", index < Size));
			return Data[index];
		}

		[[nodiscard]] bool operator==(const Array& other) const noexcept
		{
			if(Size != other.Size)
				return false;

			auto otherElement {other.begin()};
			for(const T& element : *this)
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
			if(index < Size)
				return Data[index];
			throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] const T& at(size_t index) const
		{
			if(index < Size)
				return Data[index];
			throw std::out_of_range("Index into array was out of range.");
		}

		[[nodiscard]] T& front() noexcept
		{
			return Data[0];
		}

		[[nodiscard]] const T& front() const noexcept
		{
			return Data[0];
		}

		[[nodiscard]] T& back() noexcept
		{
			return Data[Size - 1];
		}

		[[nodiscard]] const T& back() const noexcept
		{
			return Data[Size - 1];
		}

		[[nodiscard]] bool empty() const noexcept
		{
			return !Size;
		}

		[[nodiscard]] size_t size() const noexcept
		{
			return Size;
		}

		[[nodiscard]] T* data() noexcept
		{
			return Data;
		}

		[[nodiscard]] const T* data() const noexcept
		{
			return Data;
		}

		void swap(Array& other) noexcept
		{
			std::swap(Size, other.Size);
			std::swap(Data, other.Data);
		}

		template<typename U> void fill(const U& value) noexcept
		{
			for(T& element : *this)
				element = value;
		}

	private:
		template<typename QualifiedT> class ArrayIterator
		{
			friend Array;

		public:
			using iterator_category = std::contiguous_iterator_tag;
			using value_type		= QualifiedT;
			using difference_type	= ptrdiff_t;
			using pointer			= QualifiedT*;
			using reference			= QualifiedT&;

			ArrayIterator() noexcept = default;

			[[nodiscard]] reference operator*() const noexcept
			{
				return *Position;
			}

			[[nodiscard]] pointer operator->() const noexcept
			{
				return Position;
			}

			template<typename U> [[nodiscard]] bool operator==(ArrayIterator<U> other) const noexcept
			{
				return Position == other.Position;
			}

			template<typename U> [[nodiscard]] bool operator!=(ArrayIterator<U> other) const noexcept
			{
				return Position != other.Position;
			}

			template<typename U> [[nodiscard]] bool operator>(ArrayIterator<U> other) const noexcept
			{
				return Position > other.Position;
			}

			template<typename U> [[nodiscard]] bool operator>=(ArrayIterator<U> other) const noexcept
			{
				return Position >= other.Position;
			}

			template<typename U> [[nodiscard]] bool operator<(ArrayIterator<U> other) const noexcept
			{
				return Position < other.Position;
			}

			template<typename U> [[nodiscard]] bool operator<=(ArrayIterator<U> other) const noexcept
			{
				return Position <= other.Position;
			}

			ArrayIterator& operator++() noexcept
			{
				incrementPosition(1);
				return *this;
			}

			const ArrayIterator& operator++() const noexcept
			{
				incrementPosition(1);
				return *this;
			}

			ArrayIterator operator++(int) const noexcept
			{
				auto old {*this};
				incrementPosition(1);
				return old;
			}

			ArrayIterator& operator--() noexcept
			{
				decrementPosition(1);
				return *this;
			}

			const ArrayIterator& operator--() const noexcept
			{
				decrementPosition(1);
				return *this;
			}

			ArrayIterator operator--(int) const noexcept
			{
				auto old {*this};
				decrementPosition(1);
				return old;
			}

			ArrayIterator& operator+=(difference_type offset) noexcept
			{
				incrementPosition(offset);
				return *this;
			}

			const ArrayIterator& operator+=(difference_type offset) const noexcept
			{
				incrementPosition(offset);
				return *this;
			}

			[[nodiscard]] ArrayIterator operator+(difference_type offset) const noexcept
			{
				auto copy {*this};
				return copy += offset;
			}

			friend [[nodiscard]] ArrayIterator operator+(difference_type offset, ArrayIterator iterator) noexcept
			{
				return iterator + offset;
			}

			ArrayIterator& operator-=(difference_type offset) noexcept
			{
				decrementPosition(offset);
				return *this;
			}

			const ArrayIterator& operator-=(difference_type offset) const noexcept
			{
				decrementPosition(offset);
				return *this;
			}

			[[nodiscard]] ArrayIterator operator-(difference_type offset) const noexcept
			{
				auto copy {*this};
				return copy -= offset;
			}

			template<typename U> [[nodiscard]] difference_type operator-(ArrayIterator<U> other) const noexcept
			{
				return Position - other.Position;
			}

			[[nodiscard]] reference operator[](size_t index) const noexcept
			{
				return *(*this + index);
			}

		private:
			mutable pointer Position {};

#ifndef NDEBUG
			pointer Begin {};
			pointer End {};

			ArrayIterator(pointer pos, pointer begin, pointer end) noexcept : Position {pos}, Begin {begin}, End {end}
			{}
#else
			ArrayIterator(pointer pos) noexcept : Position {pos}
			{}
#endif

			void incrementPosition(difference_type offset) const noexcept
			{
				assert(("Cannot increment iterator past end.", Position < End));
				Position += offset;
			}

			void decrementPosition(difference_type offset) const noexcept
			{
				assert(("Cannot decrement iterator before begin.", Begin < Position));
				Position -= offset;
			}
		};

	public:
		using Iterator			   = ArrayIterator<T>;
		using ConstIterator		   = ArrayIterator<const T>;
		using ReverseIterator	   = std::reverse_iterator<Iterator>;
		using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

		[[nodiscard]] Iterator begin() noexcept
		{
#ifndef NDEBUG
			return {Data, Data, Data + Size};
#else
			return {Data};
#endif
		}

		[[nodiscard]] ConstIterator begin() const noexcept
		{
#ifndef NDEBUG
			return {Data, Data, Data + Size};
#else
			return {Data};
#endif
		}

		[[nodiscard]] Iterator end() noexcept
		{
#ifndef NDEBUG
			T* endPos {Data + Size};
			return {endPos, Data, endPos};
#else
			return {Data + Size};
#endif
		}

		[[nodiscard]] ConstIterator end() const noexcept
		{
#ifndef NDEBUG
			T* endPos {Data + Size};
			return {endPos, Data, endPos};
#else
			return {Data + Size};
#endif
		}

		[[nodiscard]] ReverseIterator rbegin() noexcept
		{
			return ReverseIterator {end()};
		}

		[[nodiscard]] ConstReverseIterator rbegin() const noexcept
		{
			return ConstReverseIterator {end()};
		}

		[[nodiscard]] ReverseIterator rend() noexcept
		{
			return ReverseIterator {begin()};
		}

		[[nodiscard]] ConstReverseIterator rend() const noexcept
		{
			return ConstReverseIterator {begin()};
		}

		[[nodiscard]] ConstIterator cbegin() const noexcept
		{
			return begin();
		}

		[[nodiscard]] ConstIterator cend() const noexcept
		{
			return end();
		}

		[[nodiscard]] ConstReverseIterator crbegin() const noexcept
		{
			return rbegin();
		}

		[[nodiscard]] ConstReverseIterator crend() const noexcept
		{
			return rend();
		}

	private:
		size_t Size {};
		T* Data {};
	};
}
