add_subdirectory(json-c)

include_directories(${PROJECT_NAME}
    PUBLIC json-c
)

target_link_directories(${PROJECT_NAME}
    PRIVATE json-c/
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE json-c
)