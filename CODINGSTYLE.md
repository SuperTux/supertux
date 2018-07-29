# SuperTux Coding Standards

## Repository Structure

Avoid spaces at the end of lines. Files should always end with an empty newline.

Properly separate between generic engine code and game specific code whenever
feasible.

External and third party libraries are not allowed in `src/`. They go to
`external/`.

Use one file per class.

## Includes

The path in `#include` directives must not contain `..`. All paths must be
relative to the `src/` directory.

Conditional includes should be indented.

```c++
#ifdef FOOBAR
#  include "foobar.hpp"
#endif
```

Use the `#include <>` syntax for libraries in `external/`.

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

## Namespaces

Namespaces should be written in the form:

```c++
namespace NameSpace {
  // ...
} // namespace NameSpace
```

With no newline before the `{`.

## Compiler options

Compile with a maximum warning level and with `-Werror`:

```
-Werror -ansi -pedantic -Wall -Wextra -Wnon-virtual-dtor -Weffc++ -Wcast-qual
-Winit-self -Wno-unused-parameter
```

Possible additional flags for the future are:

```
-Wconversion -Wshadow
```

## Comments

Use the `//` syntax for regular comments, don't use `/* */`.

For translator information, use the `// l10n:` syntax.

For Doxygen (code documentation), use the `/** */` syntax, don't use `/**<` and
other styles of comments.

## Other Information

More info on good practices can be found in [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.html).
