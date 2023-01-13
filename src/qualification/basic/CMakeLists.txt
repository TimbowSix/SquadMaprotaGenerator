add_executable(basicRota_test basicRota_test.cpp)

target_link_libraries(basicRota_test PRIVATE GTest::GTest)

add_test(
            NAME    BasicRunTest
            COMMAND basicRota_test
        )


add_executable(utils_test utils_test.cpp ${SOURCE})
target_compile_features(utils_test PRIVATE cxx_std_17 )
target_link_libraries(utils_test PRIVATE GTest::GTest boost_json httplib ${PROJECT_NAME}Lib)
target_include_directories(utils_test PRIVATE ../../generator/)

add_test(NAME UtilsTest COMMAND utils_test)