add_executable(basicRota_test basicRota_test.cpp)

target_link_libraries(basicRota_test PRIVATE GTest::GTest)

add_test(
            NAME    BasicRunTest
            COMMAND basicRota_test
        )


set(SOURCE
    ${CMAKE_SOURCE_DIR}/src/generator/RotaMap.cpp
    ${CMAKE_SOURCE_DIR}/src/generator/RotaMode.cpp
    ${CMAKE_SOURCE_DIR}/src/generator/RotaModePool.cpp
    ${CMAKE_SOURCE_DIR}/src/generator/RotaLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/generator/RotaTeam.cpp
    ${CMAKE_SOURCE_DIR}/src/generator/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/generator/dataParsing.cpp
    )

add_executable(utils_test utils_test.cpp ${SOURCE})
target_compile_features(utils_test PRIVATE cxx_std_17 )
target_link_libraries(utils_test PRIVATE GTest::GTest boost_json httplib)

add_test(NAME UtilsTest COMMAND utils_test)