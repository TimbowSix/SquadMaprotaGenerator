set(SOURCE
    RotaServer.cpp
)

set(INCLUDES
    RotaServer.hpp
)

add_executable(SquadMaprotaServer ${SOURCE})
target_compile_features(SquadMaprotaServer PRIVATE cxx_std_20 )
target_link_libraries(SquadMaprotaServer boost_json stdc++fs httplib ${PROJECT_NAME}Lib RotaOptimizerLib)
target_include_directories(SquadMaprotaServer PRIVATE ../generator/ ../optimizer/)

install(TARGETS SquadMaprotaServer DESTINATION bin)
install(FILES ${INCLUDES} DESTINATION include)