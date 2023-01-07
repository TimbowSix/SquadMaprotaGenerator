add_executable(optimizer_test optimizer_test.cpp)

target_link_directories(optimizer_test PUBLIC ../../optimizer/)
target_link_libraries(optimizer_test PRIVATE GTest::GTest)

add_test(
            NAME    OptimizerTest
            COMMAND optimizer_test
        )