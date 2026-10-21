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
    cout << "\n======================================================\n  " << title
         << "\n======================================================\n";

    Container c;
    for (const auto& [v, r] : initialValues) c.push_back(v, r);

    cout << "Container using write(): "; c.write(cout) << endl;
    { ofstream of(filename); c.write(of) << '\n'; }
    cout << "Container using cout directly: " << c << endl;

    if constexpr (requires { c.insert(typename Container::value_type{}, Ref{}); }) {
        for (const auto& [v, r] : extraInserts) c.insert(v, r);
        if (!extraInserts.empty()) cout << "Tras inserts ordenados: " << c << endl;
    }

    cout << "Forward traversal:  ["; c.call(PrintNode<Node>, cout); cout << "]\n";
    if constexpr (requires { c.rbegin(); c.rend(); }) {
        cout << "Backward traversal: ["; c.rcall(PrintNode<Node>, cout); cout << "]\n";
    }

    if (!c.empty()) {
        auto threshold = (*c.begin()).getValue();
        try {
            cout << "FirstThat (primer valor > " << threshold << "): "
                 << c.FirstThat(IsGreaterThan<Node, decltype(threshold)>, threshold) << endl;
        } catch (const exception& e) {
            cout << "FirstThat (ningun valor > " << threshold << "): " << e.what() << endl;
        }
    }

    Container c_read;
    { ifstream in(filename); in >> c_read; }
    cout << "Persistencia leida desde archivo: " << c_read << endl;

    if constexpr (requires { c_read.pop(); }) {
        cout << "Extraccion ordenada con pop(): [ ";
        while (!c_read.empty()) cout << c_read.pop() << " ";
        cout << "]\n";
    }

    c.clear();
    cout << "Tras clear(): empty = " << boolalpha << c.empty() << " " << c << endl;
    if (!initialValues.empty()) {
        c.push_back(initialValues.front().first, initialValues.front().second);
        cout << "Nuevo push_back post-clear: " << c << endl;
    }
}

void DemoVector() {
    RunContainerTestSuite<Vector<VectorAscTraits<TX>>>(
        "DEMO VECTOR", "vector.txt", {{0, 10}, {1, 11}, {2, 12}, {3, 13}, {4, 14}}, {{5, 15}, {6, 16}, {7, 17}, {8, 18}, {9, 19}});
    RunContainerTestSuite<Vector<VectorAscTraits<string>>>(
        "DEMO VECTOR STRINGS", "vector_str.txt", {{"Hello", 1}, {"World", 2}});
}

void DemoRaceCondition() {
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
    cout << "\nDemoRaceCondition: N=" << N << ", size=" << vec.size()
         << " (suma esperada=" << expectedSum << ", obtenida=" << actualSum << ")\n";
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
