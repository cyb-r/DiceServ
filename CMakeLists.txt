find_path(SIMDE_INCLUDE_DIR simde/x86/sse2.h
    PATHS /usr/include /usr/local/include
    DOC "Path to SIMDe headers"
)

if(NOT SIMDE_INCLUDE_DIR)
    message(FATAL_ERROR "SIMDe not found. Install it via your package manager (e.g. apt install libsimde-dev) or place it in your include path.")
endif()

include_directories(${SIMDE_INCLUDE_DIR})

build_modules(${CMAKE_CURRENT_SOURCE_DIR})

install(FILES diceserv.example.conf
    DESTINATION ${CONF_DIR}
)
