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
Follow these rules to keep compilation times as low as possible:
- Avoid including headers in other headers. Remember that including a header B in another header A includes B in every
  file that includes A. This can really lead to an explosion of inclusions because this process is recursive.
  - Use forward declarations instead.
  - Split off a part of a header into a separate header, e.g. declare some functions separately if they must include
    large headers. See the next item for more details.
- Avoid including std library headers. Since names in std cannot be forward declared, this is sometimes unavoidable,
  but there are some ways to mitigate the effects.
  - Split off parts of a header into a separate header. As an example, take a header `X.h` that declares some class `X` 
    along with `std::ostream& operator<<(std::ostream& s, const X& x)`. This output operator requires that `<ostream>` 
    be included `X.h`. This leads to a lot of includes of `<ostream>` wherever `X.h` is included, even if the stream 
    operators are only used in one place. In such a case, it might be useful to split off a header `X_IO.h` that
    declares the IO operators. Then, `<ostream>` can be removed from `X.h`.

# Misc
- Unused function parameters should be commented out, e.g.:

  ```
  void myFunction(const int index, const int /* unusedParameter */) {...}
  ```
  
  Alternatively, it is allowed to mark them as unused in the function body if commenting them out is not
  possible. 'Macros.h' contains a macro that can be used here:
   
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
