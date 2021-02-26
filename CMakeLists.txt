cmake_minimum_required(VERSION 3.1)
project(luaopus)

option(BUILD_SHARED_LIBS "Build modules as shared libraries" ON)

find_package(PkgConfig)
include(FindPackageHandleStandardArgs)

if(LUA_VERSION)
  find_package(Lua ${LUA_VERSION} EXACT REQUIRED)
else()
  find_package(Lua REQUIRED)
  set(LUA_VERSION "${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}" CACHE STRING "Lua version to use")
endif()

pkg_check_modules(PC_OPUS QUIET opus)
find_path(OPUS_INCLUDE_DIR opus/opus.h
  HINTS ${PC_OPUS_INCLUDEDIR} ${PC_OPUS_INCLUDE_DIRS})
find_library(OPUS_LIBRARY NAMES opus libopus
  HINTS ${PC_OPUS_LIBDIR} ${PC_OPUS_LIBRARY_DIRS})
find_package_handle_standard_args(LibOpus DEFAULT_MSG OPUS_LIBRARY OPUS_INCLUDE_DIR)
set(OPUS_LIBRARIES ${OPUS_LIBRARY})
set(OPUS_INCLUDE_DIRS ${OPUS_INCLUDE_DIR})

set(CMODULE_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/lua/${LUA_VERSION}")
set(LUAMODULE_INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/share/lua/${LUA_VERSION}")

configure_file(
  "src/luaopus/version.lua"
  "${CMAKE_BINARY_DIR}/luaopus/version.lua"
  COPYONLY
)

install(FILES "src/luaopus/version.lua"
  DESTINATION "${LUAMODULE_INSTALL_LIB_DIR}/luaopus/"
)

set(luaopus_sources)
list(APPEND luaopus_sources "csrc/luaopus.c")
list(APPEND luaopus_sources "csrc/luaopus_internal.c")
list(APPEND luaopus_sources "csrc/luaopus_defines.c")
list(APPEND luaopus_sources "csrc/luaopus_encoder.c")
list(APPEND luaopus_sources "csrc/luaopus_decoder.c")

add_library(luaopus ${luaopus_sources})

target_link_libraries(luaopus PRIVATE ${OPUS_LIBRARIES})
target_link_directories(luaopus PRIVATE ${OPUS_LIBRARY_DIRS})
if(WIN32)
    target_link_libraries(luaopus PRIVATE ${LUA_LIBRARIES})
endif()
target_include_directories(luaopus PRIVATE ${OPUS_INCLUDEDIR})
target_include_directories(luaopus PRIVATE ${LUA_INCLUDE_DIR})

if(APPLE)
    set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -undefined dynamic_lookup")
    if(BUILD_SHARED_LIBS)
        set_target_properties(luaopus PROPERTIES SUFFIX ".so")
    endif()
endif()

set_target_properties(luaopus PROPERTIES PREFIX "")
set_target_properties(luaopus PROPERTIES OUTPUT_NAME "luaopus")
set_target_properties(luaopus PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(luaopus PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(luaopus PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set_target_properties(luaopus PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
  set_target_properties(luaopus PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
  set_target_properties(luaopus PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}")
endforeach()

install(TARGETS luaopus
  LIBRARY DESTINATION "${CMODULE_INSTALL_LIB_DIR}"
  RUNTIME DESTINATION "${CMODULE_INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${CMODULE_INSTALL_LIB_DIR}"
)
