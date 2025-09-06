#pragma once

namespace mini {

    template<class T> struct default_delete;

    template<class T> struct default_delete<T[]>;

    template<class T, class D = default_delete<T>> class unique_ptr;

    template<class T, class D = default_delete<T>> // TODO: understand what's this class D
    class unique_ptr {

    public:
        unique_ptr();
        ~unique_ptr();
        
        unique_ptr(const unique_ptr&) = delete;
        unique_ptr& operator=(const unique_ptr&) = delete;

    private:

    }


}