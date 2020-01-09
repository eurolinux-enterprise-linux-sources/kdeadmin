
include_directories( 
	${CMAKE_SOURCE_DIR}/lilo-config/kde-qt-common/ 
	${CMAKE_SOURCE_DIR}/lilo-config/common/ )


########### next target ###############

set(kcm_lilo_PART_SRCS kcontrol.cpp Details.cpp InputBox.cpp ${libcommon_SRCS} ${libwidgets_SRCS})

kde4_add_plugin(kcm_lilo ${kcm_lilo_PART_SRCS})



target_link_libraries(kcm_lilo  ${KDE4_KIO_LIBS} ${QT_QT3SUPPORT_LIBRARY} )

install(TARGETS kcm_lilo  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES lilo.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


