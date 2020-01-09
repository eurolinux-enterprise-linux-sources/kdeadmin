project(ksysv)

add_subdirectory( pics ) 
add_subdirectory( toolbar ) 




########### next target ###############

set(ksysv_SRCS 
   main.cpp 
   kauthicon.cpp
   ActionList.cpp 
   #IOCore.cpp 
   ksvdraglist.cpp 
   RunlevelAuthIcon.cpp 
   #OldView.cpp 
   Properties.cpp 
   SpinBox.cpp 
   ServiceDlg.cpp 
   TopWidget.cpp 
   Data.cpp 
   ksv_conf.cpp 
   #kdltooltip.cpp 
   ksv_core.cpp 
   ksvdrag.cpp 
   kscroller.cpp 
   ksv_service.cpp 
   leveldb.c 
   PreferencesDialog.cpp 
   trash.cpp 
   ksvpathconfig.cpp 
   ksvmiscconfig.cpp 
   ksvlookandfeel.cpp 
   ksvconfigwizard.cpp 
   kbusymanager.cpp )

kde4_add_ui3_files(ksysv_SRCS 
   pathconfig.ui 
   miscconfig.ui 
   lookandfeelconfig.ui 
   configwizard.ui )

kde4_add_executable(ksysv ${ksysv_SRCS})

install(TARGETS  ksysv  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES ksysv.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES x-ksysv.desktop  DESTINATION  ${MIME_INSTALL_DIR}/application )
install( FILES x-ksysv-log.desktop  DESTINATION  ${MIME_INSTALL_DIR}/text )
install( FILES ksysvui.rc  DESTINATION  ${DATA_INSTALL_DIR}/ksysv )

kde4_install_icons( ${ICON_INSTALL_DIR}   )
