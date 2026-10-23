#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include "foreach.h"
#include "containers/vector.h"
#include "containers/linkedlist.h"
#include "containers/circularlinkedlist.h"
#include "containers/doublylinkedlist.h"
#include "containers/circulardoublylinkedlist.h"
#include "containers/Heap.h"
#include "containers/binarytree.h"
#include "Demos.h"

using namespace std;

const int NThreads = 5;

void AddOne(GeneralNode<TX>& node) { node.value() += 1; }
template <typename T> void AddX(GeneralNode<T>& node, T x) { node.value() += x; }
template <typename Node, typename T> bool IsGreaterThan(Node& node, T threshold) { return node.getValue() > threshold; }
template <typename Node> void PrintNode(Node& node, ostream& os) { os << node << " "; }

template <typename Container>
void RunContainerTestSuite(const string& title,
                           const string& filename,
                           const vector<pair<typename Container::value_type, Ref>>& initialValues,
                           const vector<pair<typename Container::value_type, Ref>>& extraInserts = {}) {
    using Node = typename Container::Node;
    cout << "\n\n\033[44;37m  [ " << title << " ]  \033[0m\n";

    Container c;
    for (const auto& [v, r] : initialValues) c.push_back(v, r);

    cout << "  write():            "; c.write(cout) << endl;
    { ofstream of(filename); c.write(of) << '\n'; }
    cout << "  cout << c:          " << c << endl;

    if constexpr (requires { c.insert(typename Container::value_type{}, Ref{}); }) {
        for (const auto& [v, r] : extraInserts) c.insert(v, r);
        if (!extraInserts.empty()) cout << "  Inserts ordenados:  " << c << endl;
    }

    cout << "  Forward traversal:  [ "; c.call(PrintNode<Node>, cout); cout << "]\n";
    if constexpr (requires { c.rbegin(); c.rend(); }) {
        cout << "  Backward traversal: [ "; c.rcall(PrintNode<Node>, cout); cout << "]\n";
    }

    if (!c.empty()) {
        auto threshold = (*c.begin()).getValue();
        try {
            auto res = c.FirstThat(IsGreaterThan<Node, decltype(threshold)>, threshold);
            cout << "  \033[43;30m FirstThat (> " << threshold << "): " << res << " \033[0m\n";
        } catch (const exception& e) {
            cout << "  FirstThat (> " << threshold << "): (ninguno cumple)\n";
        }
    }

    Container c_read;
    { ifstream in(filename); in >> c_read; }
    cout << "  Persistencia file:  " << c_read << endl;

    if constexpr (requires { c_read.pop(); }) {
        cout << "  \033[46;30m Extraccion ordenada con pop(): [ ";
        while (!c_read.empty()) cout << c_read.pop() << " ";
        cout << "] \033[0m\n";
    }

    c.clear();
    cout << "  Tras clear():       empty = " << boolalpha << c.empty() << " " << c << endl;
    if (!initialValues.empty()) {
        c.push_back(initialValues.front().first, initialValues.front().second);
        cout << "  Post-clear push:    " << c << endl;
    }
}

void DemoVector() {
    RunContainerTestSuite<Vector<VectorAscTraits<TX>>>(
        "DEMO VECTOR", "vector.txt", {{0, 10}, {1, 11}, {2, 12}, {3, 13}, {4, 14}}, {{5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}});
    RunContainerTestSuite<Vector<VectorAscTraits<string>>>(
        "DEMO VECTOR STRINGS", "vector_str.txt", {{"Hello", 1}, {"World", 2}});
}

void DemoRaceCondition() {
    cout << "\n\n\033[44;37m  [ DEMO RACE CONDITION ]  \033[0m\n";
    const long long N = 200000;
    Vector<VectorAscTraits<TX>> vec;
    {
        vector<jthread> workers;
        for (int t = 0; t < NThreads; ++t)
            workers.emplace_back([&vec, t]() {
                for (long long i = t; i < N; i += NThreads)
                    vec.push_back(static_cast<TX>(i), static_cast<Ref>(i));
            });
    } // auto-join RAII al cerrar el bloque

    long long expectedSum = N * (N - 1) / 2, actualSum = 0;
    vec.call([&actualSum](const auto& node) { actualSum += node.getValue(); });
    cout << "  Concurrencia (" << NThreads << " hilos): N=" << N << ", size=" << vec.size() << "\n  ";
    if (actualSum == expectedSum) {
        cout << "\033[42;30m [PASS] Sin Race Condition (suma=" << actualSum << ") \033[0m\n";
    } else {
        cout << "\033[41;37m [FAIL] Race Condition detectada \033[0m\n";
    }
}

void DemoLinkedList() {
    RunContainerTestSuite<LinkedList<LinkedListAscTraits<TX>>>(
        "DEMO LINKED LIST (ASC)", "linkedlist.txt", {{5, 15}, {1, 11}, {8, 18}, {3, 13}}, {{4, 12}, {11, 20}, {7, 6}});
    RunContainerTestSuite<LinkedList<LinkedListDescTraits<TX>>>(
        "DEMO LINKED LIST (DESC)", "linkedlist_desc.txt", {}, {{10, 1}, {5, 2}, {20, 3}});
}

void DemoCircularLinkedList() {
    RunContainerTestSuite<CircularLinkedList<LinkedListAscTraits<TX>>>(
        "DEMO CIRCULAR LINKED LIST (LC)", "circularlist.txt", {{10, 1}, {20, 2}, {30, 3}}, {{25, 25}, {5, 5}, {40, 40}});
}

void DemoDoublyLinkedList() {
    RunContainerTestSuite<DoublyLinkedList<DoublyLinkedListAscTraits<TX>>>(
        "DEMO DOUBLY LINKED LIST (LDE)", "doublylist.txt", {{10, 1}, {20, 2}, {30, 3}}, {{25, 25}, {5, 5}, {50, 50}});
}

void DemoCircularDoublyLinkedList() {
    RunContainerTestSuite<CircularDoublyLinkedList<DoublyLinkedListAscTraits<TX>>>(
        "DEMO CIRCULAR DOUBLY LINKED LIST (LDEC)", "circulardoublylist.txt", {{100, 10}, {200, 20}, {300, 30}}, {{250, 25}, {50, 5}, {400, 40}});
}

void DemoHeap() {
    vector<pair<TX, Ref>> values = {{50, 5}, {20, 2}, {60, 6}, {10, 1}, {30, 3}, {5, 0}, {40, 4}};
    RunContainerTestSuite<MinHeap<TX>>("DEMO MINHEAP", "minheap.txt", values);
    RunContainerTestSuite<MaxHeap<TX>>("DEMO MAXHEAP", "maxheap.txt", values);
}

void DemoBinaryTree() {
    RunContainerTestSuite<BinaryTree<BinaryTreeAscTraits<TX>>>(
        "DEMO BINARY SEARCH TREE (BST)", "binarytree.txt",
        {{50, 5}, {30, 3}, {70, 7}, {20, 2}, {40, 4}, {60, 6}, {80, 8}});
}
