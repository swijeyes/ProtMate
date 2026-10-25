cmake_minimum_required(VERSION 3.21)
project(ProtMate VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release)
endif()

option(PMATE_BUILD_GUI "Build the Qt graphical front end" ON)
option(PMATE_BUILD_TESTS "Build the core unit tests" ON)
option(PMATE_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

# Hardening (carried over from the 1.1.6 Makefile)
if(MSVC)
	add_compile_options(/W4 /permissive- /GS /sdl)
	add_link_options(/guard:cf /DYNAMICBASE /NXCOMPAT)
else()
	add_compile_options(-Wall -Wextra -Wformat -Wformat-security -fstack-protector-strong)
	# FORTIFY_SOURCE needs optimisation, so leave it out of Debug builds
	add_compile_definitions($<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=2>)
	if(PMATE_SANITIZE)
		add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
		add_link_options(-fsanitize=address,undefined)
	endif()
endif()

# Shared calculations: standard C++ only
add_library(pmate_core STATIC core/pmate_core.cpp)
target_include_directories(pmate_core PUBLIC core)

# Terminal front end (POSIX and Windows)
add_executable(pmate cli/pmate_cli.cpp)
target_link_libraries(pmate PRIVATE pmate_core)
if(MINGW)
	# A single self-contained pmate.exe that runs without the MinGW runtime DLLs
	target_link_options(pmate PRIVATE -static)
endif()

if(PMATE_BUILD_TESTS)
	enable_testing()
	add_executable(core_tests tests/core_tests.cpp)
	target_link_libraries(core_tests PRIVATE pmate_core)
	add_test(NAME core_tests COMMAND core_tests)
endif()

# Graphical front end
if(PMATE_BUILD_GUI)
	find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
	qt_standard_project_setup()

	qt_add_executable(ProtMate
		gui/main.cpp
		gui/MainWindow.cpp
		gui/MainWindow.h
		gui/pages/Page.cpp
		gui/pages/Page.h
		gui/pages/Pages.h
		gui/pages/CalculatorPages.cpp
		gui/pages/SequencePages.cpp
		gui/pages/FilePages.cpp
		resources/resources.qrc
	)

	# Application icon: bundle icon on macOS, embedded icon resource on Windows
	if(APPLE)
		set(PMATE_ICNS ${CMAKE_CURRENT_SOURCE_DIR}/resources/ProtMate.icns)
		set_source_files_properties(${PMATE_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
		target_sources(ProtMate PRIVATE ${PMATE_ICNS})
		set_target_properties(ProtMate PROPERTIES MACOSX_BUNDLE_ICON_FILE ProtMate.icns)
	elseif(WIN32)
		target_sources(ProtMate PRIVATE resources/ProtMate.rc)
	endif()
	target_include_directories(ProtMate PRIVATE gui)
	target_link_libraries(ProtMate PRIVATE pmate_core Qt6::Widgets)
	target_compile_definitions(ProtMate PRIVATE QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)
	set_target_properties(ProtMate PROPERTIES
		AUTORCC ON
		WIN32_EXECUTABLE ON
		MACOSX_BUNDLE ON
		MACOSX_BUNDLE_BUNDLE_NAME "ProtMate"
		MACOSX_BUNDLE_GUI_IDENTIFIER "org.protmate.ProtMate"
		MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
		MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
	)

	include(GNUInstallDirs)
	install(TARGETS ProtMate BUNDLE DESTINATION . RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

include(GNUInstallDirs)
install(TARGETS pmate RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
