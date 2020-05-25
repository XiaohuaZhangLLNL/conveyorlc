
add_executable(calcSite calcSite.cpp calcSitePO.cpp)
target_link_libraries(calcSite LBind ${Boost_LIBRARIES})
set_target_properties(calcSite PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS calcSite DESTINATION bin)

add_executable(testGround testGround.cpp)
target_link_libraries(testGround LBind ${Boost_LIBRARIES})
set_target_properties(testGround PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS testGround DESTINATION bin)

add_executable(testOpenBabel testOpenBabel.cpp obtest.cpp)
target_link_libraries(testOpenBabel LBind ${Boost_LIBRARIES} ${OPENBABEL3_LIBRARIES})
set_target_properties(testOpenBabel PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS testOpenBabel DESTINATION bin)