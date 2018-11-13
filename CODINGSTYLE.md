# SuperTux Coding Standards

## Repository Structure

Properly separate between generic engine code and game specific code whenever
feasible.

Third party libraries are not allowed in `src/`, they go to `external/`.

Third party libraries that are imported into `external/` as git
submodule have to be forked and included into the SuperTux
organisation on Github, not directly included from upstream.

## File Formating

Do not have spaces at the end of lines.

Files should always end with `/* EOF */` and a newline or a similar marker approprimate for
the given language.

Aim for one file per class, small helper classes in the same file are ok.

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

`tools/fix_include_guards.sh` is a little script that will help to fix
include guards on file renames.

## Variables

Prefix member variable names with `m_`, global variables with `g_`, and static
variables with `s_`.

## Classes

Properly separate data members and member functions. Do not mix them in the same
`public`/`private`/`protected` section.

The order of declarations in a class shall be as follows:

```c++
class Foo
{
public:
protected:
private:
   // type declarations, needs to come first as later stuff might depend on them

public:
protected:
private:
   // static stuff

public:
protected:
private:
   // constructors
   // destructor

public:
protected:
private:
   // member functions

public:
protected:
private:
   // member variables

private:
  // non-copyable footer
  Foo(const Foo&) = delete;
  Foo& operator=(const Foo&) = delete;
};
```

## Pointers

Do not use raw pointers and `new`/`delete`, use
`std::unique_ptr<>`/`std::make_unique<>` instead.

Only use `std::smart_ptr<>` when sharing of data is required, prefer
`std::unique_ptr<>` when possible.

Pass and return values as value, `&` or `const&`, only use `*` when
the value is expected to be `nullptr`.

Do not pass values as `const std::unique_ptr<T>&` or `const
std::shared_ptr<T>&`, dereference the pointer and pass as `const&`.

## Namespaces

Namespaces should be written in the form:

```c++
namespace my_namespace {
...
} // namespace my_namespace
```

With no newline before the `{`. Do not indent the content inside the
namespace. The namespace itself should be all lowercase.

## Compiler Warnings and Errors

Compile with a maximum warning level and with `-Werror`. This can be accomplished with:

```console
cmake .. -DCMAKE_BUILD_TYPE=Release  -DWARNINGS=ON -DWERROR=ON
```

This requires, among other things:

* use of `final` and `override` keywords

* use of `static_cast` and `reinterpret_cast`, not old style C casts

* all member variables have to be initialized in the constructor

* all `int`/`float` conversion has to be explicit

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

## Spaces

Use a space after `if`/`while`/`switch`/`for`:

`for (int i = 0; i < len; ++i) ...`

`if (a > b) ...`

`while (a > b) ...`

`switch (myenum) ...`

But don't use a space after a function name:

`myfunc ()` // don't do this

`myfunc()` // do this

## Other Information

More general info on good practices can be found in [Google's C++
Style Guide](https://google.github.io/styleguide/cppguide.html), note
however that we do not strictly follow that guide and divert from it
in some points.
