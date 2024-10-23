cmake_minimum_required(VERSION 3.6)
project(gd-mqtt-com)

option(BUILD_FOR_EDITOR "If true, create and install libs as release ready to use in the demo folder" FALSE)

if(DEFINED GODOT_GDEXTENSION_DIR)
	set(GODOT_GDEXTENSION_DIR ${GODOT_GDEXTENSION_DIR} CACHE STRING "Path to GDExtension interface header directory")
else()
	set(GODOT_GDEXTENSION_DIR godot-cpp/gdextension/ CACHE STRING "Path to GDExtension interface header directory")
endif()
message(STATUS "GODOT_GDEXTENSION_DIR: ${GODOT_GDEXTENSION_DIR}")

if(DEFINED CPP_BINDINGS_PATH)
	set(CPP_BINDINGS_PATH ${CPP_BINDINGS_PATH} CACHE STRING "Path to C++ bindings")
else()
	set(CPP_BINDINGS_PATH godot-cpp CACHE STRING "Path to C++ bindings")
endif()
message(STATUS "CPP_BINDINGS_PATH: ${CPP_BINDINGS_PATH}")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(TARGET_PATH x11)
	set(LIB_EXT ".so")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(TARGET_PATH win64)
	set(PAHO_MQTT_C_LIBRARIES "C:/Program Files (x86)/Eclipse Paho C/lib/paho-mqtt3as-static.lib" CACHE PATH "Paho C libraries directory override" FORCE)
	set(PAHO_MQTT_C_INCLUDE_DIRS "C:/Program Files (x86)/Eclipse Paho C/include/" CACHE PATH "Paho C includes directory override" FORCE)
	set(PahoMqttCpp_DIR "C:/Program Files (x86)/paho-mqtt-cpp/lib/cmake/PahoMqttCpp" CACHE PATH "Paho CPP directory override" FORCE)
	set(LIB_EXT ".dll")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(TARGET_PATH macos)
	set(LIB_EXT ".dylib")

else()
	message(FATAL_ERROR "Not implemented support for ${CMAKE_SYSTEM_NAME}")
endif()

message(STATUS "Configured support for ${CMAKE_SYSTEM_NAME} system.")

# Change the output directory to the bin directory
set(BUILD_PATH ${CMAKE_SOURCE_DIR}/bin/${TARGET_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BUILD_PATH}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BUILD_PATH}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BUILD_PATH}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BUILD_PATH}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BUILD_PATH}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BUILD_PATH}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BUILD_PATH}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BUILD_PATH}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BUILD_PATH}")

# Set the c++ standard to c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GODOT_COMPILE_FLAGS )
set(GODOT_LINKER_FLAGS )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /WX") # /GF /MP
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /DTYPED_METHOD_BIND")

	if(CMAKE_BUILD_TYPE MATCHES Debug)
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /MDd") # /Od /RTC1 /Zi
	else()
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /MD /O2") # /Oy /GL /Gy
		STRING(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
	endif(CMAKE_BUILD_TYPE MATCHES Debug)

	# Disable conversion warning, truncation, unreferenced var, signed mismatch
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /wd4244 /wd4305 /wd4101 /wd4018 /wd4267")

	add_definitions(-DNOMINMAX)

	# Unkomment for warning level 4
	#if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
	#	string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	#endif()

else()

#elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	# using Clang
#elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# using GCC and maybe MinGW?

	if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		set(GODOT_LINKER_FLAGS "-Wl")
	else()
		set(GODOT_LINKER_FLAGS "-static-libgcc -static-libstdc++ -Wl,-R,'$$ORIGIN'")
	endif()

	# Hmm.. maybe to strikt?
	set(GODOT_COMPILE_FLAGS "-fPIC -g -Wwrite-strings")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wchar-subscripts -Wcomment -Wdisabled-optimization")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wformat -Wformat=2 -Wformat-security -Wformat-y2k")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wimport -Winit-self -Winline -Winvalid-pch -Werror")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wmissing-braces -Wmissing-format-attribute")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wpointer-arith")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wredundant-decls -Wreturn-type -Wsequence-point")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wswitch -Wswitch-enum -Wtrigraphs")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wuninitialized -Wunknown-pragmas -Wunreachable-code -Wunused-label")
	set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wunused-value -Wvariadic-macros -Wvolatile-register-var -Wno-error=attributes")

	# -Wshadow -Wextra -Wall -Weffc++ -Wfloat-equal -Wstack-protector -Wunused-parameter -Wsign-compare -Wunused-variable -Wcast-align
	# -Wunused-function -Wstrict-aliasing -Wstrict-aliasing=2 -Wmissing-field-initializers

	if(NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -Wno-ignored-attributes")
	endif()

	if(CMAKE_BUILD_TYPE MATCHES Debug)
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -fno-omit-frame-pointer -O0")
	else()
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -O3")
	endif(CMAKE_BUILD_TYPE MATCHES Debug)
endif()

# Disable exception handling. Godot doesn't use exceptions anywhere, and this
# saves around 20% of binary size and very significant build time (GH-80513).
option(GODOT_DISABLE_EXCEPTIONS OFF "Force disabling exception handling code")
if (GODOT_DISABLE_EXCEPTIONS)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -D_HAS_EXCEPTIONS=0")
	else()
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} -fno-exceptions")
	endif()
else()
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(GODOT_COMPILE_FLAGS "${GODOT_COMPILE_FLAGS} /EHsc")
	endif()
