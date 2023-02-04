set(SOURCE
    OptimizerMain.cpp
    RotaOptimizer.cpp
    OptimizerConfig.cpp
)

set(INCLUDES
    RotaOptimizer.hpp
    OptimizerConfig.hpp
)

add_executable(RotaOptimizer ${SOURCE})
# add_library(boost RotaOptimizer.cpp)
# target_link_libraries(RotaOptimizer boost)

add_library(RotaOptimizerLib ${SOURCE} ${INCLUDES})

install(TARGETS RotaOptimizerLib DESTINATION lib)
install(FILES ${INCLUDES} DESTINATION include)