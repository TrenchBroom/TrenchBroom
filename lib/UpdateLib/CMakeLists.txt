project(UpdateLib VERSION 0.1.0 LANGUAGES CXX)

add_library(UpdateLib STATIC)

target_sources(UpdateLib
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Asset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GithubApi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HttpClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InstallUpdate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logging.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QtHttpClient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Unzip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UpdateConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UpdateController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UpdateDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UpdateIndicator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UpdateInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Updater.cpp

  PUBLIC FILE_SET headers TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/Asset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/FileUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/GithubApi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/HttpClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/InstallUpdate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/Logging.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/QtHttpClient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/Unzip.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/UpdateConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/UpdateController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/UpdateDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/UpdateIndicator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/UpdateInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/update/Updater.h
)

target_link_libraries(UpdateLib PRIVATE CompilerConfig)
target_link_libraries(UpdateLib PUBLIC Qt6::Widgets Qt6::Network)

set_target_properties(UpdateLib PROPERTIES AUTOMOC TRUE)

if (WIN32)
  set(LIB_UPDATE_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/install_update.bat" CACHE INTERNAL "Path to the update script")
elseif (LINUX)
  set(LIB_UPDATE_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/install_update.sh" CACHE INTERNAL "Path to the update script")
elseif (APPLE)
  set(LIB_UPDATE_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/install_update.sh" CACHE INTERNAL "Path to the update script")
endif()

add_subdirectory(test)
