#ifndef __HEAP_H__
#define __HEAP_H__

#include <type_traits>
#include <stdexcept>
#include <utility>
#include "vector.h"

template <typename OrderTrait>
using HeapTraits = VectorTraits<OrderTrait>;

template <typename T> using MinHeapTraits = HeapTraits<AscendingTraits<T>>;
template <typename T> using MaxHeapTraits = HeapTraits<DescendingTraits<T>>;

template <typename Traits>
class Heap : public ContainerBase<Heap<Traits>, Traits>{
    
public:
    using Base             = ContainerBase<Heap<Traits>, Traits>;
    using value_type        = typename Traits::value_type;
    using Node              = typename Traits::Node;
    using NodePtr           = Node *;
    using Compare           = typename Traits::Compare;
    using ForwardIterator  = typename Traits::ForwardIterator;
    using BackwardIterator = typename Traits::BackwardIterator;
private:
    Vector<Traits> m_data;
    Compare        m_comp{};

    void HeapifyUp(size_t index) {
        while (index > 0) {
            size_t parentIndex = (index - 1) / 2;
            if (m_comp(m_data[index].getValue(), m_data[parentIndex].getValue())) {
                std::swap(m_data[index], m_data[parentIndex]);
                index = parentIndex;
            } else {
                break;
            }
        }
    }

        void HeapifyDown(size_t index) {
        size_t n = m_data.size();
        while (true) {
            size_t left      = 2 * index + 1;
            size_t right     = 2 * index + 2;
            size_t candidate = index;

            if (left < n && m_comp(m_data[left].getValue(), m_data[candidate].getValue()))
                candidate = left;
            if (right < n && m_comp(m_data[right].getValue(), m_data[candidate].getValue()))
                candidate = right;

            if (candidate == index) break;
            std::swap(m_data[index], m_data[candidate]);
            index = candidate;
        }
    }

public:
    Heap() = default;

    Heap(std::initializer_list<std::pair<value_type, Ref>> values) {
        for (const auto& v : values)
            push(v.first, v.second);
    }

    decltype(auto) begin()  const { return m_data.begin();  }
    decltype(auto) end()    const { return m_data.end();    }
    decltype(auto) rbegin() const { return m_data.rbegin(); }
    decltype(auto) rend()   const { return m_data.rend();   }

    void push(const value_type &value, Ref ref = Ref{}) {
        std::scoped_lock lock(this->m_mutex);
        m_data.push_back(value, ref);
        HeapifyUp(m_data.size() - 1);
    }

    void push_back(const value_type& value, Ref ref = Ref{}) {
        push(value, ref);
    }

    Node pop() {
        std::scoped_lock lock(this->m_mutex);
        if (m_data.empty())
            throw std::underflow_error("Heap::pop(): el monticulo esta vacio");
        Node root = m_data[0];
        std::swap(m_data[0], m_data[m_data.size() - 1]);
        m_data.pop_back();
        if (!m_data.empty())        
            HeapifyDown(0);  
        return root;
    }

    Node top() const {
        std::scoped_lock lock(this->m_mutex);
        if (m_data.empty())
            throw std::underflow_error("Heap::top(): el monticulo esta vacio");
        return m_data[0];
    }

    size_t size() const {
        std::scoped_lock lock(this->m_mutex);
        return m_data.size();
    }

    bool empty() const {
        std::scoped_lock lock(this->m_mutex);
        return m_data.empty();
    }

    void clear() {
        std::scoped_lock lock(this->m_mutex);
        m_data.clear();
    }
};

template <typename TOrTraits>
using ResolveMinTraits = std::conditional_t<
    requires { typename TOrTraits::Compare; },
    TOrTraits,
    MinHeapTraits<TOrTraits>
>;

template <typename TOrTraits>
using ResolveMaxTraits = std::conditional_t<
    requires { typename TOrTraits::Compare; },
    TOrTraits,
    MaxHeapTraits<TOrTraits>
>;

template <typename Traits=TX>
class MinHeap : public Heap<ResolveMinTraits<Traits>> {
public:
    using Base = Heap<ResolveMinTraits<Traits>>;
    using Base::Base;
};

template <typename Traits=TX>
class MaxHeap : public Heap<ResolveMaxTraits<Traits>> {
public:
    using Base = Heap<ResolveMaxTraits<Traits>>;
    using Base::Base;    
};

#endif // __HEAP_H__