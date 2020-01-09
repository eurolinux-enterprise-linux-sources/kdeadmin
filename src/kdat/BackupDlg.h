// KDat - a tar-based DAT archiver
// Copyright (C) 1998-2000  Sean Vyain, svyain@mail.tds.net
// Copyright (C) 2001-2002  Lawrence Widman, kdat@cardiothink.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _BackupDlg_h_
#define _BackupDlg_h_

#include <qdialog.h>
//Added by qt3to4:
#include <QLabel>
#include <QTimerEvent>

class QLabel;
class QPushButton;

class K3Process;

class Archive;
class File;
class LoggerWidget;
class Tape;
class TarParser;

/**
 * @short Status dialog for backing up files.
 */
class BackupDlg : public QDialog {
    Q_OBJECT
    K3Process*       _proc;
    TarParser*      _tarParser;
    QString         _archiveName;
    QString         _workingDir;
    QStringList        _files;
    bool            _oneFilesystem;
    bool            _incremental;
    QString         _snapshot;
    bool            _removeSnapshot;
    int             _archiveSize;
    Tape*           _tape;
    float           _totalKBytes;
    uint            _totalRecords;
    QLabel*         _elapsedTime;
    QLabel*         _timeRemaining;
    QLabel*         _kbytesWritten;
    QLabel*         _transferRate;
    QLabel*         _fileCount;
    LoggerWidget*   _log;
    QPushButton*    _ok;
    QPushButton*    _save;
    QPushButton*    _abort;
    int             _startTime;
    Archive*        _archive;
    bool            _aborted;
    int             _numFiles;

    int     _fileSize;
    int     _fileMTime;
    int     _fileStartRecord;
    QString _fileName;

    void updateStats();
private slots:
    void slotProcessExited( K3Process* proc );
    void slotStdout( K3Process* proc, char* buf, int len );
    void slotOK();
    void slotAbort();
    void slotEntry( const QString& name, int size, int mtime, int record );
protected:
    void show();
    void timerEvent( QTimerEvent* e );
public:
    /**
     * Create a backup dialog.
     *
     * @param archiveName    The name for the new archive.
     * @param workingDir     The directory to backup from.
     * @param files          The list of files to backup.
     * @param oneFilesystem  TRUE means do not follow symbolic links across filesystems.
     * @param incremental    TRUE mean do a GNU listed incremental backup.
     * @param snapshot       The name of the snapshot file for an incremental backup.
     * @param removeSnapshot Remove the snapshot before backing up.
     * @param archiveSize    The estimate size of the archive in kilobytes.
     * @param tape           The tape index to add the archive to.
     * @param parent         The parent widget for this dialog.
     * @param name           The name of this widget.
     */
    BackupDlg( const QString & archiveName, const QString & workingDir, const QStringList& files, bool oneFilesystem, bool incremental,
               const QString & snapshot, bool removeSnapshot, int archiveSize, Tape* tape,
               QWidget* parent = 0, const char* name = 0 );

    /**
     * Destroy the backup dialog.
     */
    ~BackupDlg();
};

#endif
