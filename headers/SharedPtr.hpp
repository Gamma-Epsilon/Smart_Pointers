#pragma once

#include <cstddef>

/* SharedPtr для одиночного объекта */
template <typename T>
class SharedPtr {
private:
    T* ptr;
    int* ref_count;
public:
    /* Создание */
    SharedPtr(T* p = nullptr);

    /* Копирование */
    SharedPtr(const SharedPtr& other);
    SharedPtr& operator=(const SharedPtr& other);

    /* Перемещение */
    SharedPtr(SharedPtr&& other) noexcept;
    SharedPtr& operator=(SharedPtr&& other) noexcept;

    /* Удаление */
    ~SharedPtr();

    /* Доступ к объекту и счётчику владельцев */
    T& operator*() const;
    T* operator->() const;
    T* get() const;
    operator bool() const;
    int use_count() const;

    /* Управление владением */
    void reset(T* p = nullptr);
    void swap(SharedPtr& other);
};

/* SharedPtr для динамического массива */
template <typename T>
class SharedPtr<T[]> {
private:
    T* ptr;
    int* ref_count;
public:
    /* Создание */
    SharedPtr(T* p = nullptr);

    /* Копирование */
    SharedPtr(const SharedPtr& other);
    SharedPtr& operator=(const SharedPtr& other);

    /* Перемещение */
    SharedPtr(SharedPtr&& other) noexcept;
    SharedPtr& operator=(SharedPtr&& other) noexcept;

    /* Удаление */
    ~SharedPtr();

    /* Доступ к элементам массива и счётчику владельцев */
    T& operator[](std::size_t index) const;
    T* get() const;
    explicit operator bool() const;
    int use_count() const;

    /* Управление владением */
    void reset(T* p = nullptr);
    void swap(SharedPtr& other);
};

#include "../src/SharedPtr.tpp"
