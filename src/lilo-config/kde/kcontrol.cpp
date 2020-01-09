/* kcontrol.cpp
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

#include "kcontrol.moc"
#include <mainwidget.h>
#include <ui.h>
#include <kglobal.h>
#include <klocale.h>
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>
#include <kaboutdata.h>
#include <unistd.h>
#include <kcomponentdata.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>


K_PLUGIN_FACTORY(LiloFactory, registerPlugin<KControl>();)
K_EXPORT_PLUGIN(LiloFactory("lilo"))


KControl::KControl(QWidget *parent,const QVariantList &)
  : KCModule(LiloFactory::componentData(), parent)
{
	QVBoxLayout *layout=new QVBoxLayout(this);
	m=new MainWidget(this);
	layout->addWidget(m);
	connect(m, SIGNAL(configChanged()), SLOT(configChanged()));
        if (getuid() != 0) {
            m->makeReadOnly();
        }
	KAboutData *about = new KAboutData(I18N_NOOP("kcmlilo"), 0, ki18n("LILO Configuration"),
	0, KLocalizedString(), KAboutData::License_GPL,
	ki18n("(c) 2000, Bernhard Rosenkraenzer"));
	about->addAuthor(ki18n("Bernhard \"Bero\" Rosenkraenzer"), KLocalizedString(), "bero@redhat.com");
	setAboutData(about);
}

void KControl::load()
{
	m->load();
}

void KControl::save()
{
	m->save();
}

void KControl::defaults()
{
	m->defaults();
}

void KControl::reset()
{
	m->reset();
}

void KControl::configChanged() // SLOT
{
	emit changed(true);
}



