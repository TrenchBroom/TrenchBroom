set(LIB_UPDATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_UPDATE_SOURCE
  "${LIB_UPDATE_SOURCE_DIR}/update/Asset.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/FileUtils.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/GithubApi.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/HttpClient.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/InstallUpdate.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/Logging.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/QtHttpClient.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/Unzip.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateConfig.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateController.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateDialog.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateIndicator.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateInfo.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/update/Updater.cpp"
)

set(LIB_UPDATE_HEADER
  "${LIB_UPDATE_SOURCE_DIR}/update/Asset.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/FileUtils.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/GithubApi.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/HttpClient.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/InstallUpdate.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/Logging.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/Overload.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/QtHttpClient.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/Release.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/Unzip.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateConfig.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateController.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateDialog.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateIndicator.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/UpdateInfo.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/Updater.h"
  "${LIB_UPDATE_SOURCE_DIR}/update/Version.h"
)

if (WIN32)
  set(LIB_UPDATE_SCRIPT "${LIB_UPDATE_SOURCE_DIR}/scripts/install_update.bat" CACHE INTERNAL "Path to the update script")
elseif (LINUX)
  set(LIB_UPDATE_SCRIPT "${LIB_UPDATE_SOURCE_DIR}/scripts/install_update.sh" CACHE INTERNAL "Path to the update script")
elseif (APPLE)
  set(LIB_UPDATE_SCRIPT "${LIB_UPDATE_SOURCE_DIR}/scripts/install_update.sh" CACHE INTERNAL "Path to the update script")
endif()

add_library(UpdateLib STATIC ${LIB_UPDATE_SOURCE} ${LIB_UPDATE_HEADER})
set_target_properties(UpdateLib PROPERTIES AUTOMOC TRUE)
target_include_directories(UpdateLib PUBLIC ${LIB_UPDATE_SOURCE_DIR})
target_link_libraries(UpdateLib PUBLIC Qt6::Widgets Qt6::Network)

target_link_libraries(UpdateLib PRIVATE CompilerConfig)

add_subdirectory(test)
