#include <type_traits>

template <typename, typename = void>
struct has_field : std::false_type {};

template <typename T>
struct has_field< T, std::void_t< // here is where you would add functional requirements  super strict, your struct must have a certain value;
	decltype(T::field),
	decltype(&T::member_fun)
	>> 
: std::true_type{};

template<typename T>
constexpr bool has_field_v = has_field<T>::value;

template<typename T>
using requires_field = std::enable_if_t<has_field_v<T>, T>; 

template <class T, requires_field<T>* = nullptr>
auto fun(T & value) {
	return value.field * value.field;
}

struct test {
	double field;
	double member_fun(double, double);
};

