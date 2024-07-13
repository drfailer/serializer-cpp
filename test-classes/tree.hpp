#ifndef TREE_HPP
#define TREE_HPP
#include "serializer/serializable.hpp"
#include <iostream>

template <typename T> struct Node {
    SERIALIZABLE(T, Node<T> *, Node<T> *, serializer::function_t);
  public:
    Node(T value = 0)
        : SERIALIZER(this->value, left, right, SER_DFUN({
                         if (this->left)
                             this->left->father = this;
                         if (this->right)
                             this->right->father = this;
                     })),
          value(value) {}
    ~Node() {
        delete left;
        delete right;
    }

    void print(size_t rank = 0) {
        if (left)
            left->print(rank + 1);
        std::cout << std::string(rank, ' ') << value << std::endl;
        if (right)
            right->print(rank + 1);
    }

    T value;
    Node<T> *father = nullptr;
    Node<T> *left = nullptr;
    Node<T> *right = nullptr;
};

template <typename T> struct Tree {
    SERIALIZABLE(Node<T> *)
  public:
    Tree(): SERIALIZER(root) {}
    ~Tree() = default;

    void print() {
        if (root) {
            root->print(0);
        } else {
            std::cout << "empty" << std::endl;
        }
    }

    void insert(T value) {
        auto newNode = new Node<T>(value);
        Node<T> **curr = &root;

        while (*curr) {
            if ((*curr)->value < value) {
                curr = &(*curr)->left;
            } else {
                curr = &(*curr)->right;
            }
        }
        *curr = newNode;
        if (curr != &root)
            newNode->father = *curr;
    }

    Node<T> *root = nullptr;
};

#endif
