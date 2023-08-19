#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace ql
{

template<typename T>
struct type_identity
{
  using type = T;
};

template<typename T>
using type_identity_t = typename type_identity<T>::type;

template<typename... Ts>
struct parameter_pack
{
};


template<typename T>
struct function_prototype
{
};

template<typename R, typename... Ts>
struct function_prototype<R( Ts... )>
{
};

template<typename T>
struct callable_type;

template<typename R, typename... Args>
struct callable_type<R( Args... )>
{
  using return_type    = R;
  using argument_types = parameter_pack<Args...>;
};

template<typename R, typename... Args>
struct callable_type<R ( * )( Args... )> : callable_type<R( Args... )>
{
};

template<typename R, typename... Args>
struct callable_type<R ( & )( Args... )> : callable_type<R( Args... )>
{
};

template<typename T>
using return_type_t = typename callable_type<T>::return_type;

template<typename T>
using argument_types_t = typename callable_type<T>::argument_types;

template<typename... VisitorTypes>
struct Overload : VisitorTypes...
{
  using VisitorTypes::operator()...;
};

template<typename... VisitorTypes>
Overload( VisitorTypes... ) -> Overload<VisitorTypes...>;

template<typename... Ts, std::size_t... Is>
auto get_type_array(parameter_pack<Ts...>, std::index_sequence<Is...>)
-> decltype(Overload { [](std::in_place_index_t<Is>) -> type_identity_t<Ts> { return {}; }... } );

template<typename... Ts>
using type_array_t = decltype(get_type_array(parameter_pack<Ts...>(), std::index_sequence_for<Ts...>()));

template<std::size_t I, typename... Ts>
using type_for_index_t = decltype(type_array_t<Ts...>{}(std::in_place_index<I>));

template<typename... Ts, std::size_t... Is>
auto get_type_set(parameter_pack<Ts...>, std::index_sequence<Is...>)
-> decltype(Overload { [](std::in_place_type_t<Ts>) -> std::integral_constant<size_t, Is> { return {}; }... } );

template<typename... Ts>
using type_set_t = decltype(get_type_set(parameter_pack<Ts...>(), std::index_sequence_for<Ts...>()));

template<typename T, typename... Ts>
inline constexpr std::size_t index_for_type_v = type_set_t<Ts...>{}(std::in_place_type<T>);

template<typename... Ts, std::size_t... Is>
auto get_type_overload_indices(parameter_pack<Ts...>, std::index_sequence<Is...>)
-> decltype(Overload { [](Ts) -> std::integral_constant<size_t, Is> { return {}; }... } );

template<typename... Ts>
using type_overload_indices_t = decltype(get_type_overload_indices(parameter_pack<Ts...>(), std::index_sequence_for<Ts...>()));

template<typename T, typename... Ts>
inline constexpr std::size_t type_index_for_overload_selection_v = decltype(type_overload_indices_t<Ts...>{}(std::declval<T>()))::value;

template<typename... Ts>
concept unique_types = requires ( type_set_t<Ts...> t ) {
  { ( t( std::in_place_type<Ts> ), ... ) };
};

template<typename T, typename... Ts>
concept is_any_of = ( std::same_as<T, Ts> or ... );

template<typename T, typename... Ts>
concept is_convertible_to_any_of = ( std::convertible_to<T, Ts> or ... );

template<typename T>
concept function_pointer = std::invocable<std::remove_cvref_t<T>>
    and std::is_pointer_v<std::remove_cvref_t<T>>;

template<typename T>
concept function_object = std::is_class_v<std::remove_cvref_t<T>>
    and std::invocable<std::remove_cvref_t<T>>;

} // namespace ql
