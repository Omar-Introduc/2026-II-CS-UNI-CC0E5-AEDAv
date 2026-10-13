#ifndef __CIRCULAR_LINKED_LIST_H__
#define __CIRCULAR_LINKED_LIST_H__

#include "linkedlist.h"

template <typename Traits = LinkedListAscTraits<TX>>
class CircularLinkedList : public LinkedList<Traits> {
public:
    using Base = LinkedList<Traits>;

protected:
    void closeCircular() override {
        if (this->m_pTail)
            this->m_pTail->m_pNext = this->m_pRoot;
    }

public:
    CircularLinkedList() = default;
    CircularLinkedList(std::initializer_list<std::pair<typename Base::value_type, Ref>> values) {
        for (const auto& v : values) this->push_back(v.first, v.second);
    }
    CircularLinkedList(const CircularLinkedList& other) : Base() { *this = other; }
    CircularLinkedList& operator=(const CircularLinkedList& other) = default;
};

#endif // __CIRCULAR_LINKED_LIST_H__