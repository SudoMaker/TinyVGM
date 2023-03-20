cmake_minimum_required(VERSION 3.2)
project(TinyVGM
LANGUAGES C
HOMEPAGE_URL https://github.com/SudoMaker/TinyVGM
DESCRIPTION "A lightweight library for parsing the VGM format"
VERSION 1.0.2)

set(LIB_VERSION_MAJOR 1)
set(LIB_VERSION_MINOR 0)
set(LIB_VERSION_PATCH 2)
set(LIB_VERSION_STRING ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH})

set(CMAKE_C_STANDARD 99)
include(GNUInstallDirs)

#
# TinyVGMConfig.cmake File
#
include(CMakePackageConfigHelpers)
configure_package_config_file(
	cmake/TinyVGMConfig.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/TinyVGMConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TinyVGM"
)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/TinyVGMConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

add_library(TinyVGM TinyVGM.c TinyVGM.h)
target_include_directories(TinyVGM
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


set_target_properties(TinyVGM PROPERTIES
	VERSION ${LIB_VERSION_STRING} SOVERSION ${LIB_VERSION_MAJOR}
	C_VISIBILITY_PRESET hidden)

IF(BUILD_EXAMPLES)
	add_executable(TinyVGM_Example example.c)
	target_link_libraries(TinyVGM_Example TinyVGM)
endif()

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/TinyVGM.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/TinyVGM.pc"
	@ONLY
)
install(FILES
	TinyVGM.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
install(TARGETS TinyVGM
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	EXPORT TinyVGMTargets
)
install(
	EXPORT TinyVGMTargets
	FILE TinyVGMTargets.cmake
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TinyVGM"
	NAMESPACE TinyVGM::
)
install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/TinyVGMConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/TinyVGMConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/TinyVGM"
)
