if (COMPILER_IS_MSVC)
    set(LIB_STACKWALKER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
    set(LIB_STACKWALKER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
    set(LIB_STACKWALKER_SOURCE "${LIB_STACKWALKER_SOURCE_DIR}/StackWalker.cpp")

    add_library(stackwalker STATIC ${LIB_STACKWALKER_SOURCE})
    target_compile_options(stackwalker PRIVATE -w) # supress all warnings
    target_include_directories(stackwalker
            SYSTEM INTERFACE ${LIB_STACKWALKER_INCLUDE_DIR}
            PRIVATE ${LIB_STACKWALKER_INCLUDE_DIR})
endif()
