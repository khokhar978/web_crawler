#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "DynamicArray.h"
#include "LinkedList.h"
#include <string>
#include <stdexcept>
#include <functional>

template <typename K, typename V>
class HashMap {
private:
    struct KeyValuePair {
        K key;
        V value;
        KeyValuePair(const K& k, const V& v) : key(k), value(v) {}
    };

    DynamicArray<LinkedList<KeyValuePair>> buckets;
    int current_size;
    
    // Hash function
    int hash(const K& key) const {
        std::hash<K> hasher;
        // Use size_t arithmetic to avoid negative values from signed truncation
        return static_cast<int>(hasher(key) % static_cast<size_t>(buckets.capacity()));
    }

public:
    HashMap(int capacity = 16) : buckets(capacity), current_size(0) {
        // Initialize empty buckets
        for (int i = 0; i < capacity; ++i) {
            buckets.append(LinkedList<KeyValuePair>());
        }
    }

    [[nodiscard]] int size() const { return current_size; }
    [[nodiscard]] bool isEmpty() const { return current_size == 0; }
    [[nodiscard]] int getCapacity() const { return buckets.capacity(); }

    void put(const K& key, const V& value) {
        if (current_size >= buckets.capacity() * 0.75) { // Load factor 0.75
            resize();
        }

        int index = hash(key);
        LinkedList<KeyValuePair>& bucket = buckets.get(index);
        
        for (auto& pair : bucket) {
            if (pair.key == key) {
                pair.value = value;
                return;
            }
        }
        
        bucket.append(KeyValuePair(key, value));
        current_size++;
    }

private:
    void resize() {
        int new_capacity = buckets.capacity() * 2;
        DynamicArray<LinkedList<KeyValuePair>> old_buckets = std::move(buckets);
        
        buckets = DynamicArray<LinkedList<KeyValuePair>>(new_capacity);
        for (int i = 0; i < new_capacity; ++i) {
            buckets.append(LinkedList<KeyValuePair>());
        }
        
        // FIXED: Reset current_size to 0 before rehashing, because put() increments it!
        current_size = 0;
        
        for (int i = 0; i < old_buckets.capacity(); ++i) {
            for (auto& pair : old_buckets.get(i)) {
                put(pair.key, pair.value);
            }
        }
    }
public:

    V& get(const K& key) {
        int index = hash(key);
        LinkedList<KeyValuePair>& bucket = buckets.get(index);
        
        for (auto& pair : bucket) {
            if (pair.key == key) {
                return pair.value;
            }
        }
        throw std::out_of_range("Key not found in map");
    }

    const V& get(const K& key) const {
        int index = hash(key);
        const LinkedList<KeyValuePair>& bucket = buckets.get(index);
        
        for (const auto& pair : bucket) {
            if (pair.key == key) {
                return pair.value;
            }
        }
        throw std::out_of_range("Key not found in map");
    }

    void remove(const K& key) {
        int index = hash(key);
        LinkedList<KeyValuePair>& bucket = buckets.get(index);
        
        int list_index = 0;
        for (auto& pair : bucket) {
            if (pair.key == key) {
                bucket.remove(list_index);
                current_size--;
                return;
            }
            list_index++;
        }
        // If not found, do nothing
    }

    bool contains(const K& key) const {
        int index = hash(key);
        const LinkedList<KeyValuePair>& bucket = buckets.get(index);
        for (const auto& pair : bucket) {
            if (pair.key == key) return true;
        }
        return false;
    }

    V& operator[](const K& key) {
        if (current_size >= buckets.capacity() * 0.75) {
            resize();
        }

        int index = hash(key);
        LinkedList<KeyValuePair>& bucket = buckets.get(index);
        
        for (auto& pair : bucket) {
            if (pair.key == key) {
                return pair.value;
            }
        }
        
        // Key not found, insert a default constructed value
        bucket.append(KeyValuePair(key, V{}));
        current_size++;
        
        // Return reference to the newly appended value at the end of the bucket
        return bucket.get(bucket.getSize() - 1).value;
    }

    void clear() {
        for (int i = 0; i < buckets.capacity(); ++i) {
            buckets.get(i).clear();
        }
        current_size = 0;
    }

    DynamicArray<K> getKeys() const {
        DynamicArray<K> keys(current_size > 0 ? current_size : 4);
        for (int i = 0; i < buckets.capacity(); ++i) {
            for (const auto& pair : buckets.get(i)) {
                keys.append(pair.key);
            }
        }
        return keys;
    }

    DynamicArray<V> getValues() const {
        DynamicArray<V> values(current_size > 0 ? current_size : 4);
        for (int i = 0; i < buckets.capacity(); ++i) {
            for (const auto& pair : buckets.get(i)) {
                values.append(pair.value);
            }
        }
        return values;
    }
};

#endif // HASH_MAP_H
