if (BUILD_SHARED_LIBS)
    set(CMAKE_DEBUG_POSTFIX -d)
    set(CMAKE_RELWITHDEBINFO_POSTFIX -rd)
else ()
    set(CMAKE_DEBUG_POSTFIX -s-d)
    set(CMAKE_RELEASE_POSTFIX -s)
    set(CMAKE_MINSIZEREL_POSTFIX -s)
    set(CMAKE_RELWITHDEBINFO_POSTFIX -s-rd)
endif ()

file(
    GLOB_RECURSE
    LIBRARY_SOURCES
    CONFIGURE_DEPENDS
    *.h
    *.hh
    *.hpp
    *.hxx
    *.h++
    *.c
    *.cc
    *.cpp
    *.cxx
    *.c++
    *.inl
)

list(
    FILTER
    LIBRARY_SOURCES
    EXCLUDE
    REGEX
    ".*\.test\..*"
)

if (LIBRARY_SOURCES)
    add_library(${PROJECT_NAME} ${LIBRARY_SOURCES})

    include(warnings)
    target_enable_warnings(${PROJECT_NAME})

    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
            OUTPUT_NAME ${PROJECT_NAME}
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    target_include_directories(
        ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                               $<INSTALL_INTERFACE:.>
    )

    find_package(
        gsl-lite
        CONFIG
        REQUIRED
    )

    target_compile_definitions(
        ${PROJECT_NAME} PUBLIC gsl_CONFIG_DEFAULTS_VERSION=1
    )

    target_link_libraries(${PROJECT_NAME} gsl-lite::gsl-lite)

    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

    target_compile_options(${PROJECT_NAME} PRIVATE ${PROJECT_WARNINGS})

    if (WIN32 AND BUILD_SHARED_LIBS)
        set_target_properties(
            ${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON
        )
    endif ()
endif ()

if (TARGET ${PROJECT_NAME})
    if (BUILD_SHARED_LIBS)
        install(
            TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )
    endif ()
endif ()
