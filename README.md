# C-stuff gpt notes

“Enforce move-only semantics” means:
Your type cannot be copied (no duplicate owners), but it can be moved (ownership is transferred), and the moved-from object becomes empty but valid (e.g., holds nullptr) and can be destroyed/reset/reused safely.
This is exactly the ownership model std::unique_ptr uses to avoid double-deletes

std::unique_ptr has an explicit operator bool() const noexcept that answers one question:
“Do you currently own something?”

Conversion functions are user-defined implicit/explicit casts from your class type to some other type.