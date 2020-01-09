set(scriptsdir "${DATA_INSTALL_DIR}/knetworkconf/backends")


########### install files ###############
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debug.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/debug.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/file.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/file.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/general.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/general.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/network.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/network.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/parse.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/parse.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/platform.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/platform.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/process.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/process.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/replace.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/replace.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/report.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/report.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/service.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/service.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/service-list.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/service-list.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/util.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/util.pl @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xml.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/xml.pl @ONLY)
#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/system-tools-backends.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/system-tools-backends.pc )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/network-conf.cmake ${CMAKE_CURRENT_BINARY_DIR}/network-conf @ONLY)

macro_additional_clean_files( 
	${CMAKE_CURRENT_BINARY_DIR}/debug.pl
	${CMAKE_CURRENT_BINARY_DIR}/file.pl
	${CMAKE_CURRENT_BINARY_DIR}/general.pl
	${CMAKE_CURRENT_BINARY_DIR}/network.pl
	${CMAKE_CURRENT_BINARY_DIR}/parse.pl
	${CMAKE_CURRENT_BINARY_DIR}/platform.pl
	${CMAKE_CURRENT_BINARY_DIR}/process.pl
	${CMAKE_CURRENT_BINARY_DIR}/replace.pl
	${CMAKE_CURRENT_BINARY_DIR}/report.pl
	${CMAKE_CURRENT_BINARY_DIR}/service.pl
	${CMAKE_CURRENT_BINARY_DIR}/service-list.pl
	${CMAKE_CURRENT_BINARY_DIR}/util.pl
	${CMAKE_CURRENT_BINARY_DIR}/xml.pl
	#${CMAKE_CURRENT_BINARY_DIR}/system-tools-backends.pc
	${CMAKE_CURRENT_BINARY_DIR}/network-conf
)

#install(  FILES ${CMAKE_CURRENT_BINARY_DIR}/system-tools-backends.pc  DESTINATION  ${LIB_INSTALL_DIR}/pkgconfig )
install(PROGRAMS  ${CMAKE_CURRENT_BINARY_DIR}/debug.pl ${CMAKE_CURRENT_BINARY_DIR}/file.pl   ${CMAKE_CURRENT_BINARY_DIR}/general.pl ${CMAKE_CURRENT_BINARY_DIR}/network.pl  ${CMAKE_CURRENT_BINARY_DIR}/parse.pl ${CMAKE_CURRENT_BINARY_DIR}/platform.pl ${CMAKE_CURRENT_BINARY_DIR}/process.pl  ${CMAKE_CURRENT_BINARY_DIR}/replace.pl ${CMAKE_CURRENT_BINARY_DIR}/report.pl ${CMAKE_CURRENT_BINARY_DIR}/service.pl ${CMAKE_CURRENT_BINARY_DIR}/service-list.pl ${CMAKE_CURRENT_BINARY_DIR}/util.pl ${CMAKE_CURRENT_BINARY_DIR}/xml.pl ${CMAKE_CURRENT_BINARY_DIR}/network-conf DESTINATION ${DATA_INSTALL_DIR}/knetworkconf/backends)



