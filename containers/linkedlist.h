#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <mutex>
#include <initializer_list>
#include "GeneralNode.h"
#include "GeneralIterator.h"
#include "../foreach.h"

template <typename Traits>
class LinkedListNode : public GeneralNode<typename Traits::value_type> {
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using NodePtr    = Node*;

    NodePtr m_pNext = nullptr;

    LinkedListNode() : GeneralNode<value_type>(value_type{}, Ref{}), m_pNext(nullptr) {}
    LinkedListNode(const value_type& value, Ref ref, NodePtr pNext = nullptr)
        : GeneralNode<value_type>(value, ref), m_pNext(pNext) {}
};

template <typename Node>
class LinkedListForwardIterator : public GeneralIterator<LinkedListForwardIterator<Node>, Node> {
public:
    using Parent = GeneralIterator<LinkedListForwardIterator<Node>, Node>;
    using Parent::Parent;

    LinkedListForwardIterator& operator++() {
        if (this->m_ptr)
            this->m_ptr = (this->m_ptr == this->m_pStop) ? nullptr : this->m_ptr->m_pNext;
        return *this;
    }
};

template <typename T, typename _Compare>
struct DefaultTraits {
    using value_type = T;
    using Compare    = _Compare;
};

template <typename T, typename _Compare = std::less<T>>
struct AscendingTraits : public DefaultTraits<T, _Compare> {};

template <typename T, typename _Compare = std::greater<T>>
struct DescendingTraits : public DefaultTraits<T, _Compare> {};

template <typename OrderTrait>
struct LinkedListTraits : public OrderTrait {
    using Node            = LinkedListNode<LinkedListTraits<OrderTrait>>;
    using ForwardIterator = LinkedListForwardIterator<Node>;
};

template <typename T> using LinkedListAscTraits  = LinkedListTraits<AscendingTraits<T>>;
template <typename T> using LinkedListDescTraits = LinkedListTraits<DescendingTraits<T>>;

template <typename Traits>
class LinkedList {
public:
    using value_type      = typename Traits::value_type;
    using Node            = typename Traits::Node;
    using NodePtr         = Node*;
    using ForwardIterator = typename Traits::ForwardIterator;
    using Compare         = typename Traits::Compare;

protected:
    NodePtr            m_pRoot = nullptr;
    NodePtr            m_pTail = nullptr;
    Compare            m_comp{};
    mutable std::mutex m_mutex;

    NodePtr GetRoot() const { return m_pRoot; }

    // No necesita ser virtual: sirve idéntico para las 4 listas
    void openCircular() {
        if (m_pTail) m_pTail->m_pNext = nullptr;
    }

    // Únicos 2 hooks virtuales de toda la jerarquía (Patrón NVI)
    virtual void closeCircular() {}
    virtual void linkPrev(NodePtr /*node*/, NodePtr /*prev*/) {}

    // Inserción recursiva con enlace en O(1) que elimina la necesidad de syncLinks()
    void internalInsert(const value_type& value, Ref ref, NodePtr& rCurr, NodePtr prev = nullptr) {
        if (rCurr == nullptr || m_comp(value, rCurr->getValue())) {
            rCurr = new Node(value, ref, rCurr);
            linkPrev(rCurr, prev);
            if (rCurr->m_pNext)
                linkPrev(rCurr->m_pNext, rCurr);
            else
                m_pTail = rCurr;
            return;
        }
        internalInsert(value, ref, rCurr->m_pNext, rCurr);
    }

public:
    LinkedList() = default;
    LinkedList(const LinkedList& other) { *this = other; }
    LinkedList(std::initializer_list<std::pair<value_type, Ref>> values) {
        for (const auto& v : values) push_back(v.first, v.second);
    }
    virtual ~LinkedList() { clear(); }

    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            clear();
            std::lock_guard<std::mutex> lockOther(other.m_mutex);
            for (const auto& node : other)
                push_back(node.getValue(), node.getRef());
        }
        return *this;
    }

    // Interfaz pública NO-VIRTUAL (NVI)
    void clear() {
        std::scoped_lock lock(m_mutex);
        openCircular();
        NodePtr curr = m_pRoot;
        while (curr) {
            NodePtr next = curr->m_pNext;
            delete curr;
            curr = next;
        }
        m_pRoot = nullptr;
        m_pTail = nullptr;
    }

    void push_back(const value_type& value, Ref ref) {
        std::scoped_lock lock(m_mutex);
        NodePtr new_node = new Node(value, ref, nullptr);
        if (!m_pRoot) {
            m_pRoot = new_node;
        } else {
            m_pTail->m_pNext = new_node;
            linkPrev(new_node, m_pTail);
        }
        m_pTail = new_node;
        closeCircular();
    }

    void insert(const value_type& value, Ref ref) {
        std::scoped_lock lock(m_mutex);
        openCircular();
        internalInsert(value, ref, m_pRoot);
        closeCircular();
    }

    bool empty() const { return m_pRoot == nullptr; }

    ForwardIterator begin() const { return ForwardIterator(m_pRoot, m_pTail); }
    ForwardIterator end()   const { return ForwardIterator(nullptr, m_pTail); }

    template <typename Func, typename... Args>
    decltype(auto) call(Func func, Args&&... args) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return ::call(begin(), end(), std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    decltype(auto) ApplyFunction(Func func, Args&&... args) {
        return call(func, std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    decltype(auto) FirstThat(Func func, Args&&... args) {
        return call(func, std::forward<Args>(args)...);
    }

    template <typename CharT, typename StreamTraits = std::char_traits<CharT>>
    std::basic_ostream<CharT, StreamTraits>& write(std::basic_ostream<CharT, StreamTraits>& os) {
        return os << *this;
    }

    template <typename CharT, typename StreamTraits = std::char_traits<CharT>>
    std::basic_istream<CharT, StreamTraits>& read(std::basic_istream<CharT, StreamTraits>& is) {
        return is >> *this;
    }

    template <typename CharT, typename StreamTraits = std::char_traits<CharT>>
    friend std::basic_ostream<CharT, StreamTraits>& operator<<(
        std::basic_ostream<CharT, StreamTraits>& os, const LinkedList<Traits>& list) {
        std::lock_guard lock(list.m_mutex);
        bool first = true;
        os << static_cast<CharT>('[');
        for (const auto& node : list) {
            if (!first) os << static_cast<CharT>(',');
            os << node;
            first = false;
        }
        return os << static_cast<CharT>(']');
    }

    template <typename CharT, typename StreamTraits = std::char_traits<CharT>>
    friend std::basic_istream<CharT, StreamTraits>& operator>>(
        std::basic_istream<CharT, StreamTraits>& is, LinkedList<Traits>& list) {
        CharT d;
        Node node;
        list.clear();
        is >> d;
        if (is >> d && d != static_cast<CharT>(']')) {
            is.unget();
            while (is >> node >> d) {
                list.push_back(node.getValue(), node.getRef());
                if (d == static_cast<CharT>(']')) break;
            }
        }
        return is;
    }
};

#endif // __LINKEDLIST_H__
