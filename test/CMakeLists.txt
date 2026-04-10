################## add test lib #########################
if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
	add_compile_definitions(Debug)
	add_compile_options(-fsanitize=address,undefined -g)
	add_link_options(-fsanitize=address)
endif()

if(CMAKE_BUILD_TYPE MATCHES THREAD)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
	add_compile_definitions(Debug)
	add_compile_options(-fsanitize=thread -g)
	add_link_options(-fsanitize=thread)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug OR 
	CMAKE_BUILD_TYPE MATCHES THREAD)
	include(FetchContent)
	FetchContent_Declare(
			googletest
			URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
	)
	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)

	enable_testing()
	
	add_executable(test_ringbuffer
			test_ringbuffer.cpp
	)
	target_link_libraries(test_ringbuffer
			GTest::gtest_main
			ringbuffer
	)
	target_compile_features(test_ringbuffer PRIVATE cxx_std_23)
	


	include(GoogleTest)
	gtest_discover_tests(test_ringbuffer)
endif()
##########################################################
