// #pragma once
// #include <cstddef>
// #include <utility>
// #include <memory>
// #include <type_traits>

// #define SIZE std::size_t

// namespace Util
// {
//     struct IHolder;

//     using holder_sp = std::shared_ptr<IHolder>;

//     struct IHolder
//     {
//         virtual ~IHolder() = default;
//         virtual void *GetRaw() = 0;
//         virtual const void *GetRawConst() const = 0;
//     };

//     template <typename V>
//     struct HolderValue : IHolder
//     {
//         using value_t = V;

//         template <typename X>
//         HolderValue(X &&v) : value(std::forward<X>(v)) {}

//         void *GetRaw() override { return &value; }
//         const void *GetRawConst() const override { return &value; }

//         V value;
//         V *operator->() { return &value; }
//         const V *operator->() const { return &value; }
//         V &get() { return value; }
//     };

//     template <typename V>
//     struct HolderShared : IHolder
//     {
//         using sp_t = std::shared_ptr<V>;

//         HolderShared(sp_t s) : sp(s) {}

//         void *GetRaw() override { return sp.get(); }
//         const void *GetRawConst() const override { return sp.get(); }

//         sp_t sp;
//         V *operator->() { return sp.get(); }
//         const V *operator->() const { return sp.get(); }
//         sp_t &get_shared() { return sp; }
//     };

//     template <typename V>
//     struct HolderRaw : IHolder
//     {
//         using value_t = V;
//         HolderRaw(V *ptr) : raw(ptr) {}

//         void *GetRaw() override { return raw; }
//         const void *GetRawConst() const override { return raw; }

//         V *raw;
//         V *operator->() { return raw; }
//         const V *operator->() const { return raw; }
//         V *get() { return raw; }
//     };

//     template <typename T, int GROW_SIZE = 4, int INITIAL = 0>
//     class List
//     {
//     private:
//         static constexpr bool is_raw_ptr = std::is_pointer<T>::value;
//         using element_t = std::conditional_t<is_raw_ptr, std::remove_pointer_t<T>, T>;
//         using stored_t = holder_sp;

//     public:
//         List() : data(nullptr), count(0), capacity(0) { EnsureCapacity(INITIAL); }

//         List(List &&other) noexcept : data(other.data), count(other.count), capacity(other.capacity)
//         {
//             other.data = nullptr;
//             other.count = 0;
//             other.capacity = 0;
//         }

//         List(const List &other) : data(nullptr), count(0), capacity(0)
//         {
//             if (other.count == 0)
//                 return;
//             Reallocate(other.count);
//             for (SIZE i = 0; i < other.count; ++i)
//                 data[i] = other.data[i];
//             count = other.count;
//         }

//         List &operator=(List &&other) noexcept
//         {
//             if (this == &other)
//                 return *this;
//             clearStorage();
//             delete[] data;

//             data = other.data;
//             count = other.count;
//             capacity = other.capacity;

//             other.data = nullptr;
//             other.count = 0;
//             other.capacity = 0;
//             return *this;
//         }

//         List &operator=(const List &other)
//         {
//             if (this == &other)
//                 return *this;

//             clearStorage();
//             delete[] data;
//             data = nullptr;
//             count = 0;
//             capacity = 0;

//             if (other.count == 0)
//                 return *this;

//             Reallocate(other.count);
//             for (SIZE i = 0; i < other.count; ++i)
//                 data[i] = other.data[i];
//             count = other.count;
//             return *this;
//         }

//         ~List()
//         {
//             clearStorage();
//             delete[] data;
//             data = nullptr;
//             count = 0;
//             capacity = 0;
//         }

//         // -----------------------
//         // Add by value: stores a copy/move inside the list (it owns the value).
//         // returns raw element_t*
//         // -----------------------
//         template <typename U>
//         void *Add(U &&value)
//         {
//             using TYPE = std::decay_t<U>;
//             static_assert(std::is_base_of<element_t, TYPE>::value || std::is_same<element_t, TYPE>::value,
//                           "Added type must be element_t or derive from it");

//             EnsureCapacity(count + 1);
//             auto h = std::make_shared<HolderValue<TYPE>>(std::forward<U>(value));
//             data[count] = h;
//             ++count;
//             // return *h;
//         }

//         // -----------------------
//         // Add pointer: stores a raw pointer inside the list.
//         // returns the same pointer
//         // -----------------------
//         template <typename U>
//         U *Add(U *ptr)
//         {
//             static_assert(std::is_base_of<std::remove_pointer_t<element_t>, std::remove_pointer_t<U>>::value,
//                           "Added pointer must be element_t* or derived");

//             EnsureCapacity(count + 1);

//             auto h = std::make_shared<HolderValue<U>>(ptr); // store the raw pointer inside a HolderValue
//             data[count] = h;
//             ++count;

//             return ptr;
//         }

