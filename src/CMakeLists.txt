add_executable(${PROJECT_NAME}
    main.cpp
    ./hw/rfid/mfrc522.cpp
    ./hw/spi/SPI.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ./
    ./hw
    ./hw/rfid
    ./hw/spi
)

target_compile_options(${PROJECT_NAME} PUBLIC
    -Wall
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    -lwiringPi
)

