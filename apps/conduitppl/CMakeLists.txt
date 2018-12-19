
add_executable(CDT1Receptor CDT1Receptor.cpp CDT1ReceptorPO.cpp CDT1Receptor.h InitEnv.h)
target_link_libraries(CDT1Receptor LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(CDT1Receptor PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS CDT1Receptor DESTINATION bin)

add_executable(CDT2Ligand CDT2Ligand.cpp CDT2LigandPO.cpp InitEnv.h CDT2Ligand.h)
target_link_libraries(CDT2Ligand LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(CDT2Ligand PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS CDT2Ligand DESTINATION bin)

add_executable(CDT3Docking CDT3Docking.cpp dock.cpp mpiparser.cpp mainProcedure.cpp InitEnv.h)
target_link_libraries(CDT3Docking LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(CDT3Docking PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS CDT3Docking DESTINATION bin)

add_executable(CDT4mmgbsa CDT4mmgbsa.cpp CDT4mmgbsaPO.cpp InitEnv.h)
target_link_libraries(CDT4mmgbsa LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(CDT4mmgbsa PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS CDT4mmgbsa DESTINATION bin)

