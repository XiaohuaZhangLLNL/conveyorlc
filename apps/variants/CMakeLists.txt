
add_executable(SAF4mmgbsa SAF4mmgbsa.cpp SAF4mmgbsaPO.cpp ../conduitppl/InitEnv.h)
target_link_libraries(SAF4mmgbsa LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} ${OPENBABEL3_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(SAF4mmgbsa PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS SAF4mmgbsa DESTINATION bin)

add_executable(SAF4mmgbsaTest SAF4mmgbsaTest.cpp SAF4mmgbsaPO.cpp ../conduitppl/InitEnv.h)
target_link_libraries(SAF4mmgbsaTest LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} ${OPENBABEL3_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(SAF4mmgbsaTest PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS SAF4mmgbsaTest DESTINATION bin)
