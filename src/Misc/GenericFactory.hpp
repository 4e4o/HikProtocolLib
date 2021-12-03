#ifndef GENERIC_FACTORY_HPP
#define GENERIC_FACTORY_HPP

#include <map>
#include <string>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <concepts>
#include <any>

#include "ClassName.hpp"
#include "LambdaTraits.hpp"

template<class Base>
class GenericFactory {
  public:
    // Для дефолтных типов
    template<typename T, typename... CreatorArgs>
    void registerDefaultType() {
        return registerType<T, T, CreatorArgs...>();
    }

    // для простых типов без доп. параметров в конструкторах
    template<typename T, typename Derived, typename... CreatorArgs>
    requires std::derived_from<Derived, T>
    void registerType() {
        typedef typename MakeCreator<CreatorArgs...>::TCreator TCreator;

        TCreator creator{[] (auto &&... args) -> T * {
                return new Derived(args...);
            }
        };

        insert <T>(std::move(creator));
    }

    // для лямбд
    template<LambdaPtrResultDerived<Base> Callable>
    void registerType(Callable &&c) {
        typedef typename
        MakeCreator<typename LambdaTraits<Callable>::args_tuple>::TCreator TCreator;
        TCreator creator{std::move(c)};
        using RetType = typename
                        std::remove_pointer<typename LambdaTraits<Callable>::result_type>::type;

        insert <RetType>(std::move(creator));
    }

    template<class T, typename... CreatorArgs>
    requires std::derived_from<T, Base>
    T *create(CreatorArgs&&... args) {
        constexpr auto className = getFullTypeName<T>();

        if (m_types.find(className) == m_types.end())
            throw std::runtime_error("Type does not registered");

        typedef typename MakeCreator<CreatorArgs...>::TCreator TCreator;
        std::any &c = m_types[className];
        TCreator &creator = std::any_cast<TCreator &>(c);
        return static_cast<T *>(creator(std::forward<CreatorArgs>(args)...));
    }

  private:
    template<typename... Args>
    struct MakeCreator {
        typedef std::function<Base*(Args...)> TCreator;
    };

    template<typename... Ts>
    struct MakeCreator<std::tuple<Ts...>> {
        typedef std::function<Base*(Ts...)> TCreator;
    };

    template<class KeyType, class Creator>
    void insert(Creator &&creator) {
        constexpr auto className = getFullTypeName<KeyType>();
        m_types.erase(className);
        m_types.emplace(className, std::move(creator));
    }

    std::map<std::string, std::any> m_types;
};

#endif // GENERIC_FACTORY_HPP
