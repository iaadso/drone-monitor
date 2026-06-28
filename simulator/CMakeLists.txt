add_executable(drone-simulator
    main.cpp
    Drone.cpp
)

target_link_libraries(drone-simulator ws2_32)