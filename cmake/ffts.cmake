# Build the ffts lib with the same config (release/debug, etc.) as the application.

message(" ")
message("### ffts config start ###")
message(" ")

# Default to debug if CMAKE_BUILD_TYPE is not set.

set(PROJECT_BUILD_TYPE ${CMAKE_BUILD_TYPE})
if(NOT PROJECT_BUILD_TYPE)
	set(PROJECT_BUILD_TYPE debug)
endif()
string(TOLOWER ${PROJECT_BUILD_TYPE} PROJECT_BUILD_TYPE)
message("PROJECT_BUILD_TYPE: ${PROJECT_BUILD_TYPE}")

include(ExternalProject)
if (WIN32)
	#TODO:
else()
	ExternalProject_Add(ffts
		SOURCE_DIR ${IMPORT_DIR}/ffts
		PREFIX ${IMPORT_DIR}/ffts
		INSTALL_DIR ""
		BUILD_IN_SOURCE 1
		CMAKE_ARGS
		"-DCMAKE_BUILD_TYPE=${PROJECT_BUILD_TYPE}"
		"-DCMAKE_C_COMPILER=/usr/bin/gcc"
		"-DCMAKE_CXX_COMPILER=/usr/bin/g++"
		BUILD_COMMAND
		cmake
		--build .
		--config ${PROJECT_BUILD_TYPE}
		INSTALL_COMMAND echo "no install"
		)
endif()

# Show where to find the lib
SET(ffts_lib_dir "${IMPORT_DIR}/ffts")
message (${ffts_lib_dir})

if (WIN32)
	SET(FFTS_STATIC ${CMAKE_SOURCE_DIR}/win32/libffts.lib
		)
else()
	SET(FFTS_STATIC
		optimized ${ffts_lib_dir}/libffts.a
		debug ${ffts_lib_dir}/libffts.a
		)
endif()

message(" ")
message("### ffts config done! ###")
message(" ")
