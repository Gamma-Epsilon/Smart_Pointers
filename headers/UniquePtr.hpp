#pragma once

#include <cstddef>

/* UniquePtr для одиночного объекта */
template <typename T>
class UniquePtr {
private:
    T* ptr;
public:
    /* Создание и удаление */
    UniquePtr(T* p = nullptr);
    ~UniquePtr();

    /* Копирование запрещено */
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    /* Перемещение */
    UniquePtr(UniquePtr&& other) noexcept;
    UniquePtr& operator=(UniquePtr&& other) noexcept;

    /* Доступ к объекту */
    T& operator*() const;
    T* operator->() const;
    T* get() const;
    operator bool() const;

    /* Управление владением */
    T* release();
    void reset(T* p = nullptr);
    void swap(UniquePtr& other);
};

/* UniquePtr для динамического массива */
template <typename T>
class UniquePtr<T[]> {
private:
    T* ptr;
public:
    /* Создание и удаление */
    UniquePtr(T* p = nullptr);
    ~UniquePtr();

    /* Копирование запрещено */
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    /* Перемещение */
    UniquePtr(UniquePtr&& other) noexcept;
    UniquePtr& operator=(UniquePtr&& other) noexcept;

    /* Доступ к элементам массива */
    T& operator[](std::size_t index) const;
    T* get() const;
    explicit operator bool() const;

    /* Управление владением */
    T* release();
    void reset(T* p = nullptr);
    void swap(UniquePtr& other);
};

#include "../src/UniquePtr.tpp"


/*
Почему нельзя использовать конструкторы Shared и Unique?


auto p = std::make_shared<int>(6);
- делает сразу же одну общую аллокацию и int, и control block. 

=> выгоднее, чем обычный std::shared<int>(new int(6)) по числу аллокаций. 

*/