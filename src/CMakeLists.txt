project(kdeadmin)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

#macro_optional_add_subdirectory(kdat)
macro_optional_add_subdirectory(strigi-analyzer)
if ( UNIX )
  macro_optional_add_subdirectory(kuser)
  macro_optional_add_subdirectory(kcron)
  macro_optional_add_subdirectory(ksystemlog)
  macro_optional_add_subdirectory(system-config-printer-kde)
endif( UNIX )

#macro_optional_add_subdirectory(ksysv)
if(Q_WS_X11)
   macro_optional_add_subdirectory(kpackage)
   macro_optional_add_subdirectory(lilo-config)
   macro_optional_add_subdirectory(knetworkconf)
endif(Q_WS_X11)
MESSAGE(STATUS "readd kdat-ksysv when porting")

macro_optional_add_subdirectory(doc)

macro_display_feature_log()
