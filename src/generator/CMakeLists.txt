set(SOURCE
    main.cpp
    RotaMap.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} boost_json stdc++fs)