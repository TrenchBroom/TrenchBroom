# Naming
- Use camel case for class names: `MyUsefulClass`
- Functions, methods, variables and parameters use camel case and begin with a lowercase character:
  `void myUsefulFunction(const int aHelpfulParameter);`
- Private fields are prefixed with `m_`
- Constants use camel case and begin with an uppercase character:
  `static const int ThisIsAConstant = 1;`

# Formatting
- Indent with 4 spaces, no tabs
- No extra lines for opening braces:
  ```
  namespace … {
  class … {
  if (…) {
  …
  ```
- Use braces for 1-statement if / else blocks:
  ```
  if (…) {
      doThis();
  } else {
      doThat();
  }
   ```
- No inline if statements such as `if (…) doThis();`
- Avoid header files that declare more than one class

# Exceptions
- Functions should generally document their exception guarantee, but if a function can throw an exception, this is
  mandatory. There are [four levels of exception guarantee](https://en.cppreference.com/w/cpp/language/exceptions) 
  in C++:
  - *No guarantee*: If the function throws an exception, the program may not be in a valid state: resource leaks, memory
    corruption, or other invariant-destroying errors may have occurred.
  - *Basic guarantee*: If the function throws an exception, the program is in a valid state. No resources are leaked,
    and all objects' invariants are intact.
  - *Strong guarantee*: If the function throws an exception, the state of the program is rolled back to the state just
    before the function call. The strong guarantee implies the basic guarantee.
  - *Nothrow guarantee*: The function never throws exceptions. This is mandatory for destructors and other functions
    that may be called during stack unwinding, as well as swaps and move constructors.

## Handling Exceptions During Command Execution
It is mandatory that no exceptions reach the command processor. If an exception is caught by the command processor, it
will throw away the command that caused the exception, log an error message, and return false as if the command had
reported a failure to execute. However, this may or may not leave the program in an invalid state. 

This means that all command methods such must implement a nothrow guarantee. Exceptions must be handled and a valid
program state must be ensured if any function called by a command throws an exception.

# Types
- Only use `auto` when the type is obvious from surrounding visible code, e.g.:
  ```
  std::vector<Assets::Texture*> textures;
  …
  for (auto* texture : textures) {
      …
  ```
  
  Also OK: `auto textureCount = static_cast<int>(textures.size());`
- Don't do:  `auto mods = document->mods();`
  
  Instead, do: `StringList mods = document->mods();`
- The codebase often uses aliases for templates, vectors and smart pointers, e.g.:
  
  ```
  using vec3d = vec<double,3>;
  using MyClassList = std::vector<MyClass>;
  using MyClassPtr = std::shared_ptr<MyClass>;
  using String = std::string;
  ```
  
  Use these where they already exist, but for new code,
  use `std::vector<MyNewClass>` and `std::shared_ptr<MyNewClass>` directly,
  and only create aliases for verbose or deeply nested types. e.g.:
  
  ```
  using IntToListOfStringPairs = std::map<int, std::vector<std::pair<String, String>>>;
  ```

# Compilation Times

This section presents some guidelines to keep compilation times low.

## Avoid Including Headers

Avoid including headers in other headers. Remember that including a header B in another header A includes B in every
file that includes A, and so on.
- Use forward declarations wherever possible. Remember that you can forward declare classes, class templates, and
  scoped enums. Furthermore, type aliases and even template aliases can use forward declarations.
- Split off a part of a header into a separate header, e.g. declare some functions separately if they must include
  large headers. See the next item for examples involving std library headers.

Avoid including std library headers. Since names in std cannot be forward declared, this is sometimes unavoidable,
but there are some ways to mitigate the effects.
- Split off parts of a header into a separate header. Consider the following header `X.h`:
  ```
  #include <ostream>
  
  class X { ... }
  
  std::ostream& operator<<(std::ostream& s, const X& x) { ... }
  ```
  The stream insertion operator requires that `<ostream>` be included in `X.h`, with the effect that `<ostream>` is
  included wherever `X.h` is included, even if the stream insertion operator is not used in that place. In this case,
  it is useful to split off a header `X_IO.h` that declares the stream insertion operator while removing the 
  `<ostream>` include and the operator declaration from `X.h`.
  
  As an alternative when using the std library iostreams, you can include `<iosfwd>` in the header file. `<iosfwd>` has
  forward declarations for all iostream related types, so it can be used when stream operators are only declared and not
  implemented in your header file.
  
- Use a function template to avoid specifying the std library type directly. Consider the following example: 

  ```
  #include <set>
  
  class X {
      ...
  
      void doIt(const std::set& s); 
   }
  ```
  
  Here, the same problem arises: `<set>` is included everywhere where `X.h` is included. Since the member function
  declaration for `X::doIt` cannot easily be moved to another header, we might consider turning it into a function
  template:
  
  ```
  class X {
      ...
  
      template <typename S>
      void doIt(const S& s) { ... } 
   }
  ```
  
  Now, `<set>` must only be included at the call sites of `X::doIt`, which are often `cpp` files where includes do not
  propagate.
    
# Preinstantiated Templates
In certain cases involving templates, it might be useful to explicitly instantiate them if the template arguments are
known, if there is a small set of combinations of template arguments in use and if the function templates and class
template member functions need not be inlined by the compiler. Consider this example:

```
template <typename T, std::size_t S>
class vec<T,S> {...}
```

Assume that we are only ever using `vec<float,3>` and `vec<double,3>`. Then we can avoid instantiating the member
functions of these templates at every use site by preinstantiating the class templates. This requires creating a 
header file `vec_instantiation.h` and `vec_instantiation.cpp` with the following contents:

```
// vec_instantiation.h

#include "vec.h"

extern template class vec<float,3>;
extern template class vec<double,3>;
```

```
// vec_instantiation.cpp

#include "vec.h"

template class vec<float,3>;
template class vec<double,3>;
```

The compiler will still instantiate the class template `vec` wherever it is used, but depending on whether or not
its member functions are implemented inline, it will not instantiate the functions bodies. Specifically, if a
member function is implemented inline in the class template `vec`, then its function body will be instantiated by the
compiler to facilitate inlining. If the function body is implemented out of line, the compiler will not instantiate
it, but it will also not be able to inline it at the call site.

Therefore, using preinstiated templates requires careful consideration of performance, as it precludes function body
inlining by the compiler.

# Misc
- Unused function parameters should be commented out, e.g.:

  ```
  void myFunction(const int index, const int /* unusedParameter */) {...}
  ```
  
  Alternatively, it is allowed to mark them as unused in the function body if commenting them out is not
  possible. `Macros.h` contains a macro that can be used here:
   
  ```
  #include "Macros.h"
  
  void myFunction(const int index, const int unusedParameter = 0) {
      unused(unusedParameter);
  }
  ```
  
  If the semantics of the parameter is clear from the type alone, then it is also allowed to delete
  the parameter name:

  ```
  void myOtherFunction(const int index, const Model::World*) {...}
  ```
  
  In certain cases, it is more useful to use the `[[maybe_unused]]` attribute, esp. when the function
  body contains conditionally compiled code.
  
  In general, it is preferable to not delete the parameter name and it should be done with care.

# Features
- We use C++17
- The entire source code and test cases must compile without warnings.
- Everything that can be const should be const: methods, parameters and variables.
- Use RAII where possible.
- Avoid raw pointers unless they are confined to a method, class, or subsystem.
  Don't return raw pointers from public methods. Favor references and smart
  pointers over raw pointers.
