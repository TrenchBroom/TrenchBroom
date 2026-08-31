# stduuid declares cmake_minimum_required(VERSION 3.7.0), which triggers a
# deprecation warning under CMake 4. Treat it as requesting >= 3.10 to silence it.
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
CPMAddPackage("gh:mariusbancila/stduuid#v1.2.3")
unset(CMAKE_POLICY_VERSION_MINIMUM)
