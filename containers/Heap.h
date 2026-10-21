#ifndef __HEAP_H__
#define __HEAP_H__

#include <stdexcept>
#include <utility>
#include <type_traits>
#include "vector.h"

template <typename OrderTrait>
struct HeapTraits : public OrderTrait {
    using Node             = GeneralNode<typename OrderTrait::value_type>;
    using ForwardIterator  = VectorForwardIterator<Node>;
    using BackwardIterator = VectorBackwardIterator<Node>;
};

template <typename T> using MinHeapTraits = HeapTraits<AscendingTraits<T>>;
template <typename T> using MaxHeapTraits = HeapTraits<DescendingTraits<T>>;

template <typename Traits>
class Heap : public ContainerBase<Heap<Traits>, Traits> {
public:
    using Base             = ContainerBase<Heap<Traits>, Traits>;
    using value_type       = typename Traits::value_type;
    using Node             = typename Traits::Node;
    using Compare          = typename Traits::Compare;

protected:
    Vector<Traits> m_data;
    Compare        m_comp{};

    void HeapifyUp(size_t i) {
        while (i > 0 && m_comp(m_data[i].getValue(), m_data[(i - 1) / 2].getValue())) {
            std::swap(m_data[i], m_data[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }

    void HeapifyDown(size_t i) {
        for (size_t n = m_data.size(); ;) {
            size_t left = 2 * i + 1, right = 2 * i + 2, cand = i;
            if (left < n  && m_comp(m_data[left].getValue(),  m_data[cand].getValue())) cand = left;
            if (right < n && m_comp(m_data[right].getValue(), m_data[cand].getValue())) cand = right;
            if (cand == i) break;
            std::swap(m_data[i], m_data[cand]);
            i = cand;
        }
    }

public:
    Heap() = default;
    Heap(std::initializer_list<std::pair<value_type, Ref>> values) {
        for (const auto& [v, r] : values) push(v, r);
    }

    decltype(auto) begin()  const { return m_data.begin();  }
    decltype(auto) end()    const { return m_data.end();    }
    decltype(auto) rbegin() const { return m_data.rbegin(); }
    decltype(auto) rend()   const { return m_data.rend();   }

    void push(const value_type& value, Ref ref = Ref{}) {
        std::scoped_lock lock(this->m_mutex);
        m_data.push_back(value, ref);
        HeapifyUp(m_data.size() - 1);
    }
    void push_back(const value_type& value, Ref ref = Ref{}) { push(value, ref); }

    Node pop() {
        std::scoped_lock lock(this->m_mutex);
        if (m_data.empty()) throw std::underflow_error("Heap::pop(): el monticulo esta vacio");
        Node root = m_data[0];
        std::swap(m_data[0], m_data[m_data.size() - 1]);
        m_data.pop_back();
        if (!m_data.empty()) HeapifyDown(0);
        return root;
    }

    Node top() const {
        std::scoped_lock lock(this->m_mutex);
        if (m_data.empty()) throw std::underflow_error("Heap::top(): el monticulo esta vacio");
        return m_data[0];
    }

    size_t size()  const { std::scoped_lock lock(this->m_mutex); return m_data.size();  }
    bool   empty() const { std::scoped_lock lock(this->m_mutex); return m_data.empty(); }
    void   clear()       { std::scoped_lock lock(this->m_mutex); m_data.clear();        }
};

// Alias directos estandar (sin parches SFINAE/requires)
// El alias recibe simplemente el tipo de dato T y lo enlaza con su Trait correspondiente:
template <typename T> using MinHeap = Heap<MinHeapTraits<T>>;
template <typename T> using MaxHeap = Heap<MaxHeapTraits<T>>;

#endif // __HEAP_H__
