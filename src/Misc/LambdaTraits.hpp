#ifndef LAMBDA_TRAITS_HPP
#define LAMBDA_TRAITS_HPP

#include <tuple>
#include <concepts>

#include <stddef.h>

// from https://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda

template <typename T>
struct LambdaTraits
: public LambdaTraits<decltype(&T::operator())> { };
// For generic types, directly use the result of the signature of its 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct LambdaTraits<ReturnType(ClassType::*)(Args...) const> {
    // we specialize for pointers to member function
    enum { arity = sizeof...(Args) };
    // arity is the number of arguments.

    typedef ReturnType result_type;
    typedef std::tuple<Args...> args_tuple;

    template <size_t i>
    struct arg {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };
};

template <class Lambda, class Base,
          typename LambdaResult =
          typename std::remove_pointer<typename LambdaTraits<Lambda>::result_type>::type>
concept LambdaPtrResultDerived = std::is_base_of<Base, LambdaResult>::value;

/*
 * // test code below:
int main()
{
    auto lambda = [](int i) { return long(i*10); };

    typedef function_traits<decltype(lambda)> traits;

    static_assert(std::is_same<long, traits::result_type>::value, "err");
    static_assert(std::is_same<int, traits::arg<0>::type>::value, "err");

    return 0;
} */

#endif // LAMBDA_TRAITS_HPP
