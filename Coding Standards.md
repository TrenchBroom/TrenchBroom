# Preface
The following rules are updated constantly. When we update a rule, we do not update the entire codebase, so you can expect to find existing code that violates these rules. We are thankful if you take the time to update the existing code to the current set of rules. If you do that, it would be helpful for code reviewers if you did the code style changes to existing code in a separate commit at the top of your branch, titled "Cleanup: code style".

# Naming
- Use camel case for class names: `MyUsefulClass`
- Functions, methods, variables and parameters use camel case and begin with a lowercase character:
  `void myUsefulFunction(const int aHelpfulParameter);`
- Private fields are prefixed with `m_`
- Constants use camel case and begin with an uppercase character:
  `static const int ThisIsAConstant = 1;`

# Formatting
- We use clang-format version 13 to format the code. The formatting rules are in the `.clang-format` file at the root of the repository. Correct formatting is enforced by CI.

# Code Style
- We use clang-tidy to help with certain style rules. The style rules are in the `.clang-tidy` file at the root of the repository. These rules are not (yet) enforced by CI, but we encourage every contributor to adhere to them.
- Avoid header files that declare more than one class
- Class members are usually ordered as follows, but it is ok to deviate from this if there is a good reason.
  1. Type aliases and static const members.
  2. Member variables
  3. Constructors / destructors
  4. Operators
  5. Public member functions
  6. Protected member functions
  7. Private member functions
  8. Extension interface (private pure virtual member functions)
- We follow the "almost always auto" style, so we use `auto` wherever possible except doing so would make the code very awkward. These exceptions are very rare.
- Use `auto*` when declaring a pointer variable, e.g. `auto* entityNode = new EntityNode{...}`.
- Use `const` wherever possible.
- We follow a left-to-right style for declarations. For example, when declaring and initializing a variable, we write `auto entity = Entity{...};` instead of `Entity entity{...}`.
- Use private namespaces for implementation details in cpp files. If possible, avoid private member functions for helper functions - prefer free functions in an anonymous namespace in the cpp file for this.

# Features
- We use C++17.
- The entire source code and test cases must compile without warnings.
- Everything that can be const should be const: methods, parameters and variables.
- Move semantics should be used when it makes sense from a performance perspective. Generally, parameters should be taken by const reference, const pointer or by value rather than by rvalue reference or forwarding reference. If a function parameter can be moved, take it by value and move it in the function.
- Use RAII where possible.
- Avoid raw pointers unless they are confined to a method, class, or subsystem.
  Don't return raw pointers from public methods. Favor references and smart
  pointers over raw pointers.

# Compilation Times

This section presents some guidelines to keep compilation times low.

## Avoid Including Headers

Avoid including headers in other headers. Remember that including a header B in another header A includes B in every file that includes A, and so on.
- Use forward declarations wherever possible. Remember that you can forward declare classes, class templates, and
  scoped enums. Furthermore, type aliases and even template aliases can use forward declarations.
- If you need to refer to a type in a header file, you should use a forward declaration except for the following reasons:
  1. It's a typed defined in namespace `std`.
  2. The type is used as a by value member of a class that you are declaring in your header.
  3. The type is used in the implementation of a template function.
  - Note that you should also use forward declarations for types used in the following contexts:
    - By value function parameters or return types - it is ok to use a fwd declaration, the type need only be fully defined the call site, where it can be included.
    - Type parameters for standard library containers - if you are declaring a member of type `std::vector<Foo>` in a class, you need not include `Foo` in the header file where your class is declared. You may have to declare a destructor which you must implement in the corresponding cpp file, where `Foo` must be fully defined. Defaulting the constructor in the cpp file is sufficient.
