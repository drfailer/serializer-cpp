#ifndef TREE_HPP
#define TREE_HPP
#include <iostream>
#include <serializer/serializer.hpp>
#include <serializer/tools/macros.hpp>

template <typename T> struct Node {
  public:
    explicit Node(T value = 0) : value(value) {}
    ~Node() {
        delete left;
        delete right;
    }

    SERIALIZE(value, left, right, SER_DFUN({
                  if (this->left)
                      this->left->father = this;
                  if (this->right)
                      this->right->father = this;
              }));

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
  public:
    ~Tree() { delete root; }
    SERIALIZE(root);

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
