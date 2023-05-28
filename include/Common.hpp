#pragma once

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

} // namespace hh
