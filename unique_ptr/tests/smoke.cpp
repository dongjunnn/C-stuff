#include "mini_unique_ptr.hpp"
#include <type_traits>
#include <cassert>

int main() {
    // default-constructed is empty
    mini::unique_ptr<int> p;
    (void)p; // silence unused if you haven’t added operator bool yet

    // copy operations must be deleted (compile-time checks)
    static_assert(!std::is_copy_constructible_v<mini::unique_ptr<int>>);
    static_assert(!std::is_copy_assignable_v<mini::unique_ptr<int>>);

    // (Optional) You can add destructor-behavior tests later; for now just compile.
    return 0;
}
