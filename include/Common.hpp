#pragma once

#include <cstring>
#include <memory>
#include <type_traits>

#ifdef __cpp_lib_bit_cast
	#include <bit>
#endif

namespace hh {

namespace detail {

#ifdef __cpp_lib_bit_cast
	template<typename T>
	struct memset_value_constructible {
		static constexpr bool check_trait() {
			if constexpr (std::is_default_constructible_v<T> && std::is_trivially_copyable_v<T>) {
				T value = T();

				struct ByteArray {
					unsigned char bytes[sizeof(T)];
				};

				const ByteArray array = std::bit_cast<ByteArray>(value);
				for (unsigned char byte : array.bytes)
					if (byte != 0)
						return false;

				return true;
			}
			return false;
		}

		template<bool TRAIT = check_trait()>
		static constexpr bool sfinae_check(int) {
			return TRAIT;
		}

		static constexpr bool sfinae_check(...) {
			return false;
		}

		static constexpr bool value = sfinae_check(0);
	};
#endif

} // namespace detail

template<typename T>
constexpr inline bool is_memset_value_constructible =
#ifdef __cpp_lib_bit_cast
	detail::memset_value_constructible<T>::value
#else
	std::is_scalar_v<T> && !std::is_member_object_pointer_v<T>
#endif
	;

template<typename T, typename Allocator, typename Pointer, typename ConstPointer, typename SizeType>
T* allocator_copy_initialize_n(Allocator& allocator, Pointer dst_ptr, ConstPointer src_ptr, SizeType alloc_count, size_t n) {
	using AllocTraits = std::allocator_traits<Allocator>;

	T*		 dst = std::to_address(dst_ptr);
	const T* src = std::to_address(src_ptr);
	if constexpr (std::is_trivially_copyable_v<T>) {
		std::memcpy(dst, src, sizeof(T) * n);
		dst += n;
	} else {
		T* const end = dst + n;
		try {
			for (; dst != end; ++dst, ++src)
				AllocTraits::construct(allocator, dst, *src);
		} catch (...) {
			T* const begin = end - n;
			while (dst != begin) {
				--dst;
				AllocTraits::destroy(allocator, dst);
			}
			AllocTraits::deallocate(allocator, dst_ptr, alloc_count);
			throw;
		}
	}
	return dst;
}

template<typename Allocator, typename Pointer, typename SizeType, typename T>
void allocator_value_initialize_n(Allocator& allocator, Pointer storage, SizeType alloc_count, T* dst, size_t n) {
	using AllocTraits = std::allocator_traits<Allocator>;

	if constexpr (is_memset_value_constructible<T>) { // TODO: allocator construct method check
		std::memset(dst, 0, sizeof(T) * n);
	} else {
		T* const end = dst + n;
		try {
			for (; dst != end; ++dst)
				AllocTraits::construct(allocator, dst);
		} catch (...) {
			T* const begin = end - n;
			while (dst != begin) {
				--dst;
				AllocTraits::destroy(allocator, dst);
			}
			AllocTraits::deallocate(allocator, storage, alloc_count);
			throw;
		}
	}
}

template<typename Allocator, typename Pointer, typename SizeType, typename T, typename U>
void allocator_fill_initialize_n(Allocator& allocator, Pointer ptr, SizeType alloc_count, T* dst, const U& value, size_t n) {
	using AllocTraits = std::allocator_traits<Allocator>;

	if constexpr (std::is_trivially_constructible_v<T, U> && sizeof(T) == 1 && sizeof(U) == 1) {
		int byte;
		std::memcpy(&byte, &value, 1);
		std::memset(std::to_address(ptr), byte, sizeof(T) * n);
	} else {
		T* const end = dst + n;
		try {
			for (; dst != end; ++dst)
				AllocTraits::construct(allocator, dst, value);
		} catch (...) {
			T* const begin = end - n;
			while (dst != begin) {
				--dst;
				AllocTraits::destroy(allocator, dst);
			}
			AllocTraits::deallocate(allocator, ptr, alloc_count);
			throw;
		}
	}
}

} // namespace hh