endif()

##################################################################################
if(DEFINED STATIC_PAHO_MQTT)
	# Define static linking
	set(CMAKE_EXE_LINKER_FLAGS "-static")
	# TODO envolope for windows build
	set(PAHO_MQTT_INCLUDE_DIR
		${CMAKE_SOURCE_DIR}/thirdparty/paho.mqtt.cpp/install/include
	)
	set(PAHO_MQTT_LIBRARIES
		#${CMAKE_SOURCE_DIR}/thirdparty/paho.mqtt.cpp/install/lib/libpaho-mqtt3a.a
		${CMAKE_SOURCE_DIR}/thirdparty/paho.mqtt.cpp/install/lib/libpaho-mqtt3as.a
		#${CMAKE_SOURCE_DIR}/thirdparty/paho.mqtt.cpp/install/lib/libpaho-mqtt3c.a
		${CMAKE_SOURCE_DIR}/thirdparty/paho.mqtt.cpp/install/lib/libpaho-mqtt3cs.a
		${CMAKE_SOURCE_DIR}/thirdparty/paho.mqtt.cpp/install/lib/libpaho-mqttpp3.a
		-lssl -lcrypto 
		-ldl -lpthread
	)
else()
	find_package(PahoMqttCpp REQUIRED)
	set(PAHO_MQTT_LIBRARIES
		PahoMqttCpp::paho-mqttpp3
		/usr/lib/x86_64-linux-gnu/libpaho-mqtt3as.a
		/usr/lib/x86_64-linux-gnu/libpaho-mqtt3cs.a
		-lssl -lcrypto 
		-ldl -lpthread
	)
endif()
##################################################################################
set(libmqtt_com mqtt-com)
add_library(${libmqtt_com} src/mqtt_com/mqtt_com.cpp)
target_include_directories(${libmqtt_com} SYSTEM PRIVATE
	${CMAKE_SOURCE_DIR}/include
	${PAHO_MQTT_INCLUDE_DIR}
)
target_link_libraries(${libmqtt_com}
	${PAHO_MQTT_LIBRARIES}
)

set(test_mqtt_com test_mqtt_com)
add_executable(${test_mqtt_com} tests/test_mqtt_com.cpp)
target_include_directories(${test_mqtt_com} SYSTEM PRIVATE
	${CMAKE_SOURCE_DIR}/include
	${PAHO_MQTT_INCLUDE_DIR}
)
target_link_libraries(${test_mqtt_com} 
	${libmqtt_com}
)

# Get Sources
file(GLOB_RECURSE SOURCES src/*.c**)
file(GLOB_RECURSE HEADERS include/*.h**)

# Define our godot-cpp library
#add_library(${PROJECT_NAME} SHARED src/gd_mqtt_com.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
	${CMAKE_SOURCE_DIR}/include
	${CPP_BINDINGS_PATH}/include
	${CPP_BINDINGS_PATH}/build/gen/include
	${GODOT_GDEXTENSION_DIR}
	${PAHO_MQTT_INCLUDE_DIR}
)

# Create the correct name (godot.os.build_type.system_bits)
# Synchronized with godot-cpp's CMakeLists.txt

set(BITS 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BITS 64)
endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(GODOT_CPP_BUILD_TYPE Debug)
else()
	set(GODOT_CPP_BUILD_TYPE Release)
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME)
string(TOLOWER ${GODOT_CPP_BUILD_TYPE} BUILD_TYPE)

if(ANDROID)
	# Added the android abi after system name
	set(SYSTEM_NAME ${SYSTEM_NAME}.${ANDROID_ABI})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_directories(${PROJECT_NAME}
		PRIVATE
		${CPP_BINDINGS_PATH}/bin/Debug # "Debug" Always seems to be a bug on Windows
	)
	target_link_libraries(${PROJECT_NAME}
		godot-cpp.${SYSTEM_NAME}.${BUILD_TYPE}$<$<NOT:$<PLATFORM_ID:Android>>:.${BITS}>
		${PAHO_MQTT_LIBRARIES}
	)
else()
	target_link_directories(${PROJECT_NAME}
		PRIVATE
		${CPP_BINDINGS_PATH}/build/bin/
	)
	target_link_libraries(${PROJECT_NAME}
		#libgodot-cpp.linux.release.64.a
		libgodot-cpp.${SYSTEM_NAME}.${BUILD_TYPE}$<$<NOT:$<PLATFORM_ID:Android>>:.${BITS}>.a
		${PAHO_MQTT_LIBRARIES}
	)
endif()
	
	# Add the compile flags
	set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS ${GODOT_COMPILE_FLAGS})
	set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS ${GODOT_LINKER_FLAGS})
if(CMAKE_BUILD_TYPE MATCHES Debug)
	set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME "${PROJECT_NAME}.debug")
else()
	set_property(TARGET ${PROJECT_NAME} PROPERTY OUTPUT_NAME "${PROJECT_NAME}.release")
endif()

# Copy a fake debug
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${BUILD_PATH}/${PROJECT_NAME}.release${LIB_EXT} ${BUILD_PATH}/${PROJECT_NAME}.debug${LIB_EXT}
	)
else()
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${BUILD_PATH}/lib${PROJECT_NAME}.release${LIB_EXT} ${BUILD_PATH}/lib${PROJECT_NAME}.debug${LIB_EXT}
	)
endif()
