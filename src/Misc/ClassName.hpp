#ifndef CLASS_NAME_HPP
#define CLASS_NAME_HPP

#include <stddef.h>

template <size_t N>
struct static_string {
    constexpr static_string()  = default;

    constexpr static_string(const char *begin, const char *end) {
        char *out = data;
        while (begin != end)
            *out++ = *begin++;
    }

    constexpr size_t size() const {
        return N;
    }

    constexpr const char *c_str() const {
        return data;
    }

  private:
    char data[N + 1] = {};
};

template <typename T>
struct array_size;

template <typename T, size_t N>
struct array_size<const T (&)[N]> {
    static constexpr size_t value = N;
};

template <typename T>
constexpr auto getTypeNameData() {
    const auto &str = __PRETTY_FUNCTION__;
    constexpr auto prettyStringSize = array_size<decltype(str)>::value;
#ifdef __clang__
    constexpr auto startPosOffset =
        __builtin_strlen("auto getTypeNameData() [T = ");
#else
    constexpr auto startPosOffset =
        __builtin_strlen("constexpr auto getTypeNameData() [with T = ");
#endif
    constexpr auto endPosOffset = __builtin_strlen("] ");

    constexpr auto typeStringSize = prettyStringSize - startPosOffset -
                                    endPosOffset;

    return static_string<typeStringSize> {&str[startPosOffset], &str[prettyStringSize - endPosOffset]};
}

template <typename T>
constexpr auto typeNameStorage = getTypeNameData<T>();

template <typename T>
constexpr const char *getFullTypeName() {
    return typeNameStorage<T>.c_str();
}

template <typename T>
constexpr const char *getUnqualifiedTypeName() {
    const char *str = getFullTypeName<T>();
    while (*str != '\0')
        str++;
    while (*str != ':')
        str--;
    return str + 1;
}

/*
// example:
namespace MyNamespace
{
struct MyType
{};
}

constexpr bool stringsEqual(const char* lhs, const char* rhs)
{
   while (*lhs != '\0' && *rhs != '\0')
   {
      ++lhs;
      ++rhs;
   }
   return *lhs == '\0' && *rhs == '\0';
}

static_assert(stringsEqual(getFullTypeName<MyNamespace::MyType>(), "MyNamespace::MyType"), "");
static_assert(stringsEqual(getUnqualifiedTypeName<MyNamespace::MyType>(), "MyType"), "");

int main()
*/
#endif // CLASS_NAME_HPP

