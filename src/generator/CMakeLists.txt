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
    RotaConfig.cpp
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
    RotaConfig.hpp
    GlobalConfig.hpp
    OptimizerData.hpp
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(OptDataGen ${SOURCE} main.cpp)
target_compile_features(OptDataGen PRIVATE cxx_std_17 )
target_link_libraries(OptDataGen boost_json stdc++fs httplib RotaOptimizerLib)
target_include_directories(OptDataGen PRIVATE ../optimizer/)

add_library(${PROJECT_NAME}Lib STATIC ${SOURCE} ${INCLUDES})
target_compile_features(${PROJECT_NAME}Lib PRIVATE cxx_std_17 )
target_link_libraries(${PROJECT_NAME}Lib boost_json stdc++fs httplib RotaOptimizerLib)
target_include_directories(${PROJECT_NAME}Lib PRIVATE ../optimizer/)

install(TARGETS OptDataGen DESTINATION bin)
install(TARGETS ${PROJECT_NAME}Lib DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)