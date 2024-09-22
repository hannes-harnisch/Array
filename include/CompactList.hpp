#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace hh {

template<typename T>
class CompactList {
	struct Storage {
		size_t count = 0;
		size_t capacity = 0;
		T data[];
	};

	T* data_ptr;

	static inline const Storage Empty {};

public:
	CompactList() noexcept :
		data_ptr(const_cast<T*>(Empty.data)) {
	}

	explicit CompactList(size_t count) {
		Storage* storage = static_cast<Storage*>(::operator new(sizeof(Storage) + sizeof(T) * count));
		storage->count = count;
		storage->capacity = count;

		T* it = storage->data;
		T* const end = it + count;
		try {
			for (; it != end; ++it) {
				new (it) T();
			}
		} catch (...) {
			T* const begin = end - count;
			while (it != begin) {
				--it;
				it->~T();
			}
			::operator delete(storage);
			throw;
		}

		data_ptr = storage->data;
	}

	CompactList(const CompactList& other) {
		Storage* storage = allocate_and_copy(other.storage());
		data_ptr = storage->data;
	}

	CompactList(CompactList&& other) noexcept :
		data_ptr(other.data_ptr) {
		other.data_ptr = const_cast<Storage*>(&Empty);
	}

	~CompactList() {
		delete_data();
	}

	CompactList& operator=(const CompactList& other) {
		if (this != &other) {
			const size_t count = other.data_ptr->count;
			if (data_ptr->capacity >= count) {
				const bool this_larger = data_ptr->count > count;

				T* it = data_ptr->data;
				T* other_it = other.data_ptr->data;
				T* end = data_ptr->data + (this_larger ? count : data_ptr->count);
				for (; it != end; ++it) {
					*it = *other_it++;
				}

				if (this_larger) {
					end = data_ptr->data + data_ptr->count;
					for (; it != end; ++it) {
						it->~T();
					}
				} else {
					end = data_ptr->data + count;
					for (; it != end; ++it) {
						new (it) T(*other_it++);
					}
				}
				data_ptr->count = count;
			} else {
				Storage* storage = allocate_and_copy(other.storage());
				delete_data();
				data_ptr = storage;
			}
		}
		return *this;
	}

	CompactList& operator=(CompactList&& other) noexcept {
		delete_data();
		data_ptr = other.data_ptr;
		other.data_ptr = const_cast<Storage*>(&Empty);
		return *this;
	}

	T& operator[](size_t index) noexcept {
		assert(index < data_ptr->count);

		return data_ptr[index];
	}

	const T& operator[](size_t index) const noexcept {
		assert(index < data_ptr->count);

		return data_ptr->data[index];
	}

	T& at(size_t index) {
		if (index < data_ptr->count) {
			return data_ptr->data[index];
		}

		throw std::out_of_range("index into list was out of range");
	}

	const T& at(size_t index) const {
		if (index < data_ptr->count) {
			return data_ptr->data[index];
		}

		throw std::out_of_range("index into list was out of range");
	}

	T& front() noexcept {
		assert(data_ptr->count != 0);

		return data_ptr[0];
	}

	const T& front() const noexcept {
		assert(data_ptr->count != 0);

		return data_ptr[0];
	}

	T& back() noexcept {
		assert(data_ptr->count != 0);

		return data_ptr[storage() - 1];
	}

	const T& back() const noexcept {
		assert(data_ptr->count != 0);

		return data_ptr[data_ptr->count - 1];
	}

	[[nodiscard]] bool empty() const noexcept {
		return data_ptr->count == 0;
	}

	size_t size() const noexcept {
		return data_ptr->count;
	}

	size_t capacity() const noexcept {
		return data_ptr->capacity;
	}

	T* data() noexcept {
		return data_ptr->data;
	}

	const T* data() const noexcept {
		return data_ptr->data;
	}

	template<typename... Args>
	T& emplace_back(Args&&... args) {
		if (data_ptr->count == data_ptr->capacity) {
			grow();
		}

		T* new_elem = new (data_ptr->data + data_ptr->count) T(std::forward<Args>(args)...);
		++data_ptr->count;
		return *new_elem;
	}

	T& push_back(const T& value) {
		return emplace_back(value);
	}

	T& push_back(T&& value) {
		return emplace_back(std::move(value));
	}

	void reserve(size_t new_cap) {
	}

private:
	Storage* storage() noexcept {
		const size_t offset = offsetof(Storage, data);
		return reinterpret_cast<Storage*>(reinterpret_cast<char*>(data_ptr) - offset);
	}

	const Storage* storage() const noexcept {
		const size_t offset = offsetof(Storage, data);
		return reinterpret_cast<const Storage*>(reinterpret_cast<const char*>(data_ptr) - offset);
	}

	void delete_data() noexcept {
		if (data_ptr == Empty.data) {
			return;
		}

		if constexpr (!std::is_trivially_destructible_v<T>) {
			T* const end = data_ptr->data + data_ptr->count;
			for (T* it = data_ptr->data; it != end; ++it) {
				it->~T();
			}
		}

		::operator delete(data_ptr);
	}

	static Storage* allocate_and_copy(const Storage* source) {
		const size_t count = source->count;

		Storage* storage = static_cast<Storage*>(::operator new(sizeof(Storage) + sizeof(T) * count));
		storage->count = count;
		storage->capacity = count;

		if constexpr (std::is_trivially_copy_constructible_v<T>) {
			std::memcpy(storage->data, source->data, sizeof(T) * count);
		} else {
			T* src = source->data;
			T* dst = storage->data;
			T* const end = dst + count;
			try {
				for (; dst != end; ++dst, ++src) {
					new (dst) T(*src);
				}
			} catch (...) {
				T* const begin = end - count;
				while (dst != begin) {
					--dst;
					dst->~T();
				}
				::operator delete(storage);
				throw;
			}
		}
		return storage;
	}
};

} // namespace hh
