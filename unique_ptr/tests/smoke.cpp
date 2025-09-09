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

    return 0;
}
