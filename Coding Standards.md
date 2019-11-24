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
- We use C++17.
- The entire source code and test cases must compile without warnings.
- Everything that can be const should be const: methods, parameters and variables.
- Move semantics should be used when it makes sense from a performance perspective. Generally, parameters should be
  taken by const reference, const pointer or by value rather than by rvalue reference or forwarding reference.
- Use RAII where possible.
- Avoid raw pointers unless they are confined to a method, class, or subsystem.
  Don't return raw pointers from public methods. Favor references and smart
  pointers over raw pointers.
