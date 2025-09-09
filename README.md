# C-stuff gpt notes

“Enforce move-only semantics” means:
Your type cannot be copied (no duplicate owners), but it can be moved (ownership is transferred), and the moved-from object becomes empty but valid (e.g., holds nullptr) and can be destroyed/reset/reused safely.
This is exactly the ownership model std::unique_ptr uses to avoid double-deletes

std::unique_ptr has an explicit operator bool() const noexcept that answers one question:
“Do you currently own something?”

Conversion functions are user-defined implicit/explicit casts from your class type to some other type.


These are the special member functions in C++:

Default constructor T()
Destructor ~T()
Copy constructor T(const T&)
Copy assignment operator T& operator=(const T&)
Move constructor T(T&&)
Move assignment operator T& operator=(T&&)

The compiler will generate these for you if you don’t declare them (with some rules). Together they’re often called the “big five” (used to be “big three” before move semantics).