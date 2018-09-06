# SuperTux Coding Standards

## File Formating

Do not have spaces at the end of lines.

Files should always end with `/* EOF */` and a newline or a similar marker approprimate for
the given language.

Aim for one file per class, small helper classes in the same file are ok.

## Repository Structure

Properly separate between generic engine code and game specific code whenever
feasible.

Third party libraries are not allowed in `src/`, they go to `external/`.

## Includes

The path in `#include` directives must not contain `..`. All paths must be
relative to the `src/` directory.

Conditional includes should be indented.

```c++
#ifdef FOOBAR
#  include "foobar.hpp"
#endif
```

Use the `#include <>` syntax for libraries in `external/`, use `cmake`
to set the include path properly.

Include guards are of the form:

```c++
#ifndef HEADER_SUPERTUX_{PATH}_{FILE}_HPP
#define HEADER_SUPERTUX_{PATH}_{FILE}_HPP
```

## Variables

Prefix member variable names with `m_`, global variables with `g_`, and static
variables with `s_`.

Properly separate data members and member functions. Do not mix them in the same
`public`/`private`/`protected` section.

## Pointers

Do not use raw pointers and `new` or `delete`. Use `std::unique_ptr<>` instead.

Keep use of `std::smart_ptr<>` to a minimum, prefer `std::unique_ptr<>` when possible.

Pass and return values as value, `&` or `const&`, don't use `*` unless absolutely necessary.

## Namespaces

Namespaces should be written in the form:

```c++
namespace my_namespace {
...
} // namespace my_namespace
```

With no newline before the `{`. Do not indent the content inside the
namespace. The namespace itself should be all lowercase.

## Compiler options

Use `final` and `override` keywords.

Use `static_cast` and `reinterpret_cast`, not old style C casts.

Compile with a maximum warning level and with `-Werror`. This can be accomplished with:

```console
cmake .. -DCMAKE_BUILD_TYPE=Release  -DWARNINGS=ON -DWERROR=ON
```

## Comments

Use the `//` syntax for regular comments, even multiline, don't use `/* */`.

For Doxygen (code documentation), use the `/** */` syntax, don't use `/**<` and
other styles of comments.

For translator information, use the `// l10n:` syntax.

Don't do `*` prefix decorations in comments, keep things simple and
compact:

```c++
/*
 *  Don't do this
 */
```

Instead:

```c++
// Do this
```

or:

```c++
/** Also ok when it's a Doxygen comment */
```

Avoid comments unless they explain something important and
non-obvious. Prefer to use good function and variable names to create
self-documenting code.


## Other Information

More general info on good practices can be found in [Google's C++
Style Guide](https://google.github.io/styleguide/cppguide.html), note
however that we do not strictly follow that guide and divert from it
in some points.
