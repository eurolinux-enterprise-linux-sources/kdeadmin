add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)



########### next target ###############

set(kcm_knetworkconfmodule_PART_SRCS 
   kknownhostinfo.cpp 
   knetworkconfmodule.cpp 
   kroutinginfo.cpp 
   kaddressvalidator.cpp 
   kdnsinfo.cpp 
   knetworkinfo.cpp 
   knetworkconfigparser.cpp 
   knetworkinterface.cpp 
   knetworkconf.cpp 
   main.cpp 
   kwirelessinterface.cpp 
   kadddevicecontainer.cpp )

kde4_add_ui_files(kcm_knetworkconfmodule_PART_SRCS 
   kadddevicewifiext.ui
   kdetectdistrodlg.ui
   kreloadnetworkdlg.ui
   kinterfaceupdowndlg.ui
   kselectdistrodlg.ui
   kadddevicedlgextension.ui
   kadddevicedlg.ui
   kaddknownhostdlg.ui
   knetworkconfdlg.ui
   kadddnsserverdlg.ui
   )

kde4_add_plugin(kcm_knetworkconfmodule ${kcm_knetworkconfmodule_PART_SRCS})



target_link_libraries(kcm_knetworkconfmodule  ${KDE4_KDEUI_LIBS} ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kcm_knetworkconfmodule  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kcm_knetworkconfmodule.desktop   DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )

