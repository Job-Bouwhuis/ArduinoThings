#pragma once
#include "List.hpp"
#include <memory>

namespace Util
{
        template <typename K, typename V>
        struct KeyValue
        {
                K key;
                V value;
        };

        template <typename K, typename V>
        class Map
        {
        public:
                Map() = default;

                // set by copy
                void Set(const K &key, const V &value)
                {
                        for (auto &kvPtr : items) // kvPtr is std::shared_ptr<KeyValue<K,V>>&
                        {
                                if (kvPtr && kvPtr->key == key)
                                {
                                        kvPtr->value = value;
                                        return;
                                }
                        }

                        KeyValue<K, V> kv{key, value};
                        items.Add(std::move(kv));
                }

                // set by move
                void Set(const K &key, V &&value)
                {
                        for (auto &kvPtr : items)
                        {
                                if (kvPtr && kvPtr->key == key)
                                {
                                        kvPtr->value = std::move(value);
                                        return;
                                }
                        }

                        KeyValue<K, V> kv{key, std::move(value)};
                        items.Add(std::move(kv));
                }

                // Returns aliasing shared_ptr to value (lifetime tied to list storage)
                std::shared_ptr<V> Get(const K &key) const
                {
                        for (SIZE i = 0; i < items.Count(); ++i)
                        {
                                const std::shared_ptr<KeyValue<K, V>> &kvPtr = items[i];
                                if (kvPtr && kvPtr->key == key)
                                {
                                        // aliasing shared_ptr: shares ownership with kvPtr but points to kvPtr->value
                                        return std::shared_ptr<V>(kvPtr, &kvPtr->value);
                                }
                        }
                        return nullptr;
                }

                bool Remove(const K &key)
                {
                        for (SIZE i = 0; i < items.Count(); ++i)
                        {
                                const std::shared_ptr<KeyValue<K, V>> &kvPtr = items[i];
                                if (kvPtr && kvPtr->key == key)
                                {
                                        items.RemoveAt(i);
                                        return true;
                                }
                        }
                        return false;
                }

                bool Contains(const K &key) const
                {
                        for (SIZE i = 0; i < items.Count(); ++i)
                        {
                                const std::shared_ptr<KeyValue<K, V>> &kvPtr = items[i];
                                if (kvPtr && kvPtr->key == key)
                                        return true;
                        }
                        return false;
                }

                SIZE Count() const { return items.Count(); }

                // Iterator yields KeyValue<K,V>& by dereferencing the shared_ptr inside the list
                struct Iterator
                {
                        std::shared_ptr<KeyValue<K, V>> *current;

                        Iterator(std::shared_ptr<KeyValue<K, V>> *ptr) : current(ptr) {}

                        Iterator &operator++()
                        {
                                ++current;
                                return *this;
                        }
                        Iterator operator++(int)
                        {
                                Iterator tmp = *this;
                                ++current;
                                return tmp;
                        }

                        KeyValue<K, V> &operator*() const { return **current; }
                        KeyValue<K, V> *operator->() const { return current->get(); }

                        bool operator!=(const Iterator &other) const { return current != other.current; }
                        bool operator==(const Iterator &other) const { return current == other.current; }
                };

                // Const iterator for const Map
                struct ConstIterator
                {
                        const std::shared_ptr<KeyValue<K, V>> *current;

                        ConstIterator(const std::shared_ptr<KeyValue<K, V>> *ptr) : current(ptr) {}

                        ConstIterator &operator++()
                        {
                                ++current;
                                return *this;
                        }
                        ConstIterator operator++(int)
                        {
                                ConstIterator tmp = *this;
                                ++current;
                                return tmp;
                        }

                        const KeyValue<K, V> &operator*() const { return **current; }
                        const KeyValue<K, V> *operator->() const { return current->get(); }

                        bool operator!=(const ConstIterator &other) const { return current != other.current; }
                        bool operator==(const ConstIterator &other) const { return current == other.current; }
                };

                Iterator begin() { return Iterator(items.begin()); }
                Iterator end() { return Iterator(items.end()); }

                ConstIterator begin() const { return ConstIterator(items.begin()); }
                ConstIterator end() const { return ConstIterator(items.end()); }

        private:
                // List stores std::shared_ptr<KeyValue<K,V>> internally (per your List implementation)
                List<KeyValue<K, V>> items;
        };
}
