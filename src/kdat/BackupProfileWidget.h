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

#ifndef _BackupProfileWidget_h_
#define _BackupProfileWidget_h_

#include <ktabctl.h>
#include <q3strlist.h>
//Added by qt3to4:
#include <QLabel>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class Q3ListBox;

class BackupProfile;

/**
 * @short Display/edit the parameters for a backup operation.
 */
class BackupProfileWidget : public KTabCtl {
    Q_OBJECT
    QLineEdit* _archiveName;
    QComboBox* _workingDir;
    Q3ListBox*  _files;
    QCheckBox* _oneFilesystem;
    QCheckBox* _incremental;
    QLabel*    _snapshotLabel;
    QLineEdit* _snapshotFile;
    QCheckBox* _removeSnapshot;
    QStringList   _absoluteFiles;
    QStringList   _relativeFiles;
private slots:
    void slotTextChanged( const QString & text );
    void slotIncrementalToggled( bool set );
    void slotToggled( bool set );
    void slotWorkingDirActivated( const QString & text );
public:
    /**
     * Create a backup profile widget.
     *
     * @param archiveName The default name for the new archive.
     * @param files       The list of files to be archived.
     * @param parent      The parent widget of this dialog.
     * @param name        The widget name of this dialog.
     */
    BackupProfileWidget( QWidget* parent=0, const char* name=0 );

    /**
     * Destroy the backup profile widget.
     */
    ~BackupProfileWidget();

    /**
     * Set the parameters for the backup profile.
     *
     * @param backupProfile The backup profile to display/edit.
     */
    void setBackupProfile( BackupProfile* backupProfile );

    /**
     * Set the list of files for the backup profile, with full paths.
     *
     * @param files The list.
     */
    void setAbsoluteFiles( const QStringList& files );

    /**
     * Query the name of the archive.
     *
     * @return The name of the new archive.
     */
    QString getArchiveName();

    /**
     * Query the working directory for the tar command.
     *
     * @return The working directory.
     */
    QString  getWorkingDirectory();

    /**
     * Query the list of files to backup, relative to the working directory.
     *
     * @return The file list.
     */
    const QStringList& getRelativeFiles();

    /**
     * Query the list of files to backup, with full paths.
     *
     * @return The file list.
     */
    const QStringList& getAbsoluteFiles();

    /**
     * Query whether or not to cross filesystem boundaries when performing the
     * backup.
     *
     * @return TRUE if the backup is restricted to a single filesystem, FALSE
     *         if the backup can cross filesystem boundaries.
     */
    bool isOneFilesystem();

    /**
     * Query whether this is to be a GNU incremental backup.
     *
     * @return TRUE if incremental, otherwise FALSE.
     */
    bool isIncremental();

    /**
     * Query the name of the snapshot file to use for an incremental backup.
     *
     * @return The name of the snapshot file.
     */
    QString getSnapshotFile();

    /**
     * Query whether to remove the snapshot file before beginning an
     * incremental backup.  This has the effect of performing a full backup.
     *
     * @return TRUE if the snapshot file should be removed, otherwise FALSE.
     */
    bool getRemoveSnapshot();
signals:
    /**
     * Emitted whenever the user changes anything.
     */
    void sigSomethingChanged();
};

#endif
