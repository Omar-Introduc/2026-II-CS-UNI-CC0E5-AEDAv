#ifndef __CIRCULAR_DOUBLY_LINKED_LIST_H__
#define __CIRCULAR_DOUBLY_LINKED_LIST_H__

#include "doublylinkedlist.h"

template <typename Traits = DoublyLinkedListAscTraits<TX>>
class CircularDoublyLinkedList : public DoublyLinkedList<Traits> {
public:
    using Base = DoublyLinkedList<Traits>;

protected:
    void closeCircular() override {
        if (this->m_pTail && this->m_pRoot) {
            this->m_pTail->m_pNext = this->m_pRoot;
            this->m_pRoot->m_pPrev = this->m_pTail;
        }
    }

public:
    CircularDoublyLinkedList() = default;
    CircularDoublyLinkedList(std::initializer_list<std::pair<typename Base::value_type, Ref>> values) {
        for (const auto& v : values) this->push_back(v.first, v.second);
    }
    CircularDoublyLinkedList(const CircularDoublyLinkedList& other) : Base() { *this = other; }
    CircularDoublyLinkedList& operator=(const CircularDoublyLinkedList& other) = default;
};

#endif // __CIRCULAR_DOUBLY_LINKED_LIST_H__