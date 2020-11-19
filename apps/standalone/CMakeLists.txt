
add_executable(SDL1Receptor SDL1Receptor.cpp SDL1ReceptorPO.cpp SDL1Receptor.h InitEnv.h)
target_link_libraries(SDL1Receptor LBind ${Boost_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(SDL1Receptor PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS SDL1Receptor DESTINATION bin)

add_executable(SDL2LigandNoMin SDL2LigandNoMin.cpp SDL2LigandPO.cpp SDL2LigandNoMin.h InitEnv.h )
target_link_libraries(SDL2LigandNoMin LBind ${Boost_LIBRARIES} ${OPENBABEL3_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(SDL2LigandNoMin PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS SDL2LigandNoMin DESTINATION bin)

add_executable(SDL3Docking SDL3Docking.cpp dock.cpp mpiparser.cpp mainProcedure.cpp InitEnv.h)
target_link_libraries(SDL3Docking LBind ${Boost_LIBRARIES} conduit conduit_relay conduit_blueprint)
set_target_properties(SDL3Docking PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS SDL3Docking DESTINATION bin)

