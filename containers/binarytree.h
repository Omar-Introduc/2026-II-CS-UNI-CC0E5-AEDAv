#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <iostream>
#include <vector>
#include <utility>
#include <initializer_list>
#include <mutex>
#include "GeneralIterator.h"
#include "GeneralNode.h"
#include "../foreach.h"
#include "../types.h"

template <typename T>
class BinaryTreeNode : public GeneralNode<T> {
public:
    using NodePtr = BinaryTreeNode<T>*;
    NodePtr m_pChild[2] = {nullptr, nullptr}; // 0 = left, 1 = right

    BinaryTreeNode() = default;
    BinaryTreeNode(const T& value, Ref ref = Ref{}) : GeneralNode<T>(value, ref) {}
};

// Iterador único bidireccional parametrizado por dirección (0 = In-Order Asc, 1 = In-Order Desc)
template <typename Node, size_t Dir = 0>
class BinaryTreeIterator : public GeneralIterator<BinaryTreeIterator<Node, Dir>, Node> {
    std::vector<Node*> m_stack;
    void push_branch(Node* curr) {
        for (; curr; curr = curr->m_pChild[Dir]) m_stack.push_back(curr);
    }
public:
    using Parent = GeneralIterator<BinaryTreeIterator<Node, Dir>, Node>;

    BinaryTreeIterator(Node* root = nullptr) : Parent(nullptr) {
        push_branch(root);
        this->m_ptr = m_stack.empty() ? nullptr : m_stack.back();
    }

    BinaryTreeIterator& operator++() {
        if (!m_stack.empty()) {
            Node* node = m_stack.back();
            m_stack.pop_back();
            push_branch(node->m_pChild[1 - Dir]);
        }
        this->m_ptr = m_stack.empty() ? nullptr : m_stack.back();
        return *this;
    }
};

template <typename Node> using BinaryTreeForwardIterator  = BinaryTreeIterator<Node, 0>;
template <typename Node> using BinaryTreeBackwardIterator = BinaryTreeIterator<Node, 1>;

template <typename OrderTrait>
struct BinaryTreeTraits : public OrderTrait {
    using Node             = BinaryTreeNode<typename OrderTrait::value_type>;
    using ForwardIterator  = BinaryTreeForwardIterator<Node>;
    using BackwardIterator = BinaryTreeBackwardIterator<Node>;
};

template <typename T> using BinaryTreeAscTraits  = BinaryTreeTraits<AscendingTraits<T>>;
template <typename T> using BinaryTreeDescTraits = BinaryTreeTraits<DescendingTraits<T>>;

template <typename Traits>
class BinaryTree : public ContainerBase<BinaryTree<Traits>, Traits> {
public:
    using Base             = ContainerBase<BinaryTree<Traits>, Traits>;
    using value_type       = typename Traits::value_type;
    using Node             = typename Traits::Node;
    using NodePtr          = Node*;
    using Compare          = typename Traits::Compare;
    using ForwardIterator  = typename Traits::ForwardIterator;
    using BackwardIterator = typename Traits::BackwardIterator;

private:
    NodePtr m_root = nullptr;
    size_t  m_size = 0;
    Compare m_comp{};

    void internal_insert(NodePtr& node, const value_type& value, Ref ref) {
        if (!node) { node = new Node(value, ref); ++m_size; return; }
        internal_insert(node->m_pChild[m_comp(node->getValue(), value) ? 1 : 0], value, ref);
    }

    NodePtr clone_tree(NodePtr node) {
        if (!node) return nullptr;
        NodePtr copy = new Node(node->getValue(), node->getRef());
        copy->m_pChild[0] = clone_tree(node->m_pChild[0]);
        copy->m_pChild[1] = clone_tree(node->m_pChild[1]);
        return copy;
    }

public:
    BinaryTree() = default;
    BinaryTree(std::initializer_list<std::pair<value_type, Ref>> values) {
        for (const auto& [v, r] : values) push_back(v, r);
    }
    virtual ~BinaryTree() { clear(); }

    BinaryTree(const BinaryTree& other) : Base() { *this = other; }
    BinaryTree& operator=(const BinaryTree& other) {
        if (this != &other) {
            clear();
            std::scoped_lock lock(this->m_mutex, other.m_mutex);
            m_root = clone_tree(other.m_root);
            m_size = other.m_size;
        }
        return *this;
    }

    BinaryTree(BinaryTree&& other) noexcept : Base() { *this = std::move(other); }
    BinaryTree& operator=(BinaryTree&& other) noexcept {
        if (this != &other) {
            clear();
            std::scoped_lock lock(this->m_mutex, other.m_mutex);
            m_root = std::exchange(other.m_root, nullptr);
            m_size = std::exchange(other.m_size, 0);
        }
        return *this;
    }

    void insert(const value_type& value, Ref ref = Ref{}) {
        std::scoped_lock lock(this->m_mutex);
        internal_insert(m_root, value, ref);
    }
    void push_back(const value_type& value, Ref ref = Ref{}) { insert(value, ref); }

    ForwardIterator  begin()  const { return ForwardIterator(m_root);  }
    ForwardIterator  end()    const { return ForwardIterator(nullptr); }
    BackwardIterator rbegin() const { return BackwardIterator(m_root); }
    BackwardIterator rend()   const { return BackwardIterator(nullptr);}

    size_t size()  const { std::scoped_lock lock(this->m_mutex); return m_size;      }
    bool   empty() const { std::scoped_lock lock(this->m_mutex); return m_size == 0; }

    void clear() {
        std::scoped_lock lock(this->m_mutex);
        while (m_root) {
            if (m_root->m_pChild[0]) {
                NodePtr left = m_root->m_pChild[0];
                m_root->m_pChild[0] = left->m_pChild[1];
                left->m_pChild[1]   = m_root;
                m_root              = left;
            } else {
                delete std::exchange(m_root, m_root->m_pChild[1]);
            }
        }
        m_size = 0;
    }
};

#endif // __BINARYTREE_H__
