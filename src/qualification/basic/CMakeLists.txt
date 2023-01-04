add_executable(basicRota_test basicRota_test.cpp)

target_link_libraries(basicRota_test PRIVATE GTest::GTest)

add_test(
            NAME    BasicRunTest
            COMMAND basicRota_test
        )