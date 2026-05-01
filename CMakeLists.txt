cmake_minimum_required(VERSION 3.16)

project(Notepad VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(Notepad
    main.cpp
    main_window.h
    main_window.cpp
    text_transform.h
    sort.h
    notepad_exception.h
    spell_checker.h
    spell_checker_highlighter.h
    find_replace_dialog.ui
    word_frequency_dialog.ui
)

target_link_libraries(Notepad PRIVATE Qt6::Widgets)
