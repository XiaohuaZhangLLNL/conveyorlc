
add_executable(PPL1Receptor PPL1Receptor.cpp PPL1ReceptorPO.cpp)
target_link_libraries(PPL1Receptor LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})

add_executable(PPL2Ligand PPL2Ligand.cpp PPL2LigandPO.cpp)
target_link_libraries(PPL2Ligand LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})

add_executable(PPL3Docking PPL3Docking.cpp dock.cpp mpiparser.cpp mainProcedure.cpp)
target_link_libraries(PPL3Docking LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})

add_executable(PPL4mmgbsa PPL4mmgbsa.cpp PPL4mmgbsaPO.cpp)
target_link_libraries(PPL4mmgbsa LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})

add_executable(PPL4parseXML PPL4parseXML.cpp)
target_link_libraries(PPL4parseXML LBind ${Boost_LIBRARIES})

