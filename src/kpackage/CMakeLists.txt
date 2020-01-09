
project(kpackage)

add_subdirectory( pics ) 
add_subdirectory( toolbar ) 
add_subdirectory( icon ) 

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)


########### next target ###############

set(kpackage_SRCS 
   kpackage.cpp 
   kpkg.cpp
   managementWidget.cpp 
   packageDepends.cpp
   packageDisplay.cpp 
   packageProperties.cpp 
   findf.cpp 
   search.cpp 
   pkgOptions.cpp 
   packageInfo.cpp 
   main.cpp 
   utils.cpp 
   procbuf.cpp 
   kplview.cpp 
   pkgInterface.cpp 
   debInterface.cpp
   rpmInterface.cpp 
   kpPty.cpp 
   kpTerm.cpp
   kpConfig.cpp
   kpSettings.cpp
)


kde4_add_kcfg_files(kpackage_SRCS kpackageSettings.kcfgc )

kde4_add_ui_files(kpackage_SRCS Command.ui Misc.ui )

kde4_add_executable(kpackage ${kpackage_SRCS})

target_link_libraries(kpackage  ${KDE4_KPTY_LIBS} ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS  kpackage  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES kpackage.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kpackageui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kpackage )
install( FILES kpackageSettings.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )



