add_executable(H5Receptor H5Receptor.cpp H5ReceptorPO.cpp H5Receptor.h)
target_link_libraries(H5Receptor LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(H5Receptor PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS H5Receptor DESTINATION bin)

add_executable(H5Ligand H5Ligand.cpp H5LigandPO.cpp H5Ligand.h)
target_link_libraries(H5Ligand LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(H5Ligand PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS H5Ligand DESTINATION bin)


add_executable(H5Docking H5Docking.cpp H5DockingPO.cpp H5Docking.h)
target_link_libraries(H5Docking LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(H5Docking PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS H5Docking DESTINATION bin)

add_executable(H5Merge H5Merge.cpp H5MergePO.cpp H5Merge.h)
target_link_libraries(H5Merge LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(H5Merge PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS H5Merge DESTINATION bin)

add_executable(H5Split H5Split.cpp H5SplitPO.cpp H5Split.h)
target_link_libraries(H5Split LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(H5Split PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS H5Split DESTINATION bin)