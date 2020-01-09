find_package( KDE4 REQUIRED )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules )

macro_optional_find_package(PythonLibrary)
macro_optional_find_package(SIP)

macro_optional_find_package(PyQt4)
IF(NOT PYQT4_FOUND)
    macro_log_feature(PYQT4_FOUND "PyQt4" "PyQt4 was not found.  It is needed by system-config-printer-kde to run. (Use -DINSTALL_SYSTEM_CONFIG_PRINTER=TRUE to install anyway)" "http://www.riverbankcomputing.co.uk/pyqt/" FALSE)
ENDIF(NOT PYQT4_FOUND)

macro_optional_find_package(PyKDE4)
IF(NOT PYKDE4_FOUND)
    macro_log_feature(PYKDE_FOUND "PyKDE4" "PyKDE4 was not found.  It is needed by system-config-printer-kde to run.  (Use -DINSTALL_SYSTEM_CONFIG_PRINTER=TRUE to install anyway)" "http://websvn.kde.org/trunk/KDE/kdebindings/python/pykde4/" FALSE)
ENDIF(NOT PYKDE4_FOUND)

macro_optional_find_package(PyCups)
IF(NOT PYCUPS_FOUND)
    macro_log_feature(PYCUPS_FOUND "PyCups" "PyCups was not found.  It is needed by system-config-printer-kde to run.  (Use -DINSTALL_SYSTEM_CONFIG_PRINTER=TRUE to install anyway)" "http://cyberelk.net/tim/software/pycups/" FALSE)
ENDIF(NOT PYCUPS_FOUND)

macro_optional_find_package(SystemConfigPrinter)
IF(NOT SYSTEMCONFIGPRINTER_FOUND)
    macro_log_feature(SYSTEMCONFIGPRINTER_FOUND "system-config-printer" "system-config-printer was not found.  Some of its modules (cupshelpers.py, config.py, smburi.py and debug.py) are required by system-config-printer-kde." "http://cyberelk.net/tim/software/system-config-printer/" FALSE)
ENDIF(NOT SYSTEMCONFIGPRINTER_FOUND)

IF(PYQT4_FOUND AND PYKDE4_FOUND AND PYCUPS_FOUND AND SYSTEMCONFIGPRINTER_FOUND)
    SET(INSTALL_SYSTEM_CONFIG_PRINTER TRUE)
ENDIF(PYQT4_FOUND AND PYKDE4_FOUND AND PYCUPS_FOUND AND SYSTEMCONFIGPRINTER_FOUND)

IF(INSTALL_SYSTEM_CONFIG_PRINTER)
    install( FILES
        new-printer.ui
        system-config-printer.ui
        system-config-printer-kde.py
        options.py
        optionwidgets.py
        ipp-browse-dialog.ui
        smb-browse-dialog.ui
        DESTINATION ${DATA_INSTALL_DIR}/system-config-printer-kde )
    install(FILES system-config-printer-kde.desktop DESTINATION ${SERVICES_INSTALL_DIR})
ENDIF(INSTALL_SYSTEM_CONFIG_PRINTER)

macro_display_feature_log()
