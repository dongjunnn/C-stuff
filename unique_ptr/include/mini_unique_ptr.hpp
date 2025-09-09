#pragma once
#include <type_traits>  // for std::is_nothrow_invocable_v
#include <memory>

namespace mini {
    template<class T> struct default_delete;
    template<class T> struct default_delete<T[]>;

    template<class T, class D = std::default_delete<T>> // TODO: understand what's this class D
    class unique_ptr {  

        public:
        unique_ptr() = default;
        unique_ptr(const unique_ptr&) = delete;
        unique_ptr operator=(const unique_ptr&) = delete;

        unique_ptr(T* raw_ptr) {
            ptr = raw_ptr;
        }

        T* get() {
            return ptr;
        }

        T& operator*() { //Note: must be a reference here
            return *ptr;
        }

        T* operator->() {
            return ptr;
        }

        explicit operator bool() const noexcept {
            return ptr != nullptr;
        }

        T* release() noexcept {
            T* temp = ptr;
            ptr = nullptr;
            return temp;
        }

        void swap(unique_ptr& other) noexcept {
            T* temp = ptr;
            ptr = other.ptr;
            other.ptr = temp;
        }

        void reset(T* p = nullptr) noexcept(std::is_nothrow_invocable_v<D&, T*>) {
            deleter(ptr);
            ptr = p;
        }

        private:
        D deleter = D();
        T* ptr = nullptr;
    
    };
}