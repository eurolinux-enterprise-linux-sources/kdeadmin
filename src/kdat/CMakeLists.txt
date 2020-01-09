project(kdat)
add_subdirectory( pics ) 

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)


########### next target ###############

set(kdat_SRCS 
   Archive.cpp 
   ArchiveInfoWidget.cpp 
   BackupDlg.cpp 
   BackupOptDlg.cpp 
   BackupProfile.cpp 
   BackupProfileInfoWidget.cpp 
   BackupProfileManager.cpp 
   BackupProfileWidget.cpp 
   ErrorHandler.cpp 
   File.cpp 
   FileInfoWidget.cpp 
   ImageCache.cpp 
   IndexDlg.cpp 
   InfoShellWidget.cpp 
   KDatMainWindow.cpp 
   LoggerWidget.cpp 
   Node.cpp 
   Options.cpp 
   OptionsDlg.cpp 
   Range.cpp 
   Tape.cpp 
   TapeDrive.cpp 
   TapeFileInfoWidget.cpp 
   TapeInfoWidget.cpp 
   TapeManager.cpp 
   FormatOptDlg.cpp 
   TarParser.cpp 
   Util.cpp 
   VerifyDlg.cpp 
   VerifyOptDlg.cpp 
   ktreeview.cpp 
   main.cpp )

kde4_add_ui3_files(kdat_SRCS OptionsDlgWidget.ui )

kde4_add_executable(kdat ${kdat_SRCS})

target_link_libraries(kdat  ${KDE4_KIO_LIBS} )

install(TARGETS  kdat  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES kdat.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )




#original Makefile.am contents follow:

## KDE tags expanded automatically by am_edit - $Revision: 306930 $ 
#
## this 10 paths are KDE specific. Use them:
## kde_htmldir       Where your docs should go to. (contains lang subdirs)
## kde_appsdir       Where your application file (.desktop) should go to. 
## kde_icondir       Where your icon should go to.
## kde_minidir       Where your mini icon should go to.
## kde_datadir       Where you install application data. (Use a subdir)
## kde_locale        Where translation files should go to.(contains lang subdirs)
## kde_cgidir        Where cgi-bin executables should go to.
## kde_confdir       Where config files should go to.
## kde_mimedir       Where mimetypes should go to.
## kde_toolbardir    Where general toolbar icons should go to.
## kde_wallpaperdir  Where general wallpapers should go to.
#
## just set the variable
#xdg_apps_DATA = kdat.desktop
#
## set the include path for X, qt and KDE
#INCLUDES= $(all_includes)
## claim, which subdirectories you want to install
#SUBDIRS = pics
#
######## This part is very kdat specific
## you can add here more. This one gets installed 
#bin_PROGRAMS = 	kdat
#
## Which sources should be compiled for kdat.
#kdat_SOURCES	=			\
#	Archive.cpp			\
#	ArchiveInfoWidget.cpp		\
#	BackupDlg.cpp			\
#	BackupOptDlg.cpp		\
#	BackupProfile.cpp		\
#	BackupProfileInfoWidget.cpp	\
#	BackupProfileManager.cpp	\
#	BackupProfileWidget.cpp		\
#	ErrorHandler.cpp		\
#	File.cpp			\
#	FileInfoWidget.cpp		\
#	ImageCache.cpp			\
#	IndexDlg.cpp			\
#	InfoShellWidget.cpp		\
#	KDat.cpp			\
#	LoggerWidget.cpp		\
#	Node.cpp			\
#	Options.cpp			\
#	OptionsDlg.cpp			\
#	OptionsDlgWidget.ui       \
#	Range.cpp			\
#	Tape.cpp			\
#	TapeDrive.cpp			\
#	TapeFileInfoWidget.cpp		\
#	TapeInfoWidget.cpp		\
#	TapeManager.cpp			\
#	FormatOptDlg.cpp		\
#	TarParser.cpp			\
#	Util.cpp			\
#	VerifyDlg.cpp			\
#	VerifyOptDlg.cpp		\
#	ktreeview.cpp			\
#	main.cpp
#
#
## the library search path
#kdat_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
## the libraries to link against. Be aware of the order. First the libraries,
## that depend on the following ones.
#kdat_LDADD   = $(LIB_KFILE)
#
## this option you can leave out. Just, if you use "make dist", you need it
#noinst_HEADERS =			\
#	Archive.h			\
#	ArchiveInfoWidget.h		\
#	BackupDlg.h			\
#	BackupOptDlg.h			\
#	BackupProfile.h			\
#	BackupProfileInfoWidget.h	\
#	BackupProfileManager.h		\
#	BackupProfileWidget.h		\
#	File.h				\
#	FileInfoWidget.h		\
#	ImageCache.h			\
#	IndexDlg.h			\
#	InfoShellWidget.h		\
#	KDat.h				\
#	LoggerWidget.h			\
#	Node.h				\
#	Options.h			\
#	OptionsDlg.h			\
#	Range.h				\
#	Tape.h				\
#	TapeDrive.h			\
#	TapeFileInfoWidget.h		\
#	TapeInfoWidget.h		\
#	TapeManager.h			\
#	FormatOptDlg.h			\
#	TarParser.h			\
#	Util.h				\
#	VerifyDlg.h			\
#	VerifyOptDlg.h			\
#	kdat.h				\
#	ktreeview.h
#
## just to make sure, automake makes them 
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kdat.pot
