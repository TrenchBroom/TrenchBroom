# Naming
- Use camel case for class names: MyUsefulClass
- Functions, methods, variables and parameters use camel case and begin with a lowercase character:
  void myUsefulFunction(const int aHelpfulParameter);
- Private fields are prefixed with m_
- Constants use camel case and begin with an uppercase character:
  static const int ThisIsAConstant = 1;

# Formatting
- No extra lines for opening braces:
  namespace … {
  class … {
  if (…) {
  …
- No braces for 1-statement if / else blocks:
  if (…)
    doThis();
  else
    doThat();
- No inline if statements such as
  if (…) doThis();
- Avoid header files that declare more than one class.
- Make typedefs for templates, vectors and smart pointers, e.g.
typedef Vec<double,3> Vec3d;
typedef std::vector<MyClass> MyClassList;
typedef std::shared_ptr<MyClass> MyClassPtr;

# Features
- The entire source code and test cases must compile without warnings.
- Don't use C++13 features.
- Everything that can be const should be const: methods, parameters and variables.
- Use RAII where possible.
- Avoid raw pointers unless they are confined to a method, class, or subsystem. Don't return raw pointers from public methods. Favor references and smart pointers over raw pointers.
- Favor iterators over indices in loops:
  const MyClassList& list = …;
  MyClassList::iterator it, end;
  for (it = list(), end = list(); it != end; ++it) {
    const MyClass& myObject = *it;
    …
  }
