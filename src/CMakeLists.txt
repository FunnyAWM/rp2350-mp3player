set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
add_executable(${ProjectName}
        main.cpp
        hooks.cpp
        ../lib/OLED-UI/OLED.c
        ../lib/OLED-UI/OLED_Driver.c
        ../lib/OLED-UI/OLED_Fonts.c
        ../lib/OLED-UI/OLED_UI.c
        ../lib/OLED-UI/OLED_UI_Driver.c
        ../lib/OLED-UI/OLED_UI_MenuData.c
)

target_include_directories(${ProjectName} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${ProjectName}
        pico_stdlib
        hardware_i2c
        hardware_uart
        FreeRTOS-Kernel-Heap4
)
pico_add_extra_outputs(${ProjectName})