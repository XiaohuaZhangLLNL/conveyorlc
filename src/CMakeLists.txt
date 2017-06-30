file(GLOB_RECURSE sources  */*.cpp */*.h)
add_library(LBind ${sources})
install(TARGETS LBind DESTINATION lib)

file(GLOB_RECURSE header */*.h)
install(FILES ${header} DESTINATION include)
