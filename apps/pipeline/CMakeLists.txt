
add_executable(PPL1Receptor PPL1Receptor.cpp PPL1ReceptorPO.cpp)
target_link_libraries(PPL1Receptor LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})
set_target_properties(PPL1Receptor PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS PPL1Receptor DESTINATION bin)

add_executable(PPL2VolumeFilter PPL2VolumeFilter.cpp PPL2VolumeFilterPO.cpp)
target_link_libraries(PPL2VolumeFilter LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})
set_target_properties(PPL2VolumeFilter PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS PPL2VolumeFilter DESTINATION bin)

add_executable(PPL2Ligand PPL2Ligand.cpp PPL2LigandPO.cpp)
target_link_libraries(PPL2Ligand LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})
set_target_properties(PPL2Ligand PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS PPL2Ligand DESTINATION bin)

add_executable(PPL3Docking PPL3Docking.cpp dock.cpp mpiparser.cpp mainProcedure.cpp)
target_link_libraries(PPL3Docking LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})
set_target_properties(PPL3Docking PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS PPL3Docking DESTINATION bin)

add_executable(PPL4mmgbsa PPL4mmgbsa.cpp PPL4mmgbsaPO.cpp)
target_link_libraries(PPL4mmgbsa LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})
set_target_properties(PPL4mmgbsa PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS PPL4mmgbsa DESTINATION bin)

add_executable(PPL4parseXML PPL4parseXML.cpp)
target_link_libraries(PPL4parseXML LBind ${Boost_LIBRARIES})
set_target_properties(PPL4parseXML PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS PPL4parseXML DESTINATION bin)

