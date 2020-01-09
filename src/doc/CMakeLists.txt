MESSAGE(STATUS "Readd kdat/ksysv doc when/if kdat/ksysv will port")
#add_subdirectory(kdat)
#add_subdirectory(ksysv)

FIND_PROGRAM(LILO_EXECUTABLE NAMES lilo LILO  PATHS /sbin /usr/sbin /usr/local/sbin /opt/lilo  )

if ( UNIX )
  add_subdirectory(kuser)
  add_subdirectory(kcron)
  add_subdirectory(ksystemlog)
endif( UNIX )

if(Q_WS_X11)
   add_subdirectory(kpackage)
   if(LILO_EXECUTABLE)
      add_subdirectory(lilo-config)
   endif(LILO_EXECUTABLE)
   add_subdirectory(kcontrol)
endif(Q_WS_X11)

