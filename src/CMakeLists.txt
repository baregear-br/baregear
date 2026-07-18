find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSE REQUIRED libpulse)

# Generate audio headers from MP3 files for embedding
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/include/success_audio.h ${CMAKE_SOURCE_DIR}/include/failure_audio.h
    COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/generateAudioHeaders.py
    DEPENDS ${CMAKE_SOURCE_DIR}/scripts/generateAudioHeaders.py
            ${CMAKE_SOURCE_DIR}/media/success.mp3
            ${CMAKE_SOURCE_DIR}/media/failure.mp3
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating audio header files from MP3s"
    VERBATIM
)

add_executable(baregear
        main.cpp
        lexer.cpp
        parser.cpp
        preprocessor.cpp
        transpiler.cpp
        ${CMAKE_SOURCE_DIR}/include/success_audio.h
        ${CMAKE_SOURCE_DIR}/include/failure_audio.h
)

target_compile_definitions(baregear PRIVATE
        APP_VERSION="${APP_VERSION}"
)

target_compile_options(baregear PRIVATE
        -std=c++23
        -Wno-c23-extensions
)

llvm_map_components_to_libnames(llvm_libs core native)
target_link_libraries(baregear PRIVATE
        ${llvm_libs}
        Qt5::Core
        Qt5::Network
        ${PULSE_LIBRARIES}
        ZLIB::ZLIB
        optimizedRuntime
)

target_include_directories(baregear PRIVATE
        ${PULSE_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include
)