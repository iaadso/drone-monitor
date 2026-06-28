add_executable(drone-backend
    main.cpp
    AlertEngine.cpp
    DataStore.cpp
)

target_link_libraries(drone-backend ws2_32)