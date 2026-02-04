#pragma once
#include <cstddef>
#include <utility>
#include <memory>
#include <type_traits>

#define SIZE std::size_t

namespace Util
{
    template <typename T, int GROW_SIZE = 4, int INITIAL = 0>
    class List
    {
    private:
        // helper types
        static constexpr bool is_raw_ptr = std::is_pointer<T>::value;
        using element_t = std::conditional_t<is_raw_ptr, std::remove_pointer_t<T>, T>;
        using stored_t = std::shared_ptr<element_t>;

    public:
        List() : data(nullptr), count(0), capacity(0)
        {
            EnsureCapacity(INITIAL);
        }

        // Move
        List(List &&other) noexcept
            : data(other.data), count(other.count), capacity(other.capacity)
        {
            other.data = nullptr;
            other.count = 0;
            other.capacity = 0;
        }

        // Copy
        List(const List &other) : data(nullptr), count(0), capacity(0)
        {
            if (other.count == 0)
                return;
            Reallocate(other.count);
            for (SIZE i = 0; i < other.count; ++i)
            {
                if (other.data[i])
                    data[i] = std::make_shared<element_t>(*other.data[i]);
            }
            count = other.count;
        }

        // Move
        List &operator=(List &&other) noexcept
        {
            if (this == &other)
                return *this;
            clearStorage();
            delete[] data;

            data = other.data;
            count = other.count;
            capacity = other.capacity;

            other.data = nullptr;
            other.count = 0;
            other.capacity = 0;
            return *this;
        }

        // Copy
        List &operator=(const List &other)
        {
            if (this == &other)
                return *this;

            clearStorage();
            delete[] data;
            data = nullptr;
            count = 0;
            capacity = 0;

            if (other.count == 0)
                return *this;

            Reallocate(other.count);
            for (SIZE i = 0; i < other.count; ++i)
            {
                if (other.data[i])
                    data[i] = std::make_shared<element_t>(*other.data[i]);
            }
            count = other.count;
            return *this;
        }

        ~List()
        {
            clearStorage();
            delete[] data;
            data = nullptr;
            count = 0;
            capacity = 0;
        }

        template <typename U = T>
        std::enable_if_t<!std::is_pointer<U>::value, void> Add(const U &value)
        {
            EnsureCapacity(count + 1);
            data[count] = std::make_shared<element_t>(value);
            ++count;
        }

        template <typename U = T>
        std::enable_if_t<!std::is_pointer<U>::value, void> Add(U &&value)
        {
            EnsureCapacity(count + 1);
            data[count] = std::make_shared<element_t>(std::move(value));
            ++count;
        }

        template <typename U = T>
        std::enable_if_t<std::is_pointer<U>::value, void> Add(std::remove_pointer_t<U> *rawPtr)
        {
            EnsureCapacity(count + 1);
            data[count] = stored_t(rawPtr);
            ++count;
        }

        template <typename U = T>
        std::enable_if_t<std::is_pointer<U>::value, void> Add(const element_t &value)
        {
            EnsureCapacity(count + 1);
            data[count] = std::make_shared<element_t>(value);
            ++count;
        }

        template <typename U = T>
        std::enable_if_t<std::is_pointer<U>::value, void> Add(element_t &&value)
        {
            EnsureCapacity(count + 1);
            data[count] = std::make_shared<element_t>(std::move(value));
            ++count;
        }

        void Add(const stored_t &ptr)
        {
            EnsureCapacity(count + 1);
            data[count] = ptr;
            ++count;
        }

        void Add(stored_t &&ptr)
        {
            EnsureCapacity(count + 1);
            data[count] = std::move(ptr);
            ++count;
        }

        stored_t &operator[](SIZE index)
        {
            return data[index];
        }

        const stored_t &operator[](SIZE index) const
        {
            return data[index];
        }

        stored_t Get(SIZE index) const
        {
            if (!data || index >= count)
                return nullptr;
            return data[index];
        }

        SIZE Count() const { return count; }
        SIZE Capacity() const { return capacity; }

        void Clear()
        {
            clearStorage();
            count = 0;
        }

        void RemoveAt(SIZE index)
        {
            if (index >= count)
                return;
            for (SIZE i = index; i + 1 < count; ++i)
                data[i] = std::move(data[i + 1]);
            data[count - 1].reset();
            --count;
        }

        void Reserve(SIZE newCapacity)
        {
            if (newCapacity <= capacity)
                return;
            Reallocate(newCapacity);
        }

        stored_t *begin() { return data; }
        stored_t *end() { return data + count; }

        const stored_t *begin() const { return data; }
        const stored_t *end() const { return data + count; }

    private:
        stored_t *data;
        SIZE count;
        SIZE capacity;

        void EnsureCapacity(SIZE required)
        {
            if (required <= capacity)
                return;
            SIZE newCapacity = (capacity == 0) ? static_cast<SIZE>(GROW_SIZE) : (capacity + static_cast<SIZE>(GROW_SIZE));
            if (newCapacity < required)
                newCapacity = required;
            Reallocate(newCapacity);
        }

        void Reallocate(SIZE newCapacity)
        {
            auto *newData = new stored_t[newCapacity];
            for (SIZE i = 0; i < count; ++i)
                newData[i] = std::move(data[i]);
            clearStorage();
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }

        void clearStorage()
        {
            if (!data)
                return;
            for (SIZE i = 0; i < count; ++i)
                data[i].reset();
        }
    };
}
