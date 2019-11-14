file(
    GLOB
    ENTRY_FILES
    CONFIGURE_DEPENDS
    *.cpp
)

foreach (ENTRY_FILE ${ENTRY_FILES})
    get_filename_component(
        APP_NAME
        ${ENTRY_FILE}
        NAME_WE
    )

    set(TARGET_NAME ${APP_NAME}_exe)

    file(
        GLOB_RECURSE
        APP_SOURCES
        CONFIGURE_DEPENDS
        ${APP_NAME}/*.h
        ${APP_NAME}/*.hh
        ${APP_NAME}/*.hpp
        ${APP_NAME}/*.hxx
        ${APP_NAME}/*.h++
        ${APP_NAME}/*.c
        ${APP_NAME}/*.cc
        ${APP_NAME}/*.cpp
        ${APP_NAME}/*.cxx
        ${APP_NAME}/*.c++
        ${APP_NAME}/*.inl
    )

    set(APP_SOURCES ${ENTRY_FILE} ${APP_SOURCES})

    add_executable(${TARGET_NAME} ${APP_SOURCES})

    target_include_directories(
        ${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(
        ${TARGET_NAME} PRIVATE $<TARGET_NAME_IF_EXISTS:${PROJECT_NAME}>
    )

    include(warnings)
    target_enable_warnings(${TARGET_NAME})

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${APP_NAME})

    install(
        TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endforeach ()

find_package(
    ImGui-SFML
    CONFIG
    REQUIRED
)

find_package(
    sfml
    CONFIG
    REQUIRED
)

find_package(
    toml11
    CONFIG
    REQUIRED
)

find_package(
    whereami
    CONFIG
    REQUIRED
)

find_package(
    fmt
    CONFIG
    REQUIRED
)

find_package(
    gsl-lite
    CONFIG
    REQUIRED
)

target_compile_definitions(
    ${PROJECT_NAME}-sfml_exe PRIVATE gsl_CONFIG_DEFAULTS_VERSION=1
)

target_link_libraries(
    ${PROJECT_NAME}-sfml_exe
    PRIVATE
        ImGui-SFML::ImGui-SFML
        whereami::whereami
        toml11::toml11
        gsl-lite::gsl-lite
        sfml::sfml
        fmt::fmt
        tinyfiledialogs
)
