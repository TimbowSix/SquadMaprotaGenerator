set(SOURCE
    #__test.cpp
    RotaMap.cpp
    RotaMode.cpp
    RotaModePool.cpp
    RotaLayer.cpp
    RotaTeam.cpp
    utils.cpp
    dataParsing.cpp
    Generator.cpp
    Config.cpp
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(${PROJECT_NAME} ${SOURCE} main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17 )
target_link_libraries(${PROJECT_NAME} boost_json stdc++fs httplib)


add_library(${PROJECT_NAME}Lib STATIC ${SOURCE})
target_compile_features(${PROJECT_NAME}Lib PRIVATE cxx_std_17 )
target_link_libraries(${PROJECT_NAME}Lib boost_json stdc++fs httplib)