
add_executable(CDT1Receptor CDT1Receptor.cpp CDT1ReceptorPO.cpp CDT1Receptor.h InitEnv.h)
target_link_libraries(CDT1Receptor PRIVATE LBind ${Boost_LIBRARIES} MPI::MPI_CXX conduit conduit_relay conduit_blueprint)
set_target_properties(CDT1Receptor PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS CDT1Receptor DESTINATION bin)

add_executable(CDT2Ligand CDT2Ligand.cpp CDT2LigandPO.cpp CDT2Ligand.h InitEnv.h )
target_link_libraries(CDT2Ligand PRIVATE LBind ${Boost_LIBRARIES} MPI::MPI_CXX conduit conduit_relay conduit_blueprint)
set_target_properties(CDT2Ligand PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS CDT2Ligand DESTINATION bin)

add_executable(CDT2LigandNoMin CDT2LigandNoMin.cpp CDT2LigandPO.cpp CDT2LigandNoMin.h InitEnv.h )
target_link_libraries(CDT2LigandNoMin PRIVATE LBind ${Boost_LIBRARIES} MPI::MPI_CXX ${OPENBABEL3_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(CDT2LigandNoMin PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS CDT2LigandNoMin DESTINATION bin)

add_executable(CDT3Docking CDT3Docking.cpp dock.cpp mpiparser.cpp mainProcedure.cpp InitEnv.h)
target_link_libraries(CDT3Docking PRIVATE LBind ${Boost_LIBRARIES} Threads::Threads MPI::MPI_CXX conduit conduit_relay conduit_blueprint)
set_target_properties(CDT3Docking PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS CDT3Docking DESTINATION bin)

add_executable(CDT4mmgbsa CDT4mmgbsa.cpp CDT4mmgbsaPO.cpp InitEnv.h)
target_link_libraries(CDT4mmgbsa PRIVATE LBind ${Boost_LIBRARIES} MPI::MPI_CXX conduit conduit_relay conduit_blueprint)
set_target_properties(CDT4mmgbsa PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS CDT4mmgbsa DESTINATION bin)

