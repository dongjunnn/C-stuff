#include "mini_unique_ptr.hpp"
#include <type_traits>
#include <cassert>
#include <utility>   // std::declval
#include <cstddef>

// --- Helper: a type that counts destructions ---
struct DtorCount {
    static inline int dcount = 0;
    int v{0};
    explicit DtorCount(int x = 0) : v(x) {}
    ~DtorCount() { ++dcount; }
};

// --- Additional helper: custom deleter ---
struct CustomDeleter {
    static inline int called = 0;
    void operator()(int* p) noexcept {
        ++called;
        delete p;
    }
};

int main() {
    // ===== Compile-time properties =====
    {
        static_assert(!std::is_copy_constructible_v<mini::unique_ptr<int>>);
        static_assert(!std::is_copy_assignable_v<mini::unique_ptr<int>>);
    }

    // ===== Construct from new int(7), mutate via *p, scope end → no leaks =====
    {
        auto p = mini::unique_ptr<int>(new int(7));
        *p = *p + 1;                  // 8
        assert(*p == 8);
        // scope end → destructor should delete; check with ASan at runtime
    }
    // Stronger: verify dtor at scope end using a counting type
    {
        DtorCount::dcount = 0;
        {
            auto p = mini::unique_ptr<DtorCount>(new DtorCount(7));
            assert(p && p->v == 7);
        }
        assert(DtorCount::dcount == 1);
    }

    // ===== release(): capture raw, manually delete, ASan clean =====
    {
        auto p = mini::unique_ptr<int>(new int(7));
        *p += 1;                          // 8
        int* raw = p.release();           // take ownership
        assert(raw && *raw == 8);
        assert(!p);                       // empty
        assert(p.get() == nullptr);
        delete raw;                       // manual delete to avoid leak
    }

    // ===== reset(): exactly one destructor on live object; adopts new =====
    {
        DtorCount::dcount = 0;

        auto p = mini::unique_ptr<DtorCount>(new DtorCount(1));
        p.reset(new DtorCount(2));        // destroy old(1) exactly once
        assert(DtorCount::dcount == 1);
        assert(p && p->v == 2);

        p.reset();                        // destroy current(2)
        assert(DtorCount::dcount == 2);
        assert(!p);

        p.reset(new DtorCount(3));        // adopt new
        assert(p && p->v == 3);
        // scope end will destroy (3) → total 3
    }
    assert(DtorCount::dcount == 3);

    // ===== swap(): exchange pointees; confirm via reads =====
    {
        auto a = mini::unique_ptr<int>(new int(10));
        auto b = mini::unique_ptr<int>(new int(20));

        a.swap(b);
        assert(*a == 20);
        assert(*b == 10);

        // member swap should be noexcept
        static_assert(noexcept(std::declval<mini::unique_ptr<int>&>().swap(std::declval<mini::unique_ptr<int>&>())));
    }

    // ===== noexcept policy checks =====
    {
        using UP = mini::unique_ptr<int>;
        static_assert(noexcept(std::declval<UP&>().release()));
        static_assert(noexcept(std::declval<UP&>().reset(nullptr))); // default_delete is noexcept
    }

    // ===== Custom deleter is called =====
    CustomDeleter::called = 0;
    {
        mini::unique_ptr<int, CustomDeleter> p(new int(42), CustomDeleter());
        assert(*p == 42);
    }
    assert(CustomDeleter::called == 1);

    // ===== get_deleter returns deleter =====
    {
        mini::unique_ptr<int, CustomDeleter> p(new int(1), CustomDeleter());
        CustomDeleter d = p.get_deleter();
        (void)d; // just check type
    }

    // ===== operator-> and operator* =====
    {
        auto p = mini::unique_ptr<int>(new int(99));
        assert(*p == 99);
        *p = 100;
        assert(*p == 100);
        assert(p.operator->() == p.get());
    }

    // ===== explicit operator bool =====
    {
        mini::unique_ptr<int> p;
        assert(!p);
        p.reset(new int(1));
        assert(p);
    }

    // ===== swap with empty =====
    {
        mini::unique_ptr<int> a(new int(5));
        mini::unique_ptr<int> b;
        a.swap(b);
        assert(!a);
        assert(b && *b == 5);
    }

    // ===== Array specialization: compile-time properties =====
    {
        static_assert(!std::is_copy_constructible_v<mini::unique_ptr<int[]>>);
        static_assert(!std::is_copy_assignable_v<mini::unique_ptr<int[]>>);
        // observers are noexcept where declared
        static_assert(noexcept(std::declval<const mini::unique_ptr<int[]>&>().get()));
        static_assert(noexcept(std::declval<const mini::unique_ptr<int[]>&>().operator bool()));
    }

    // ===== Array: construct from new int[10], read/write via operator[] =====
    {
        auto a = mini::unique_ptr<int[]>(new int[10]);
        assert(a);                        // operator bool()
        a[0] = 7;
        a[9] = 21;
        for (int i = 1; i < 9; ++i) a[i] = i;
        assert(a[0] == 7);
        assert(a[1] == 1);
        assert(a[8] == 8);
        assert(a[9] == 21);
        // get() returns the raw T*
        int* raw = a.get();
        assert(raw != nullptr);
    }

    // ===== Array: default-constructed is empty, becomes non-empty after take =====
    {
        mini::unique_ptr<int[]> a;        // empty
        assert(!a);
        a = mini::unique_ptr<int[]>(new int[3]);  // move-assign from temporary
        assert(a);
        a[0] = 1; a[1] = 2; a[2] = 3;
        assert(a[2] == 3);
    }

    // ===== Array: destructor uses delete[] (check via per-element dtor count) =====
    {
        DtorCount::dcount = 0;
        {
            auto a = mini::unique_ptr<DtorCount[]>(new DtorCount[3]);
            assert(a);
        }
        // All three elements must be destroyed
        assert(DtorCount::dcount == 3);
    }

    // ===== Array: move constructor transfers ownership & empties source =====
    {
        DtorCount::dcount = 0;
        mini::unique_ptr<DtorCount[]> a(new DtorCount[2]);
        assert(a);
        mini::unique_ptr<DtorCount[]> b(std::move(a));  // move-construct
        assert(!a);
        assert(b);
        // scope end: b destroys its 2 elements
        // (a has none)
        // verify after leaving scope below
    }
    assert(DtorCount::dcount == 2);

    // ===== Array: move assignment releases current array, then takes new =====
    {
        DtorCount::dcount = 0;
        mini::unique_ptr<DtorCount[]> a(new DtorCount[2]); // A
        mini::unique_ptr<DtorCount[]> b(new DtorCount[3]); // B
        // move-assign: B must first destroy its current 3, then take A's 2
        b = std::move(a);
        assert(!a);
        assert(b);
        // After assignment, the old 3 from b must be destroyed already
        assert(DtorCount::dcount == 3);
        // leaving scope now will destroy the 2 that b currently owns
    }
    // Total destroyed: 3 (on assign) + 2 (on scope exit) = 5
    assert(DtorCount::dcount == 5);

    return 0;
}
