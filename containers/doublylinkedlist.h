#ifndef __DOUBLY_LINKED_LIST_H__
#define __DOUBLY_LINKED_LIST_H__

#include "DoublyLinkedListNode.h"
#include "GeneralIterator.h"
#include "linkedlist.h"

template <typename Node>
class DoublyLinkedListBackwardIterator
    : public GeneralIterator<DoublyLinkedListBackwardIterator<Node>, Node> {
public:
    using Parent = GeneralIterator<DoublyLinkedListBackwardIterator<Node>, Node>;
    using Parent::Parent;

    DoublyLinkedListBackwardIterator& operator++() {
        if (this->m_ptr)
            this->m_ptr = (this->m_ptr == this->m_pStop) ? nullptr : this->m_ptr->m_pPrev;
        return *this;
    }
};

template <typename OrderTrait>
struct DoublyLinkedListTraits : public OrderTrait {
    using Node             = DoublyLinkedListNode<DoublyLinkedListTraits<OrderTrait>>;
    using ForwardIterator  = LinkedListForwardIterator<Node>;
    using BackwardIterator = DoublyLinkedListBackwardIterator<Node>;
};

template <typename T> using DoublyLinkedListAscTraits  = DoublyLinkedListTraits<AscendingTraits<T>>;
template <typename T> using DoublyLinkedListDescTraits = DoublyLinkedListTraits<DescendingTraits<T>>;

template <typename Traits = DoublyLinkedListAscTraits<TX>>
class DoublyLinkedList : public LinkedList<Traits> {
public:
    using Base             = LinkedList<Traits>;
    using BackwardIterator = typename Traits::BackwardIterator;

protected:
    void linkPrev(typename Base::NodePtr node, typename Base::NodePtr prev) override {
        node->m_pPrev = prev;
    }

public:
    DoublyLinkedList() = default;
    DoublyLinkedList(std::initializer_list<std::pair<typename Base::value_type, Ref>> values) {
        for (const auto& v : values) this->push_back(v.first, v.second);
    }
    DoublyLinkedList(const DoublyLinkedList& other) : Base() { *this = other; }
    DoublyLinkedList& operator=(const DoublyLinkedList& other) = default;

    BackwardIterator rbegin() const { return BackwardIterator(this->m_pTail, this->m_pRoot); }
    BackwardIterator rend()   const { return BackwardIterator(nullptr, this->m_pRoot); }

    template <typename Func, typename... Args>
    decltype(auto) rcall(Func func, Args&&... args) {
        std::lock_guard<std::mutex> lock(this->m_mutex);
        return ::call(rbegin(), rend(), std::forward<Func>(func), std::forward<Args>(args)...);
    }
};

#endif // __DOUBLY_LINKED_LIST_H__