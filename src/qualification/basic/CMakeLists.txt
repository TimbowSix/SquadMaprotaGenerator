add_executable(basicRota_test basicRota_test.cpp)
target_compile_features(basicRota_test PRIVATE cxx_std_17 )
target_link_libraries(basicRota_test PRIVATE GTest::GTest boost_json httplib ${PROJECT_NAME}Lib)
target_include_directories(basicRota_test PRIVATE ../../generator/)

add_executable(resetRota_test resetRota_test.cpp)
target_compile_features(resetRota_test PRIVATE cxx_std_17 )
target_link_libraries(resetRota_test PRIVATE GTest::GTest boost_json httplib ${PROJECT_NAME}Lib)
target_include_directories(resetRota_test PRIVATE ../../generator/)

add_executable(utils_test utils_test.cpp)
target_compile_features(utils_test PRIVATE cxx_std_17 )
target_link_libraries(utils_test PRIVATE GTest::GTest boost_json httplib ${PROJECT_NAME}Lib)
target_include_directories(utils_test PRIVATE ../../generator/)

add_test(NAME UtilsTest COMMAND utils_test)
add_test(NAME ResetRotaTest COMMAND resetRota_test)
add_test(NAME BasicRotaTest COMMAND basicRota_test)
