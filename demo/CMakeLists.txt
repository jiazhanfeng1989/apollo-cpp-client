cmake_minimum_required(VERSION 3.11)

set(APOLLO_DEMO_TARGET apollo_client_demo)
func_collect_source_files(SRC_FILES ${PROJECT_SOURCE_DIR}/demo)
add_executable(${APOLLO_DEMO_TARGET} ${SRC_FILES})

func_link_libraries(${APOLLO_DEMO_TARGET}
    ${APOLLO_CLIENT_TARGET}
    boost_url
    boost_program_options
    nlohmann_json::nlohmann_json
)

# func_auto_format_code(${APOLLO_DEMO_TARGET} ${SRC_FILES})
