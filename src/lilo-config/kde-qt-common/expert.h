/* expert.h
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

/*
** Bug reports and questions can be sent to <kde-devel@kde.org>
*/

#ifndef _EXPERT_H_
#define _EXPERT_H_ 1
#include <lilo.h>
#include <QWidget>
#include <QTextEdit>
#include <QLayout>
//Added by qt3to4:
#include <QHBoxLayout>

class Expert:public QWidget
{
	Q_OBJECT
public:
	explicit Expert(liloconf *l=0, QWidget *parent=0);
	~Expert();
	void setCfg(liloconf *l) { lilo=l; }
    void makeReadOnly();
public slots:
	void update();
	void saveChanges();
signals:
	void configChanged();
private:
	liloconf	*lilo;
	QHBoxLayout	*layout;
	QTextEdit	*edit;
};
#endif
