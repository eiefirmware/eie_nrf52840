zephyr_include_directories(BTN LED)

add_subdirectory(BTN)
add_subdirectory(LED)
add_subdirectory_ifdef(CONFIG_DISPLAY LCD)
