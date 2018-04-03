
add_executable(calcSite calcSite.cpp calcSitePO.cpp)
target_link_libraries(calcSite LBind ${Boost_LIBRARIES})
set_target_properties(calcSite PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS calcSite DESTINATION bin)
