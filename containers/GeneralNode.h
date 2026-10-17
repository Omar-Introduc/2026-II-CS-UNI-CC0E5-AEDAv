#ifndef __GENERAL_NODE_H__
#define __GENERAL_NODE_H__
#include <iostream>
#include "../types.h" // Ref

template <typename T, typename _Compare>
struct DefaultTraits {
    using value_type = T;
    using Compare    = _Compare;
};

template <typename T, typename _Compare = std::less<T>>
struct AscendingTraits : public DefaultTraits<T, _Compare> {};

template <typename T, typename _Compare = std::greater<T>>
struct DescendingTraits : public DefaultTraits<T, _Compare> {};

template <typename T>
struct GeneralNode{
private:
    T   m_value{};
    Ref m_ref{};      // Reference to the value

public:
    using value_type = T;
    using Delim      = std::istream::char_type;

    GeneralNode() = default; // requerido por resize(): new Node[new_cap]
    GeneralNode(const T& value, Ref ref = Ref{}) : m_value(value), m_ref(ref) {}
    T    getValue() const { return m_value; }
    Ref  getRef()   const { return m_ref;   }
    T&   value()          { return m_value; } // acceso mutable para ApplyFunction

    friend std::ostream &operator <<(std::ostream &os, const GeneralNode<T> &node) {
        return os << "(" << node.getValue() << "," << node.getRef() << ")";
    }

    friend std::istream &operator >>(std::istream &is, GeneralNode<T> &node){
        Delim d;
        return is >> d >> node.m_value >> d >> node.m_ref >> d;
    }
};

#endif // __GENERAL_NODE_H__