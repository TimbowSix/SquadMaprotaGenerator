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
SET(INCLUDES
    RotaMap.hpp
    RotaMode.hpp
    RotaModePool.hpp
    RotaLayer.hpp
    RotaTeam.hpp
    utils.hpp
    dataParsing.hpp
    Generator.hpp
    Config.hpp
    RotaConfig.hpp
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(${PROJECT_NAME} ${SOURCE} main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17 )
target_link_libraries(${PROJECT_NAME} boost_json stdc++fs httplib)

add_library(${PROJECT_NAME}Lib STATIC ${SOURCE} ${INCLUDES})
target_compile_features(${PROJECT_NAME}Lib PRIVATE cxx_std_17 )
target_link_libraries(${PROJECT_NAME}Lib boost_json stdc++fs httplib)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(TARGETS ${PROJECT_NAME}Lib DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)