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

#include <qobject.h>
#include <q3objectdict.h>
//Added by qt3to4:
#include <QResizeEvent>

#include "InfoShellWidget.h"

#include "InfoShellWidget.moc"

InfoShellWidget::InfoShellWidget( QWidget* parent, const char* name )
        : QWidget( parent, name )
{
}

InfoShellWidget::~InfoShellWidget()
{
}

void InfoShellWidget::resizeEvent( QResizeEvent* )
{
    if ( !children().isEmpty() ) {
       for (int i = 0; i < children().size(); ++i) {
		((QWidget*)children().at(i))->resize(size());
        }
    }
}
