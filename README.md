# String Views in C
Get rid of annoying null-termination and memory allocation. Especially useful for networking.

## How to use
To use a header-only library simple include `string_view.h` like any normal header. Then pick on C File to define `STRING_VIEW_IMPLEMENTATION` before the include.
```c
// in main.c
#define STRING_VIEW_IMPLEMENTATION
#include "string_view.h"

int main() {
  ...
  return 0;
}

// in the rest of the source files
#include "string_view.h"
...
```

## The Type
A StringView is an immutable string.
```c
typedef struct {
  const char* data; // Start of the string
  size_t length; // Length
}
```
Instead of ending at a null-terminator the StringViews knows it's length and `data` does not have to be null-terminated.
Plus since it's lifetime is bound to `data`, we have no extra memory allocations.

## Displaying and Creating
To create a StringView there are 3 options.

```c
// 1. Manual
StringView sv = { .data = <some str>, .length = <some number> };

// 2. SV_LIT macro for string literals. (No strlen runtime overhead)
StringView sv = SV_LIT("Hello World");

// 3. SV_STR macro for runtime cstrings.
char buf[256];
scanf("%s", buf);
StringView sv = SV_STR(buf);
```

StringViews can be printed using printf via the SV_FMT and SV_ARG macros.
```c
printf("This is a StringView: " SV_FMT "\n", SV_ARG(sv);
```

## Useful Functions 
Most of these function would normally require allocating a new buffer with regular cstrings.
* Trimming Whitespaces 
* Comparison - case sensitive and insensitive
* Making substrings
* Parsing to a signed / unsigned number
* Chopping the end of a string
