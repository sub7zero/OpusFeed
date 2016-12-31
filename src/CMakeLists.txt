cmake_minimum_required(VERSION 3.0)
project(OpusFeed VERSION 1.0 LANGUAGES C CXX )
#---
include_directories(../3rdparty/include)
link_directories(../3rdparty/lib)
include_directories(./)
add_executable(OpusFeed
					OpusFeed.cpp
					Feed.cpp
					FileUtil.cpp
					StrUtil.cpp
					OpusConverter.cpp
					Downloader.cpp
					ByteArray.cpp
					ConsoleColor.cpp
					Log.cpp
					Regex.cpp
					RFC822Time.cpp
					Tree.cpp
					SqliteDB.cpp
					SqliteStatement.cpp
					WinResources.rc)
target_link_libraries(OpusFeed
							avformat
							avcodec
							avutil
							avresample
							swscale
							opus
							curl
							ssl
							crypto
							z
							sqlite3
							pcre
							tinyxml2)
if (WIN32)
	target_link_libraries(OpusFeed
							ws2_32
							wldap32
							gdi32
							Psapi)
else()
	find_package(Threads)
	target_link_libraries(OpusFeed ${CMAKE_THREAD_LIBS_INIT})
	target_link_libraries(OpusFeed dl)
	target_link_libraries(OpusFeed rt)
endif()
target_compile_definitions(OpusFeed PUBLIC CURL_STATICLIB=1)
target_compile_definitions(OpusFeed PUBLIC PCRE_STATIC=1)
set_target_properties(OpusFeed PROPERTIES LINK_FLAGS "-static")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(OpusFeed PROPERTIES COMPILE_FLAGS "-Wall -g")
else()
	set_target_properties(OpusFeed PROPERTIES COMPILE_FLAGS "-Wall -fexceptions -s -O2")
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")
set_target_properties(OpusFeed PROPERTIES OUTPUT_NAME "opusfeed")
install(TARGETS OpusFeed
		RUNTIME DESTINATION bin)
