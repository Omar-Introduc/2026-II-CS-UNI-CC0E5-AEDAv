#ifndef __FOREACH_H__
#define __FOREACH_H__
#include <utility> // std::forward
#include <type_traits> // std::is_void_v, std::invoke_result_t
#include <stdexcept> // std::runtime_error
#include <mutex>
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
requires requires(Container& c) { c.begin(); c.end(); }
void ApplyFunction(Container &container, Func func, Args&&... args) {
    container.call(std::forward<Func>(func), std::forward<Args>(args)...);
}

template <typename Container, typename Func, typename... Args>
requires requires(Container& c) { c.begin(); c.end(); }
decltype(auto) FirstThat(Container& container, Func func, Args&&... args) {
    return container.FirstThat(std::forward<Func>(func), std::forward<Args>(args)...);
}

template <typename Derived, typename Traits>
class ContainerBase {
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using NodePtr    = Node*;

protected:
    mutable std::mutex m_mutex;

    Derived&       derived()       { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }

public:
    template <typename Func, typename... Args>
    decltype(auto) call(Func func, Args&&... args) {
        std::scoped_lock lock(m_mutex);
        return ::call(derived().begin(), derived().end(),
                      std::forward<Func>(func), std::forward<Args>(args)...);
    }

    // rcall condicional: solo participa si Derived tiene rbegin() y rend() (Vector y Heap)
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

    std::ostream& write(std::ostream& os) const { return os << derived(); }
    std::istream& read(std::istream& is)        { return is >> derived(); }
};

template <typename Container>
requires requires(const Container& c) {
    c.begin();
    c.end();
    typename Container::Node;
} && (!std::is_same_v<std::remove_cvref_t<Container>, std::string>)
  && (!std::is_same_v<std::remove_cvref_t<Container>, std::string_view>)
std::ostream& operator<<(std::ostream& os, const Container& container) {
    os << "[";
    bool first = true;
    for (const auto& node : container) {
        if (!first) os << ",";
        os << node;
        first = false;
    }
    return os << "]";
}

template <typename Container>
requires requires(Container& c) {
    c.clear();
    typename Container::Node;
}
std::istream& operator>>(std::istream& is, Container& container) {
    using Node  = typename Container::Node;
    using Delim = typename Node::Delim;
    Delim d;
    Node node;
    container.clear();
    if (!(is >> d) || d != '[') return is;
    if (is >> d && d != ']') {
        is.unget();
        while (is >> node >> d) {
            container.push_back(node.getValue(), node.getRef());
            if (d == ']') break;
        }
    }
    return is;
}
#endif // __FOREACH_H__