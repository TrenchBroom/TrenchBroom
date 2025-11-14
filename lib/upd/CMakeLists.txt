set(LIB_UPD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LIB_UPDATE_SOURCE
  "${LIB_UPD_SOURCE_DIR}/upd/Asset.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/FileUtils.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/GithubApi.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/HttpClient.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/InstallUpdate.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/Logging.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/QtHttpClient.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/Unzip.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateConfig.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateController.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateDialog.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateIndicator.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateInfo.cpp"
  "${LIB_UPD_SOURCE_DIR}/upd/Updater.cpp"
)

set(LIB_UPDATE_HEADER
  "${LIB_UPD_SOURCE_DIR}/upd/Asset.h"
  "${LIB_UPD_SOURCE_DIR}/upd/FileUtils.h"
  "${LIB_UPD_SOURCE_DIR}/upd/GithubApi.h"
  "${LIB_UPD_SOURCE_DIR}/upd/HttpClient.h"
  "${LIB_UPD_SOURCE_DIR}/upd/InstallUpdate.h"
  "${LIB_UPD_SOURCE_DIR}/upd/Logging.h"
  "${LIB_UPD_SOURCE_DIR}/upd/Overload.h"
  "${LIB_UPD_SOURCE_DIR}/upd/QtHttpClient.h"
  "${LIB_UPD_SOURCE_DIR}/upd/Release.h"
  "${LIB_UPD_SOURCE_DIR}/upd/Unzip.h"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateConfig.h"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateController.h"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateDialog.h"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateIndicator.h"
  "${LIB_UPD_SOURCE_DIR}/upd/UpdateInfo.h"
  "${LIB_UPD_SOURCE_DIR}/upd/Updater.h"
  "${LIB_UPD_SOURCE_DIR}/upd/Version.h"
)

if (WIN32)
  set(LIB_UPDATE_SCRIPT "${LIB_UPD_SOURCE_DIR}/scripts/install_update.bat" CACHE INTERNAL "Path to the update script")
elseif (LINUX)
  set(LIB_UPDATE_SCRIPT "${LIB_UPD_SOURCE_DIR}/scripts/install_update.sh" CACHE INTERNAL "Path to the update script")
elseif (APPLE)
  set(LIB_UPDATE_SCRIPT "${LIB_UPD_SOURCE_DIR}/scripts/install_update.sh" CACHE INTERNAL "Path to the update script")
endif()

add_library(lib-upd STATIC ${LIB_UPDATE_SOURCE} ${LIB_UPDATE_HEADER})
set_target_properties(lib-upd PROPERTIES AUTOMOC TRUE)
target_include_directories(lib-upd PUBLIC ${LIB_UPD_SOURCE_DIR})
target_link_libraries(lib-upd PUBLIC Qt6::Widgets Qt6::Network)

target_link_libraries(lib-upd PRIVATE compiler-config precompile-std-headers)

add_subdirectory(test)
