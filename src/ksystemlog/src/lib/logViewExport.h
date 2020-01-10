/***************************************************************************
 *   KSystemLog, a system log viewer tool                                  *
 *   Copyright (C) 2007 by Nicolas Ternisien                               *
 *   nicolas.ternisien@gmail.com                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef _LOG_VIEW_EXPORT_H_
#define _LOG_VIEW_EXPORT_H_

#include <QObject>
#include <QPainter>
#include <QRect>

class LogViewWidget;

class LogViewExport : public QObject {
	
	Q_OBJECT
	
	public:
		LogViewExport(QWidget* parent, LogViewWidget* logViewWidget);

		virtual ~LogViewExport();

		void copyToClipboard();

		void fileSave();
		
		void sendMail();
		
		void printSelection();
		
	signals:
		void statusBarChanged(const QString& message);

	private:
		
		void printPageNumber(QPainter& painter, QRect& printView, int movement, int page);

		QWidget* parent;
		
		LogViewWidget* logViewWidget;
};


#endif //_LOG_VIEW_EXPORT_H_
