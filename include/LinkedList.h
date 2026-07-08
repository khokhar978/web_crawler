#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdexcept>
#include <iostream>

template <typename T>
class LinkedList {
private:
    // Node structure for the linked list
    struct Node {
        T data;
        Node* next;
        
        Node(const T& val) : data(val), next(nullptr) {}
        Node(T&& val) : data(std::move(val)), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int size;

public:
    // Default constructor
    LinkedList() : head(nullptr), tail(nullptr), size(0) {}

    // Copy Constructor (Deep Copy)
    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), size(0) {
        Node* current = other.head;
        while (current != nullptr) {
            append(current->data);
            current = current->next;
        }
    }

    // Copy Assignment Operator
    LinkedList& operator=(const LinkedList& other) {
        if (this == &other) {
            return *this;
        }
        clear();
        Node* current = other.head;
        while (current != nullptr) {
            append(current->data);
            current = current->next;
        }
        return *this;
    }

    // Move Constructor
    LinkedList(LinkedList&& other) noexcept : head(other.head), tail(other.tail), size(other.size) {
        // FIXED: Neutralize the source object so it doesn't delete the nodes when it's destroyed!
        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
    }

    // Move Assignment Operator
    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        clear();
        
        head = other.head;
        tail = other.tail;
        size = other.size;
        
        // FIXED: Neutralize the source object
        other.head = nullptr;
        other.tail = nullptr;
        other.size = 0;
        return *this;
    }

    // Destructor
    ~LinkedList() {
        clear();
    }

    // --- State Queries ---
    
    [[nodiscard]] int getSize() const { return size; }
    [[nodiscard]] bool isEmpty() const { return size == 0; }

    // --- Modifiers ---

    void prepend(const T& val) {
        Node* newNode = new Node(val);
        if (isEmpty()) {
            head = newNode;
            tail = newNode;
        } else {
            newNode->next = head;
            head = newNode;
        }
        size++;
    }

    void append(const T& val) {
        Node* newNode = new Node(val);
        if (isEmpty()) {
            head = newNode;
            // FIXED: Set tail as well when appending to empty list!
            tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        size++;
    }

    void insert(int index, const T& val) {
        if (index < 0 || index > size) {
            throw std::out_of_range("Index out of bounds");
        }
        if (index == 0) {
            prepend(val);
            return;
        }
        if (index == size) {
            append(val);
            return;
        }

        Node* current = head;
        for (int i = 0; i < index - 1; ++i) {
            current = current->next;
        }

        Node* newNode = new Node(val);
        newNode->next = current->next;
        current->next = newNode;
        size++;
    }

    T& get(int index) {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Index out of bounds");
        }
        Node* current = head;
        for (int i = 0; i < index; ++i) {
            current = current->next;
        }
        return current->data;
    }

    const T& get(int index) const {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Index out of bounds");
        }
        Node* current = head;
        for (int i = 0; i < index; ++i) {
            current = current->next;
        }
        return current->data;
    }

    void removeFirst() {
        if (isEmpty()) throw std::out_of_range("List is empty");
        Node* temp = head;
        head = head->next;
        delete temp;
        size--;
        if (size == 0) tail = nullptr;
    }

    void removeLast() {
        if (isEmpty()) throw std::out_of_range("List is empty");
        if (size == 1) {
            removeFirst();
            return;
        }
        Node* current = head;
        while (current->next != tail) {
            current = current->next;
        }
        delete tail;
        tail = current;
        tail->next = nullptr;
        size--;
    }

    void remove(int index) {
        if (index < 0 || index >= size) {
            throw std::out_of_range("Index out of bounds");
        }
        if (index == 0) {
            removeFirst();
            return;
        }
        if (index == size - 1) {
            removeLast();
            return;
        }

        Node* current = head;
        Node* prev = nullptr;
        for (int i = 0; i < index; ++i) {
            prev = current;
            current = current->next;
        }

        // FIXED: Properly bypass the current node by linking prev directly to current's next node!
        prev->next = current->next;
        
        delete current;
        size--;
    }

    bool contains(const T& val) const {
        return indexOf(val) != -1;
    }

    int indexOf(const T& val) const {
        Node* current = head;
        int index = 0;
        while (current != nullptr) {
            if (current->data == val) {
                return index;
            }
            // FIXED: Removed the else block that was prematurely returning -1!
            current = current->next;
            index++;
        }
        return -1;
    }

    void clear() {
        Node* current = head;
        while (current != nullptr) {
            Node* nextNode = current->next;
            delete current;
            current = nextNode;
        }
        head = nullptr;
        tail = nullptr;
        size = 0;
    }

    // --- Iterators ---
    class Iterator {
    private:
        Node* current;
    public:
        Iterator(Node* ptr) : current(ptr) {}
        T& operator*() { return current->data; }
        Iterator& operator++() {
            current = current->next;
            return *this;
        }
        bool operator!=(const Iterator& other) const {
            return current != other.current;
        }
    };

    class ConstIterator {
    private:
        const Node* current;
    public:
        ConstIterator(const Node* ptr) : current(ptr) {}
        const T& operator*() const { return current->data; }
        ConstIterator& operator++() {
            current = current->next;
            return *this;
        }
        bool operator!=(const ConstIterator& other) const {
            return current != other.current;
        }
    };

    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
    ConstIterator begin() const { return ConstIterator(head); }
    ConstIterator end() const { return ConstIterator(nullptr); }

    // Range constructor
    template <typename InputIt>
    LinkedList(InputIt first, InputIt last) : head(nullptr), tail(nullptr), size(0) {
        for (auto it = first; it != last; ++it) {
            append(*it);
        }
    }
};

#endif // LINKED_LIST_H
