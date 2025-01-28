set(LIB_UPDATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_UPDATE_SOURCE
  "${LIB_UPDATE_SOURCE_DIR}/upd/HttpApi.cpp"
)

set(LIB_UPDATE_HEADER
  "${LIB_UPDATE_SOURCE_DIR}/upd/HttpApi.h"
)

add_library(lib_update OBJECT ${LIB_UPDATE_SOURCE} ${LIB_UPDATE_HEADER})
set_target_properties(lib_update PROPERTIES AUTOMOC TRUE)
target_include_directories(lib_update PUBLIC ${LIB_UPDATE_SOURCE_DIR})
target_link_libraries(lib_update PUBLIC kdl Qt6::Widgets fmt::fmt nlohmann_json::nlohmann_json)

# use precompiled headers on CMake 3.16 or later
if (TB_ENABLE_PCH AND ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    message(STATUS "Using precompiled headers")
    target_precompile_headers(lib_update PUBLIC
            <algorithm>
            <any>
            <array>
            <cassert>
            <chrono>
            <cstddef>
            <cstdio>
            <cstdlib>
            <ctime>
            <filesystem>
            <functional>
            <initializer_list>
            <iterator>
            <iostream>
            <istream>
            <limits>
            <list>
            <map>
            <memory>
            <optional>
            <ostream>
            <ranges>
            <set>
            <string>
            <tuple>
            <type_traits>
            <unordered_map>
            <unordered_set>
            <utility>
            <variant>
            <vector>

            <QApplication>
            <QBoxLayout>
            <QCheckBox>
            <QDebug>
            <QEvent>
            <QtGlobal>
            <QGridLayout>
            <QHBoxLayout>
            <QLabel>
            <QLineEdit>
            <QObject>
            <QPushButton>
            <QString>
            <QStringBuilder>
            <QVBoxLayout>
            <QWidget>
    )
else()
    message(STATUS "Not using precompiled headers")
endif()

# Organize files into IDE folders
source_group(TREE "${LIB_UPDATE_SOURCE_DIR}" FILES ${LIB_UPDATE_SOURCE} ${LIB_UPDATE_HEADER})

set_compiler_config(lib_update)

add_subdirectory(test)
