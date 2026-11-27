find_package(Flatbuffers REQUIRED)
find_program(FLATC_BIN flatc REQUIRED)

file(GLOB FBS_FILES "*.fbs")

set(CPP_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/cpp")
file(MAKE_DIRECTORY "${CPP_GEN_DIR}")

set(CPP_GENERATED_HEADERS "")

foreach(FBS_FILE IN LISTS FBS_FILES)
    get_filename_component(FBS_NAME "${FBS_FILE}" NAME_WE)

    set(CPP_HEADER "${CPP_GEN_DIR}/${FBS_NAME}_generated.h")
    add_custom_command(
        OUTPUT "${CPP_HEADER}"
        COMMAND "${FLATC_BIN}"
                --cpp
                -I "${CMAKE_CURRENT_SOURCE_DIR}"
                -o "${CPP_GEN_DIR}"
                "${FBS_FILE}"
        DEPENDS ${FBS_FILES}
        COMMENT "Generating C++ FlatBuffers header: ${FBS_NAME}.fbs"
        VERBATIM
    )
    list(APPEND CPP_GENERATED_HEADERS "${CPP_HEADER}")
endforeach()

add_custom_target(protocol_gen DEPENDS ${CPP_GENERATED_HEADERS})
add_library(protocol_lib INTERFACE)
target_include_directories(protocol_lib INTERFACE "${CPP_GEN_DIR}")
target_link_libraries(protocol_lib INTERFACE flatbuffers::flatbuffers)
add_dependencies(protocol_lib protocol_gen)