#pragma once

namespace mini {
    template<class T> struct default_delete;
    template<class T> struct default_delete<T[]>;

    template<class T, class D = default_delete<T>> // TODO: understand what's this class D
    class unique_ptr {  

        public:
        unique_ptr() = default;
        unique_ptr(const unique_ptr&) = delete;
        unique_ptr operator=(const unique_ptr) = delete;

        unique_ptr(T* raw_ptr) {
            ptr = raw_ptr;
        }

        T* get() {
            return ptr;
        }

        T operator*() {
            return *ptr;
        }

        T* operator->() {
            return ptr;
        }

        explicit operator bool() const noexcept {
            return ptr != nullptr;
        }

        private:
        T* ptr = nullptr;
    
    };
}