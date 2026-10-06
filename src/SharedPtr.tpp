/* SharedPtr для одиночного объекта */
/* Создание */
template <typename T>
SharedPtr<T>::SharedPtr(T* p) : ptr(p), ref_count(p == nullptr ? nullptr : new int(1)) {}

/* Копирование */
template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other) : ptr(other.ptr), ref_count(other.ref_count) {
    if (ref_count != nullptr) {
        ++(*ref_count);
    }
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) {
    if (this != &other) {
        if (ref_count != nullptr && --(*ref_count) == 0) {
            delete ptr;
            delete ref_count;
        }

        ptr = other.ptr;
        ref_count = other.ref_count;
        if (ref_count != nullptr) {
            ++(*ref_count);
        }
    }
    return *this;
}

/* Перемещение */
template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& other) noexcept : ptr(other.ptr), ref_count(other.ref_count) {
    other.ptr = nullptr;
    other.ref_count = nullptr;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) noexcept {
    if (this != &other) {
        if (ref_count != nullptr && --(*ref_count) == 0) {
            delete ptr;
            delete ref_count;
        }

        ptr = other.ptr;
        ref_count = other.ref_count;
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }
    return *this;
}

/* Удаление */
template <typename T>
SharedPtr<T>::~SharedPtr() {
    if (ref_count != nullptr) {
        if (*ref_count == 1) {
            delete ptr;
            delete ref_count;
        } else {
            --(*ref_count);
        }
    }
}

/* Доступ к объекту и счётчику владельцев */
template <typename T>
T& SharedPtr<T>::operator*() const {
    return *ptr;
}

template <typename T>
T* SharedPtr<T>::operator->() const {
    return ptr;
}

template <typename T>
T* SharedPtr<T>::get() const {
    return ptr;
}

template <typename T>
SharedPtr<T>::operator bool() const {
    return ptr != nullptr;
}

template <typename T>
int SharedPtr<T>::use_count() const {
    return ref_count == nullptr ? 0 : *ref_count;
}

/* Управление владением */
template <typename T>
void SharedPtr<T>::reset(T* p) {
    if (ptr != p) {
        if (ptr != nullptr) {
            --(*ref_count);
        }

        if (ref_count != nullptr && *ref_count == 0) {
            delete ptr;
            delete ref_count;
        }

        if (p == nullptr) {
            ptr = nullptr;
            ref_count = nullptr;
        } else {
            ptr = p;
            ref_count = new int(1);
        }
    }
}

template <typename T>
void SharedPtr<T>::swap(SharedPtr& other) {
    T* tmp_ptr = ptr;
    int* tmp_count = ref_count;

    ptr = other.ptr;
    ref_count = other.ref_count;
    other.ptr = tmp_ptr;
    other.ref_count = tmp_count;
}

/* SharedPtr для динамического массива */
/* Создание */
template <typename T>
SharedPtr<T[]>::SharedPtr(T* p) : ptr(p), ref_count(p == nullptr ? nullptr : new int(1)) {}

/* Копирование */
template <typename T>
SharedPtr<T[]>::SharedPtr(const SharedPtr& other) : ptr(other.ptr), ref_count(other.ref_count) {
    if (ref_count != nullptr) {
        ++(*ref_count);
    }
}

template <typename T>
SharedPtr<T[]>& SharedPtr<T[]>::operator=(const SharedPtr& other) {
    if (this != &other) {
        if (ref_count != nullptr && --(*ref_count) == 0) {
            delete[] ptr;
            delete ref_count;
        }

        ptr = other.ptr;
        ref_count = other.ref_count;
        if (ref_count != nullptr) {
            ++(*ref_count);
        }
    }
    return *this;
}

/* Перемещение */
template <typename T>
SharedPtr<T[]>::SharedPtr(SharedPtr&& other) noexcept : ptr(other.ptr), ref_count(other.ref_count) {
    other.ptr = nullptr;
    other.ref_count = nullptr;
}

template <typename T>
SharedPtr<T[]>& SharedPtr<T[]>::operator=(SharedPtr&& other) noexcept {
    if (this != &other) {
        if (ref_count != nullptr && --(*ref_count) == 0) {
            delete[] ptr;
            delete ref_count;
        }

        ptr = other.ptr;
        ref_count = other.ref_count;
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }
    return *this;
}

/* Удаление */
template <typename T>
SharedPtr<T[]>::~SharedPtr() {
    if (ref_count != nullptr && --(*ref_count) == 0) {
        delete[] ptr;
        delete ref_count;
    }
}

/* Доступ к элементам массива и счётчику владельцев */
template <typename T>
T& SharedPtr<T[]>::operator[](std::size_t index) const {
    return ptr[index];
}

template <typename T>
T* SharedPtr<T[]>::get() const {
    return ptr;
}

template <typename T>
SharedPtr<T[]>::operator bool() const {
    return ptr != nullptr;
}

template <typename T>
int SharedPtr<T[]>::use_count() const {
    return ref_count == nullptr ? 0 : *ref_count;
}

/* Управление владением */
template <typename T>
void SharedPtr<T[]>::reset(T* p) {
    if (ptr != p) {
        if (ref_count != nullptr && --(*ref_count) == 0) {
            delete[] ptr;
            delete ref_count;
        }

        ptr = p;
        ref_count = p == nullptr ? nullptr : new int(1);
    }
}

template <typename T>
void SharedPtr<T[]>::swap(SharedPtr& other) {
    T* tmp_ptr = ptr;
    int* tmp_count = ref_count;

    ptr = other.ptr;
    ref_count = other.ref_count;
    other.ptr = tmp_ptr;
    other.ref_count = tmp_count;
}