//         // -----------------------
//         // Add an owning shared_ptr (list keeps ownership).
//         // returns the raw pointer to the element
//         // -----------------------
//         template <typename U>
//         element_t *AddOwned(std::shared_ptr<U> sp)
//         {
//             using Udec = std::decay_t<U>;
//             static_assert(std::is_base_of<element_t, Udec>::value || std::is_same<element_t, Udec>::value,
//                           "Shared_ptr element type must be element_t or derive from it");

//             EnsureCapacity(count + 1);
//             auto h = std::make_shared<HolderShared<Udec>>(sp);
//             data[count] = h;
//             ++count;
//             return reinterpret_cast<U *>(h->GetRaw());
//         }

//         // -----------------------
//         // Add a non-owning raw pointer (list does not delete it).
//         // returns the same raw pointer
//         // -----------------------
//         element_t *AddRaw(element_t *rawPtr)
//         {
//             EnsureCapacity(count + 1);
//             auto h = std::make_shared<HolderRaw<element_t>>(rawPtr);
//             data[count] = h;
//             ++count;
//             return rawPtr;
//         }

//         // -----------------------
//         // Accessors - always return raw pointers to element_t
//         // -----------------------
//         element_t *Get(SIZE index)
//         {
//             if (!data || index >= count)
//                 return nullptr;
//             return reinterpret_cast<element_t *>(data[index]->GetRaw());
//         }

//         const element_t *Get(SIZE index) const
//         {
//             if (!data || index >= count)
//                 return nullptr;
//             return reinterpret_cast<const element_t *>(data[index]->GetRawConst());
//         }

//         // operator[] returns raw pointer
//         element_t *operator[](SIZE index) { return Get(index); }
//         const element_t *operator[](SIZE index) const { return Get(index); }

//         SIZE Count() const { return count; }
//         SIZE Capacity() const { return capacity; }

//         void Clear()
//         {
//             clearStorage();
//             count = 0;
//         }

//         void RemoveAt(SIZE index)
//         {
//             if (index >= count)
//                 return;
//             for (SIZE i = index; i + 1 < count; ++i)
//                 data[i] = std::move(data[i + 1]);
//             data[count - 1].reset();
//             --count;
//         }

//         void Reserve(SIZE newCapacity)
//         {
//             if (newCapacity <= capacity)
//                 return;
//             Reallocate(newCapacity);
//         }

//         // Iterator yields raw element_t*
//         struct Iterator
//         {
//             stored_t *ptr;
//             Iterator(stored_t *p) : ptr(p) {}
//             element_t *operator*() const { return reinterpret_cast<element_t *>((*ptr)->GetRaw()); }
//             Iterator &operator++()
//             {
//                 ++ptr;
//                 return *this;
//             }
//             Iterator operator++(int)
//             {
//                 Iterator tmp = *this;
//                 ++ptr;
//                 return tmp;
//             }
//             bool operator==(const Iterator &o) const { return ptr == o.ptr; }
//             bool operator!=(const Iterator &o) const { return ptr != o.ptr; }
//         };

//         Iterator begin() { return Iterator(data); }
//         Iterator end() { return Iterator(data + count); }
//         // const iterators
//         struct ConstIterator
//         {
//             stored_t *ptr;
//             ConstIterator(stored_t *p) : ptr(p) {}
//             const element_t *operator*() const { return reinterpret_cast<const element_t *>((*ptr)->GetRawConst()); }
//             ConstIterator &operator++()
//             {
//                 ++ptr;
//                 return *this;
//             }
//             bool operator==(const ConstIterator &o) const { return ptr == o.ptr; }
//             bool operator!=(const ConstIterator &o) const { return ptr != o.ptr; }
//         };
//         ConstIterator begin() const { return ConstIterator(data); }
//         ConstIterator end() const { return ConstIterator(data + count); }

//     private:
//         stored_t *data;
//         SIZE count;
//         SIZE capacity;

//         void EnsureCapacity(SIZE required)
//         {
//             if (required <= capacity)
//                 return;
//             SIZE newCapacity = (capacity == 0) ? static_cast<SIZE>(GROW_SIZE) : (capacity + static_cast<SIZE>(GROW_SIZE));
//             if (newCapacity < required)
//                 newCapacity = required;
//             Reallocate(newCapacity);
//         }

//         void Reallocate(SIZE newCapacity)
//         {
//             auto *newData = new stored_t[newCapacity];
//             for (SIZE i = 0; i < count; ++i)
//                 newData[i] = std::move(data[i]);
//             clearStorage();
//             delete[] data;
//             data = newData;
//             capacity = newCapacity;
//         }

//         void clearStorage()
//         {
//             if (!data)
//                 return;
//             for (SIZE i = 0; i < count; ++i)
//                 data[i].reset();
//         }
//     };
// } // namespace Util