/*
 * frontend_types_namespace_import.hpp
 *
 *  Created on: Dec 15, 2014
 *      Author: rafa
 */

//------------------------------------------------------------------------------
inline ufo::deep_copy_bytes bytes (void* mem, ufo::uword size)
{
    assert (mem && size);
    ufo::deep_copy_bytes b;
    b.mem  = mem;
    b.size = size;
    return b;
}
//------------------------------------------------------------------------------
inline ufo::deep_copy_string deep_copy(
         const char* mem, ufo::uword size_no_null_term
         )
{
    assert (mem && size_no_null_term);
    assert (mem[size_no_null_term - 1] != 0);
    ufo::deep_copy_string s;
    s.mem  = mem;
    s.size = size_no_null_term;
    return s;
}
//------------------------------------------------------------------------------
inline ufo::deep_copy_string deep_copy (const char* str)                             //you should be avoiding strlen by taking the overload that takes the size if possible, this function is just written for cases where there really is something better.
{
    assert (str);
    return deep_copy (str, strlen (str));
}
//------------------------------------------------------------------------------
inline ufo::deep_copy_string deep_copy (const std::string& str)
{
    assert (str.size());
    return (deep_copy (&str[0], str.size()));
}
//------------------------------------------------------------------------------
inline ufo::deep_copy_string deep(
        const char* mem, ufo::uword size_no_null_term
        )
{
    return deep_copy (mem, size_no_null_term);
}
//------------------------------------------------------------------------------
inline ufo::deep_copy_string deep (const char* str)                             //you should be avoiding strlen by taking the overload that takes the size if possible, this function is just written for cases where there really is something better.
{
    return deep_copy (str);
}
//------------------------------------------------------------------------------
inline ufo::deep_copy_string deep (const std::string& str)
{
    return deep_copy (str);
}
//------------------------------------------------------------------------------
inline ufo::literal_wrapper lit (const char* literal)                                //this is because: 1-I don't want the user to remember that the const char* must point to a decayed literal 2-make clear when to print a pointer
{
    assert (literal);
    ufo::literal_wrapper l;
    l.lit = literal;
    return l;
}
//------------------------------------------------------------------------------
inline ufo::ptr_wrapper ptr (const void* pointer)                                //The pointers have to be marked explicitly, I could add a modifier in the fmt string (less verbose), but the code has to work in compilers that can't run the compile time validation
{
    ufo::ptr_wrapper l;
    l.ptr = pointer;
    return l;
}
//------------------------------------------------------------------------------
