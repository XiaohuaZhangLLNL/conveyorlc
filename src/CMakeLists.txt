file(GLOB_RECURSE sources  */*.cpp */*.h */*.hpp)
add_library(LBind ${sources})
target_link_libraries (LBind ${SQLITE3_LIBRARIES} ${ZLIB_LIBRARIES} conduit conduit_relay conduit_blueprint)
install(TARGETS LBind DESTINATION lib)

file(GLOB_RECURSE header */*.h)
install(FILES ${header} DESTINATION include)
