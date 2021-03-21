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
		Array() = default;

		Array(T data[], size_t size) : Size {size}, Data {data}
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

		T& operator[](size_t index) noexcept
		{
			assert(("Index into array was out of range.", index < Size));
			return Data[index];
		}

		const T& operator[](size_t index) const noexcept
		{
			assert(("Index into array was out of range.", index < Size));
			return Data[index];
		}

		bool operator==(const Array& other) const noexcept
		{
			if(Size != other.Size)
				return false;

			auto otherElement {other.begin()};
			for(const T& element : *this)
				if(element != *otherElement++)
					return false;

			return true;
		}

		bool operator!=(const Array& other) const noexcept
		{
			return !operator==(other);
		}

		bool operator<(const Array& other) const noexcept
		{
			return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
		}

		bool operator>(const Array& other) const noexcept
		{
			return other < *this;
		}

		bool operator<=(const Array& other) const noexcept
		{
			return !operator>(other);
		}

		bool operator>=(const Array& other) const noexcept
		{
			return !operator<(other);
		}

		auto operator<=>(const Array& other) const noexcept
		{
			return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end());
		}

		T& at(size_t index)
		{
			if(index < Size)
				return Data[index];
			throw std::out_of_range("Index into array was out of range.");
		}

		const T& at(size_t index) const
		{
			if(index < Size)
				return Data[index];
			throw std::out_of_range("Index into array was out of range.");
		}

		T& front() noexcept
		{
			return Data[0];
		}

		const T& front() const noexcept
		{
			return Data[0];
		}

		T& back() noexcept
		{
			return Data[Size - 1];
		}

		const T& back() const noexcept
		{
			return Data[Size - 1];
		}

		bool empty() const noexcept
		{
			return !Size;
		}

		size_t size() const noexcept
		{
			return Size;
		}

		T* data() noexcept
		{
			return Data;
		}

		const T* data() const noexcept
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
			using difference_type	= ptrdiff_t;
			using value_type		= QualifiedT;
			using pointer			= QualifiedT*;
			using reference			= QualifiedT&;

			ArrayIterator() noexcept = default;

			QualifiedT& operator*() const noexcept
			{
				return *Position;
			}

			QualifiedT* operator->() const noexcept
			{
				return Position;
			}

			bool operator==(ArrayIterator other) const noexcept
			{
				return Position == other.Position;
			}

			bool operator!=(ArrayIterator other) const noexcept
			{
				return Position != other.Position;
			}

			bool operator>(ArrayIterator other) const noexcept
			{
				return Position > other.Position;
			}

			bool operator>=(ArrayIterator other) const noexcept
			{
				return Position >= other.Position;
			}

			bool operator<(ArrayIterator other) const noexcept
			{
				return Position < other.Position;
			}

			bool operator<=(ArrayIterator other) const noexcept
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

			ArrayIterator& operator+=(ptrdiff_t offset) noexcept
			{
				incrementPosition(offset);
				return *this;
			}

			const ArrayIterator& operator+=(ptrdiff_t offset) const noexcept
			{
				incrementPosition(offset);
				return *this;
			}

			ArrayIterator operator+(ptrdiff_t offset) const noexcept
			{
				auto copy {*this};
				return copy += offset;
			}

			friend ArrayIterator operator+(ptrdiff_t offset, ArrayIterator iterator) noexcept
			{
				return iterator + offset;
			}

			ArrayIterator& operator-=(ptrdiff_t offset) noexcept
			{
				decrementPosition(offset);
				return *this;
			}

			const ArrayIterator& operator-=(ptrdiff_t offset) const noexcept
			{
				decrementPosition(offset);
				return *this;
			}

			ArrayIterator operator-(ptrdiff_t offset) const noexcept
			{
				auto copy {*this};
				return copy -= offset;
			}

			ptrdiff_t operator-(ArrayIterator other) const noexcept
			{
				return Position - other.Position;
			}

			QualifiedT& operator[](size_t index) const noexcept
			{
				return *(*this + index);
			}

		private:
			mutable QualifiedT* Position {};

#ifndef NDEBUG
			QualifiedT* Begin {};
			QualifiedT* End {};

			ArrayIterator(QualifiedT* pos, QualifiedT* begin, QualifiedT* end) noexcept :
				Position {pos}, Begin {begin}, End {end}
			{}
#else
			ArrayIterator(QualifiedT* pos) noexcept : Position {pos}
			{}
#endif

			void incrementPosition(ptrdiff_t offset) const noexcept
			{
				assert(("Cannot increment iterator past end.", Position < End));
				Position += offset;
			}

			void decrementPosition(ptrdiff_t offset) const noexcept
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

		Iterator begin() noexcept
		{
#ifndef NDEBUG
			return {Data, Data, Data + Size};
#else
			return {Data};
#endif
		}

		ConstIterator begin() const noexcept
		{
#ifndef NDEBUG
			return {Data, Data, Data + Size};
#else
			return {Data};
#endif
		}

		Iterator end() noexcept
		{
#ifndef NDEBUG
			T* endPos {Data + Size};
			return {endPos, Data, endPos};
#else
			return {Data + Size};
#endif
		}

		ConstIterator end() const noexcept
		{
#ifndef NDEBUG
			T* endPos {Data + Size};
			return {endPos, Data, endPos};
#else
			return {Data + Size};
#endif
		}

		ReverseIterator rbegin() noexcept
		{
			return ReverseIterator {end()};
		}

		ConstReverseIterator rbegin() const noexcept
		{
			return ConstReverseIterator {end()};
		}

		ReverseIterator rend() noexcept
		{
			return ReverseIterator {begin()};
		}

		ConstReverseIterator rend() const noexcept
		{
			return ConstReverseIterator {begin()};
		}

		ConstIterator cbegin() const noexcept
		{
			return begin();
		}

		ConstIterator cend() const noexcept
		{
			return end();
		}

		ConstReverseIterator crbegin() const noexcept
		{
			return rbegin();
		}

		ConstReverseIterator crend() const noexcept
		{
			return rend();
		}

	private:
		size_t Size {};
		T* Data {};
	};
}
