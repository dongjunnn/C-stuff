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

        unique_ptr(T* raw_ptr, D deleter) {
            ptr = raw_ptr;
            this->deleter = deleter;
        }

        unique_ptr(unique_ptr&& other) noexcept { // move constructor; && is to differentiate between different overloaded functionse
            ptr = other.ptr;
            other.ptr = nullptr;
        }

        unique_ptr operator=(const unique_ptr&& other) {
            ptr = other.ptr;
            deleter(other.ptr);
        }

        ~unique_ptr() noexcept {
            deleter(ptr);
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

        D get_deleter() {
            return deleter;
        }

        private:
        D deleter = D();
        T* ptr = nullptr;
    
    };

    // NOTE:
    // In C++, partial specializations are NOT allowed to introduce default template arguments.
    // That’s why we cannot write:
    //
    //   template<class T, class D = std::default_delete<T[]>>
    //   class unique_ptr<T[], D> { ... };   // ❌ illegal
    //
    // Instead, we must declare it as:
    //
    //   template<class T, class D>
    //   class unique_ptr<T[], D> { ... };   // ✅
    //
    // If we want `std::default_delete<T[]>` to be the default deleter,
    // we rely on the primary template’s default and on constructor overloads.
    // The C++ standard library handles this in exactly this way for std::unique_ptr.
    template<class T, class D>
    class unique_ptr<T[], D> {
    public:
        // lifecycle (just what's needed to own/release an array and move it)
        constexpr unique_ptr() noexcept = default;
        explicit unique_ptr(T* p) noexcept {
            ptr_ = p;
        }
        unique_ptr(unique_ptr&& other) noexcept {
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        unique_ptr& operator=(unique_ptr&& other) noexcept {
            // ptr = other.ptr;
            // other.ptr = nullptr;
            if (this == &other) {
                return *this;
            }
            if (ptr_) {
                del_(ptr_);
            }
            del_ = std::move(other.del_);
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
            return *this;
        }

        ~unique_ptr() {
            if (ptr_) {
                del_(ptr_);
            }
        }

        // required observers for Session 7
        T* get() const noexcept {
            return ptr_;
        }

        T& operator[](std::size_t i) const {
            return *(ptr_+i);
        }

        explicit operator bool() const noexcept {
            return ptr_ != nullptr;
        }

    private:
        T* ptr_ = nullptr;
        D  del_ = D{};
    };

}