project(lilo-config)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

FIND_PROGRAM(LILO_EXECUTABLE NAMES lilo LILO  PATHS /sbin /usr/sbin /usr/local/sbin /opt/lilo  )

if(LILO_EXECUTABLE)
	ADD_DEFINITIONS(-DUSE_KDE ) # added manually
        include_directories (${CMAKE_SOURCE_DIR}/lilo-config/)
	
	set(libwidgets_SRCS 
		${CMAKE_SOURCE_DIR}/lilo-config/kde-qt-common/mainwidget.cpp 
		${CMAKE_SOURCE_DIR}/lilo-config/kde-qt-common/general.cpp 
		${CMAKE_SOURCE_DIR}/lilo-config/kde-qt-common/images.cpp 
		${CMAKE_SOURCE_DIR}/lilo-config/kde-qt-common/expert.cpp
		${CMAKE_SOURCE_DIR}/lilo-config/kde-qt-common/EditWidget.cpp	
		)
	
	set(libcommon_SRCS
		${CMAKE_SOURCE_DIR}/lilo-config/common/String.cc 
		${CMAKE_SOURCE_DIR}/lilo-config/common/Files.cc 
		${CMAKE_SOURCE_DIR}/lilo-config/common/Disks.cc 
		${CMAKE_SOURCE_DIR}/lilo-config/common/Config.cc 
		${CMAKE_SOURCE_DIR}/lilo-config/common/lilo.cc
		)
	add_subdirectory( kde ) 
endif(LILO_EXECUTABLE)

