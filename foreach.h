#ifndef __FOREACH_H__
#define __FOREACH_H__
#include <iostream>
#include <mutex>
#include <utility> // std::forward
#include <type_traits> // std::is_void_v, std::invoke_result_t
#include <stdexcept> // std::runtime_error
#include <concepts>
using namespace std;

template <typename Iterator, typename Func, typename... Args>
decltype(auto) call(Iterator begin, Iterator end, Func func, Args&&... args) {
    if constexpr(is_void_v<invoke_result_t<Func, decltype(*begin), Args...>>) {
        for (auto iter = begin; iter != end; ++iter)
            func(*iter, std::forward<Args>(args)...);
    } else {
        for (auto iter = begin; iter != end; ++iter) {
            auto ret = func(*iter, std::forward<Args>(args)...);
            if(ret) return *iter;
        }
        throw std::runtime_error("call(): ningun elemento satisface el predicado");
    }
}

// Variadic template to allow passing additional arguments to the function
// template <typename Container, typename Func, typename... Args>
// Iterator Level #1
// void ApplyFunction(Container &container, Func func, Args... args) {
//     for (size_t i = 0; i < container.size(); ++i) {
//         func(container[i], args...);
//     }
// }

// Iterator Level #2
// template <typename Container, typename Func, typename... Args>
// void ApplyFunction(Container &container, Func func, Args... args) {
//     for (auto iter = container.begin(); iter != container.end(); ++iter) {
//         func(*iter, args...);
//     }
// }

// Iterator Level #3
template <typename Iterator, typename Func, typename... Args>
void ApplyFunction(Iterator begin, Iterator end, Func func, Args&&... args) {
    // for (auto iter = begin; iter != end; ++iter)
    //     func(*iter, std::forward<Args>(args)...);
    ::call(begin, end, func, std::forward<Args>(args)...);
}

template <typename Iterator, typename Func, typename... Args>
decltype(auto) FirstThat(Iterator begin, Iterator end, Func func, Args&&... args) {
    // for (auto iter = begin; iter != end; ++iter)
    //     func(*iter, std::forward<Args>(args)...);
    ::call(begin, end, func, std::forward<Args>(args)...);
}

// // Iterator Level #3
// template <typename Container, typename Func, typename... Args>
// void ApplyFunction(Container &container, Func func, Args... args) {
//     ::ApplyFunction(container.begin(), container.end(), func, args...);
// }

// Iterator Level #4
template <typename Container, typename Func, typename... Args>
void ApplyFunction(Container &container, Func func, Args&&... args) {
    for (auto &v : container)
        func(v, std::forward<Args>(args)...);
}

// 3. Base CRTP 

template <typename Derived, typename Traits>
class ContainerBase {
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using NodePtr    = Node*;

    Derived&       derived()       { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

protected:
    mutable std::mutex m_mutex;

public:
    template <typename Func, typename... Args>
    decltype(auto) call(Func func, Args&&... args) {
        std::scoped_lock lock(m_mutex);
        return ::call(derived().begin(), derived().end(),
                      std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    decltype(auto) rcall(Func func, Args&&... args)
    requires requires(Derived& d) { d.rbegin(); d.rend(); } {
        std::scoped_lock lock(m_mutex);
        return ::call(derived().rbegin(), derived().rend(),
                      std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    decltype(auto) ApplyFunction(Func func, Args&&... args) {
        return call(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    decltype(auto) FirstThat(Func func, Args&&... args) {
        return call(std::forward<Func>(func), std::forward<Args>(args)...);
    }

    std::ostream& write(std::ostream& os) const { return os << *this; }
    std::istream& read(std::istream& is)        { return is >> const_cast<ContainerBase&>(*this); }
};

// 4. Operadores << y >> globales
template <typename Derived, typename Traits>
std::ostream& operator<<(std::ostream& os, const ContainerBase<Derived, Traits>& container) {
    os << "[";
    bool first = true;
    for (const auto& node : container.derived()) {
        if (!first) os << ",";
        os << node;
        first = false;
    }
    return os << "]";
}

template <typename Derived, typename Traits>
std::istream& operator>>(std::istream& is, ContainerBase<Derived, Traits>& container) {
    using Node = typename Traits::Node;
    char d;
    Node node;
    container.derived().clear();
    if (!(is >> d) || d != '[') return is;
    if (is >> d && d != ']') {
        is.unget();
        while (is >> node >> d) {
            container.derived().push_back(node.getValue(), node.getRef());
            if (d == ']') break;
        }
    }
    return is;
}

#endif // __FOREACH_H__