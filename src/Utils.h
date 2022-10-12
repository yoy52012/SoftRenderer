#pragma once
#include <string>
namespace SoftRenderer
{
#define IMAGE_DIR RESOURCE_DIR"/Images/"

#define ENUM_CLASS_FLAGS(Enum) \
	inline           Enum& operator|=(Enum& lhs, Enum rhs) { return lhs = static_cast<Enum>(static_cast<std::underlying_type<Enum>::type>(lhs) | static_cast<std::underlying_type<Enum>::type>(rhs)); } \
	inline           Enum& operator&=(Enum& lhs, Enum rhs) { return lhs = static_cast<Enum>(static_cast<std::underlying_type<Enum>::type>(lhs) & static_cast<std::underlying_type<Enum>::type>(rhs)); } \
	inline           Enum& operator^=(Enum& lhs, Enum rhs) { return lhs = static_cast<Enum>(static_cast<std::underlying_type<Enum>::type>(lhs) ^ static_cast<std::underlying_type<Enum>::type>(rhs)); } \
	inline constexpr Enum  operator| (Enum  lhs, Enum rhs) { return static_cast<Enum>(static_cast<std::underlying_type<Enum>::type>(lhs) | static_cast<std::underlying_type<Enum>::type>(rhs)); } \
	inline constexpr Enum  operator& (Enum  lhs, Enum rhs) { return static_cast<Enum>(static_cast<std::underlying_type<Enum>::type>(lhs) & static_cast<std::underlying_type<Enum>::type>(rhs)); } \
	inline constexpr Enum  operator^ (Enum  lhs, Enum rhs) { return static_cast<Enum>(static_cast<std::underlying_type<Enum>::type>(lhs) ^ static_cast<std::underlying_type<Enum>::type>(rhs)); } \
	inline constexpr bool  operator! (Enum  e)             { return !static_cast<std::underlying_type<Enum>::type>(e); } \
	inline constexpr Enum  operator~ (Enum  e)             { return static_cast<Enum>(~(static_cast<std::underlying_type<Enum>::type>(e))); }

}