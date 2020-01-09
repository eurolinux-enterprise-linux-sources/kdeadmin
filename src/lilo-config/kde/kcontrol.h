/* kcontrol.h
**
** Copyright (C) 2000,2001 by Bernhard Rosenkraenzer
**
** Contributions by M. Laurent and W. Bastian.
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

#ifndef _KCONTROL_H_
#define _KCONTROL_H_

/*
** Bug reports and questions can be sent to <kde-devel@kde.org>
*/
#include <QWidget>
#include <qevent.h>
#include <kcmodule.h>
#include "mainwidget.h"


class KControl: public KCModule {
	Q_OBJECT
public:
	explicit KControl(QWidget * parent = 0, const QVariantList &list = QVariantList() );
	void load();
	void save();
	void defaults();
	void reset();
public slots:
	void configChanged();
private:
	MainWidget	*m;
};

#endif

