#pragma once

#include <type_traits>

#ifdef __cpp_lib_bit_cast
	#include <bit>
#endif

namespace hh {

namespace detail {

	template<typename T>
	constexpr bool is_default_all_zero() {
		T value = T();

		struct ByteArray {
			unsigned char bytes[sizeof(T)];
		};

		ByteArray array = std::bit_cast<ByteArray>(value);
		for (unsigned char byte : array.bytes)
			if (byte != 0)
				return false;

		return true;
	}

	template<typename T>
	constexpr bool is_memset_value_constructible(int (*)[is_default_all_zero<T>()]) {
		return is_default_all_zero<T>();
	}

	template<typename>
	constexpr bool is_memset_value_constructible(...) {
		return false;
	}

} // namespace detail

template<typename T>
constexpr inline bool is_memset_value_constructible =
#ifdef __cpp_lib_bit_cast
	detail::is_memset_value_constructible<T>(nullptr)
#else
	std::is_scalar_v<T> && !std::is_member_object_pointer_v<T>
#endif
	;

} // namespace hh
