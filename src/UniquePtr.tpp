/* UniquePtr для одиночного объекта */
/* Создание и удаление */
template <typename T>
UniquePtr<T>::UniquePtr(T* p) : ptr(p) {}

template <typename T>
UniquePtr<T>::~UniquePtr() {
    delete ptr;
}

/* Перемещение */
template <typename T>
UniquePtr<T>::UniquePtr(UniquePtr&& other) noexcept : ptr(other.ptr) {
    other.ptr = nullptr;
}

template <typename T>
UniquePtr<T>& UniquePtr<T>::operator=(UniquePtr&& other) noexcept {
    if (this != &other) {
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
    }
    return *this;
}

/* Доступ к объекту */
template <typename T>
T& UniquePtr<T>::operator*() const {
    return *ptr;
}

template <typename T>
T* UniquePtr<T>::operator->() const {
    return ptr;
}

template <typename T>
T* UniquePtr<T>::get() const {
    return ptr;
}

template <typename T>
UniquePtr<T>::operator bool() const {
    return ptr != nullptr;
}

/* Управление владением */
template <typename T>
T* UniquePtr<T>::release() {
    T* tmp = ptr;
    ptr = nullptr;
    return tmp;
}

template <typename T>
void UniquePtr<T>::reset(T* p) {
    if (ptr != p) {
        delete ptr;
        ptr = p;
    }
}

template <typename T>
void UniquePtr<T>::swap(UniquePtr& other) {
    T* tmp = ptr;
    ptr = other.ptr;
    other.ptr = tmp;
}

/* UniquePtr для динамического массива */
/* Создание и удаление */
template <typename T>
UniquePtr<T[]>::UniquePtr(T* p) : ptr(p) {}

template <typename T>
UniquePtr<T[]>::~UniquePtr() {
    delete[] ptr;
}

/* Перемещение */
template <typename T>
UniquePtr<T[]>::UniquePtr(UniquePtr&& other) noexcept : ptr(other.ptr) {
    other.ptr = nullptr;
}

template <typename T>
UniquePtr<T[]>& UniquePtr<T[]>::operator=(UniquePtr&& other) noexcept {
    if (this != &other) {
        delete[] ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
    }
    return *this;
}

/* Доступ к элементам массива */
template <typename T>
T& UniquePtr<T[]>::operator[](std::size_t index) const {
    return ptr[index];
}

template <typename T>
T* UniquePtr<T[]>::get() const {
    return ptr;
}

template <typename T>
UniquePtr<T[]>::operator bool() const {
    return ptr != nullptr;
}

/* Управление владением */
template <typename T>
T* UniquePtr<T[]>::release() {
    T* tmp = ptr;
    ptr = nullptr;
    return tmp;
}

template <typename T>
void UniquePtr<T[]>::reset(T* p) {
    if (ptr != p) {
        delete[] ptr;
        ptr = p;
    }
}

template <typename T>
void UniquePtr<T[]>::swap(UniquePtr& other) {
    T* tmp = ptr;
    ptr = other.ptr;
    other.ptr = tmp;
}
