set(SOURCE
    RotaServer.cpp
)

add_executable(RotaServer ${SOURCE})
target_compile_features(RotaServer PRIVATE cxx_std_20 )
target_link_libraries(RotaServer boost_json stdc++fs httplib ${PROJECT_NAME}Lib)
target_include_directories(RotaServer PRIVATE ../generator/)