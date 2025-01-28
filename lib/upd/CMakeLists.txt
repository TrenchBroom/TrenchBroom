set(LIB_UPDATE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_UPDATE_SOURCE
  "${LIB_UPDATE_SOURCE_DIR}/upd/Asset.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/FileUtils.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/GithubApi.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/HttpClient.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/InstallUpdate.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/Logging.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/QtHttpClient.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/Unzip.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateConfig.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateController.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateDialog.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateIndicator.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateInfo.cpp"
  "${LIB_UPDATE_SOURCE_DIR}/upd/Updater.cpp"
)

set(LIB_UPDATE_HEADER
  "${LIB_UPDATE_SOURCE_DIR}/upd/Asset.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/FileUtils.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/GithubApi.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/HttpClient.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/InstallUpdate.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/Logging.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/Overload.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/QtHttpClient.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/Release.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/Unzip.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateConfig.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateController.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateDialog.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateIndicator.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/UpdateInfo.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/Updater.h"
  "${LIB_UPDATE_SOURCE_DIR}/upd/Version.h"
)

if (WIN32)
  set(LIB_UPDATE_SCRIPT "${LIB_UPDATE_SOURCE_DIR}/scripts/install_update.bat" CACHE INTERNAL "Path to the update script")
elseif (LINUX)
  set(LIB_UPDATE_SCRIPT "${LIB_UPDATE_SOURCE_DIR}/scripts/install_update.sh" CACHE INTERNAL "Path to the update script")
elseif (APPLE)
  set(LIB_UPDATE_SCRIPT "${LIB_UPDATE_SOURCE_DIR}/scripts/install_update.sh" CACHE INTERNAL "Path to the update script")
endif()

add_library(upd ${LIB_UPDATE_SOURCE} ${LIB_UPDATE_HEADER})
set_target_properties(upd PROPERTIES AUTOMOC TRUE)
target_include_directories(upd PUBLIC ${LIB_UPDATE_SOURCE_DIR})
target_link_libraries(upd PUBLIC Qt6::Widgets Qt6::Network)

set_compiler_config(upd)

add_subdirectory(test)
