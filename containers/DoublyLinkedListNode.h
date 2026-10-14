#ifndef __DOUBLY_LINKED_LIST_NODE_H__
#define __DOUBLY_LINKED_LIST_NODE_H__

#include "linkedlist.h"

template <typename Traits>
class DoublyLinkedListNode : public LinkedListNode<Traits> {
public:
    using Base = LinkedListNode<Traits>;

    typename Base::NodePtr m_pPrev = nullptr;

    DoublyLinkedListNode() = default;
    DoublyLinkedListNode(const typename Base::value_type& value, Ref ref,
                         typename Base::NodePtr pNext = nullptr,
                         typename Base::NodePtr pPrev = nullptr)
        : Base(value, ref, pNext), m_pPrev(pPrev) {}
};

#endif // __DOUBLY_LINKED_LIST_NODE_H__