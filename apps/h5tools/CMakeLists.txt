add_executable(H5Receptor H5Receptor.cpp H5ReceptorPO.cpp H5Receptor.h)
target_link_libraries(H5Receptor LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(H5Receptor PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS H5Receptor DESTINATION bin)


add_executable(H5Docking H5Docking.cpp H5DockingPO.cpp H5Docking.h)
target_link_libraries(H5Docking LBind ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(H5Docking PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS H5Docking DESTINATION bin)