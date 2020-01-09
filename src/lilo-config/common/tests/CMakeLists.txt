



########### next target ###############

set(String_SRCS String.cc )

kde4_add_executable(String ${String_SRCS})

target_link_libraries(String  ${KDE4_KDECORE_LIBS} )


########### install files ###############





#original Makefile.am contents follow:

#String_LDADD = ../.libs/libcommon.a
#noinst_PROGRAMS = String
#INCLUDES = -I.. $(all_includes)
#
#String_SOURCES = \
#	String.cc
#
#messages:
#
