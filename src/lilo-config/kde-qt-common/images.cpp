/* images.cpp
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

#include "images.moc"
#include <ui.h>

#include <QRegExp>

//Added by qt3to4:
#include <QHBoxLayout>
#include <kvbox.h>

#ifdef USE_KDE
#include "kde/InputBox.h"
#include "kde/Details.h"
#else
#include "qt/InputBox.h"
#include "qt/Details.h"
#endif

Images::Images(liloconf *l, QWidget *parent):QWidget(parent)
{
	current=""; previous=""; // Using QString() gives problems!
	lilo=l;
	layout=new QHBoxLayout(this);
	layout->setMargin(SPACE_MARGIN);
	layout->setSpacing(SPACE_INSIDE);
	images=new Q3ListBox(this);
	layout->addWidget(images, 1);
	connect(images, SIGNAL(highlighted(const QString &)), SLOT(imageSelected(const QString &)));
	images->setWhatsThis( _("This is the list of kernels and operating systems you can currently boot. Select which one you want to edit here."));

	parameters=new KVBox(this);
	//parameters->setMargin(SPACE_MARGIN);
	parameters->setSpacing(SPACE_INSIDE);
	layout->addWidget(parameters, 2);
	image=new EditWidget(_("&Kernel:"), "", true, parameters);
	image->setWhatsThis( _("Enter the filename of the kernel you want to boot here."));
	connect(image, SIGNAL(textChanged(const QString &)), this, SIGNAL(configChanged()));
	label=new EditWidget(_("&Label:"), "", false, parameters);
	label->setWhatsThis( _("Enter the label (name) of the kernel you want to boot here."));
	connect(label, SIGNAL(textChanged(const QString &)), this, SIGNAL(configChanged()));
	root=new EditWidget(_("&Root filesystem:"), "", true, parameters);
	root->setWhatsThis( _("Enter the root filesystem (i.e. the partition that will be mounted as / at boot time) for the kernel you want to boot here."));
	connect(root, SIGNAL(textChanged(const QString &)), this, SIGNAL(configChanged()));
	initrd=new EditWidget(_("&Initial ramdisk:"), "", true, parameters);
	initrd->setWhatsThis( _("If you want to use an initial ramdisk (initrd) for this kernel, enter its filename here. Leave this field blank if you don't intend to use an initial ramdisk for this kernel."));
	connect(initrd, SIGNAL(textChanged(const QString &)), this, SIGNAL(configChanged()));
	append=new EditWidget(_("E&xtra parameters:"), "", false, parameters);
	append->setWhatsThis( _("Enter any extra parameters you wish to pass to the kernel here. Usually, this can be left blank.<br>This sets the <i>append</i> option in lilo.conf."));
	connect(append, SIGNAL(textChanged(const QString &)), this, SIGNAL(configChanged()));

	actions=new KVBox(this);
	//actions->setMargin(SPACE_MARGIN);
	actions->setSpacing(SPACE_INSIDE);
	layout->addWidget(actions);
	dflt=new QPushButton(_("Set &Default"), actions);
	dflt->setWhatsThis( _("Boot this kernel/OS if the user doesn't make a different choice"));
	connect(dflt, SIGNAL(clicked()), SLOT(dfltClicked()));
	details=new QPushButton(_("De&tails"), actions);
	details->setWhatsThis( _("This button brings up a dialog box with further, less commonly used, options."));
	connect(details, SIGNAL(clicked()), SLOT(detailsClicked()));
	probe=new QPushButton(_("&Probe"), actions);
	connect(probe, SIGNAL(clicked()), SLOT(probeClicked()));
	probe->setWhatsThis( _("Automatically generate a (hopefully) reasonable lilo.conf for your system"));
	check=new QPushButton(_("&Check Configuration"), actions);
	connect(check, SIGNAL(clicked()), SLOT(checkClicked()));
	check->setWhatsThis( _("Run LILO in test mode to see if the configuration is ok"));
	addKrnl=new QPushButton(_("Add &Kernel..."), actions);
	connect(addKrnl, SIGNAL(clicked()), SLOT(addKrnlClicked()));
	addKrnl->setWhatsThis( _("Add a new Linux kernel to the boot menu"));
	addOS=new QPushButton(_("Add Other &OS..."), actions);
	connect(addOS, SIGNAL(clicked()), SLOT(addOSClicked()));
	addOS->setWhatsThis( _("Add a non-Linux OS to the boot menu"));
	remove=new QPushButton(_("&Remove Entry"), actions);
	connect(remove, SIGNAL(clicked()), SLOT(removeClicked()));
	remove->setWhatsThis( _("Remove entry from the boot menu"));
	update();
}
Images::~Images()
{
	delete probe;
}
void Images::update() // SLOT
{
	int selection=images->currentItem();
	if(selection==-1) selection=0;
	String dflt=lilo->dflt();
	String entry;
	StringList imgs=lilo->entries();
	images->clear();
	for(StringList::const_iterator it=imgs.begin(); it!=imgs.end(); it++) {
		if(*it==dflt)
			entry=*it + " (default)";
		else
			entry=*it;
		images->insertItem(entry.cstr());
	}
	if((unsigned int)selection>images->count()) selection=images->count();
	images->setSelected(selection, true);
	imageSelected(images->text(selection));
}
void Images::probeClicked() // SLOT
{
	lilo->probe();
	update();
	emit configChanged();
}
void Images::dfltClicked() // SLOT
{
	if (images->currentItem() < 0) return;
	lilo->setDefault(images->currentText().replace(QRegExp(" (default)", true, true),
                    "").toLatin1().data());
	update();
	emit configChanged();
}
void Images::detailsClicked() // SLOT
{
	liloimage *l=lilo->images[current.toLatin1().data()];
	Details *d = new Details(l, this);
	if(d->exec()==QDialog::Accepted) {
		String tmp;
		tmp=l->grep("^[ \t]*read-only[ \t]*");
		if(d->isReadOnly() && tmp.empty())
			l->insert(l->end(), "\tread-only");
		else if(!d->isReadOnly() && !tmp.empty())
			l->remove(tmp);
		l->set("vga", d->vgaMode().toLatin1().data(), true, true, "\t");
		tmp=l->grep("^[ \t]*unsafe[ \t]*");
		if(d->isUnsafe() && tmp.empty())
			l->insert(l->end(), "\tunsafe");
		else if(!d->isUnsafe() && !tmp.empty())
			l->remove(tmp);
		tmp=l->grep("^[ \t]*lock[ \t]*");
		if(d->isLocked() && tmp.empty())
			l->insert(l->end(), "\tlock");
		else if(!d->isLocked() && !tmp.empty())
			l->remove(tmp);
		tmp=l->grep("^[ \t]*restricted[ \t]*");
		if(d->isRestricted() && tmp.empty())
			l->insert(l->end(), "\trestricted");
		else if(!d->isRestricted() && !tmp.empty())
			l->remove(tmp);
		if(d->isRestricted() || d->usePassword())
			l->set("password", d->Password().toLatin1().data(), true, true, "\t");

		l->set("password", d->Password().toLatin1().data(), true, true, "\t");
		emit configChanged();
	}
	delete d;
}
void Images::checkClicked() // SLOT
{
	QString LiloOut=lilo->liloOut().cstr();
	if(lilo->isOk()) {
		LiloOut=_("Configuration ok. LILO said:\n")+LiloOut;
		InformationOK(this, LiloOut, _("Configuration OK"), "lilo-config.confOK");
	} else {
		LiloOut=_("Configuration NOT ok. LILO said:\n")+LiloOut;
		ErrorOK(this, _("Configuration NOT ok"), LiloOut);
	}
}
void Images::addKrnlClicked() // SLOT
{
	InputBox::entries e;
	InputBox::entry l0={ _("&Kernel filename:"), "", true, _("Enter the filename of the kernel you want to boot here.") };
	InputBox::entry l1={ _("&Label:"), "", false, _("Enter the label (name) of the kernel you want to boot here.") };
	InputBox::entry l2={ _("&Root filesystem:"), "", true, _("Enter the root filesystem (i.e. the partition that will be mounted as / at boot time) for the kernel you want to boot here.") };
	InputBox::entry l3={ _("&Initial ramdisk:"), "", true, _("If you want to use an initial ramdisk (initrd) for this kernel, enter its filename here. Leave this field blank if you don't intend to use an initial ramdisk for this kernel.") };
	e.insert(e.end(), l0);
	e.insert(e.end(), l1);
	e.insert(e.end(), l2);
	e.insert(e.end(), l3);
	InputBox *label=new InputBox(e, this);
	if(label->exec()==QDialog::Accepted) {
		QStringList s=label->text();
		QStringList::Iterator it=s.begin();
		String kernel=(*it).toLatin1().data();
		it++;
		String label=(*it).toLatin1().data();
		it++;
		String root=(*it).toLatin1().data();
		it++;
		String initrd=(*it).toLatin1().data();
		lilo->addLinux(label, kernel, root, initrd);
		update();
		emit configChanged();
	}
	delete label;
}
void Images::addOSClicked() // SLOT
{
	InputBox::entries e;
	InputBox::entry l0={_("Boot from dis&k:"), "", true, _("Enter the partition containing the operating system you'd like to boot here.") };
	InputBox::entry l1={_("&Label:"), "", false, _("Enter the label (name) of the operating system here.") };
	e.insert(e.end(), l0);
	e.insert(e.end(), l1);
	InputBox *label=new InputBox(e, this);
	if(label->exec()==QDialog::Accepted) {
		QStringList s=label->text();
		QStringList::Iterator it=s.begin();
		String disk=(*it).toLatin1().data();
		it++;
		String label=(*it).toLatin1().data();
		lilo->addOther(label, disk);
		update();
		emit configChanged();
	}
	delete label;
}
void Images::removeClicked() // SLOT
{
	if(images->currentItem()==-1)
		return;
	QString s=images->currentText();
	if(s.right(10)==" (default)")
		s=s.left(s.length()-10);
        if (s.isNull())
		s = "";
	lilo->images.remove(s.toLatin1().data());
	previous=""; current="";
	update();
	emit configChanged();
}

QString value(QString const &s)
{
	QString r=s.mid(s.find('=')+1).simplifyWhiteSpace();
	if(r.left(1)=="\"")
		r=r.mid(1);
	if(r.right(1)=="\"")
		r=r.left(r.length()-1);
	if (r.isNull())
		r = "";
	return r;
}

void Images::imageSelected(const QString &i) // SLOT
{
	bool blocked = signalsBlocked();
	blockSignals(true);
	QString s=i;
	if(s.right(10)==" (default)")
		s=s.left(s.length()-10);
	if(previous!=s && !previous.isEmpty()) {
		previous=s;
		saveChanges();
	} else if(previous.isEmpty())
		previous=s;

        if (s.isNull())
		s = "";
	current=s;
	liloimage *l=lilo->images[s.toLatin1().data()];
	if(l) {
		QString img=value(l->grep("^[ \t]*(image|other)[ \t]*=").cstr());
		image->setText(img);
		label->setText(s);
		if(l->isLinux()) {
			image->setLabel(_("&Kernel:"));
			String rt=l->grep("^[ \t]*root[ \t]*=");
			if(!rt.empty())
				root->setText(value(rt.cstr()));
			else
				root->setText("");
			String rd=l->grep("^[ \t]*initrd[ \t]*=");
			if(!rd.empty())
				initrd->setText(value(rd.cstr()));
			else
				initrd->setText("");
			append->setText(l->get("append").cstr());
			root->show();
			initrd->show();
			append->show();
		} else {
			image->setLabel(_("Dis&k:"));
			root->hide();
			initrd->hide();
			append->hide();
		}
	}
	blockSignals(blocked);
}
void Images::saveChanges() // SLOT
{
	if(!current.isEmpty()) {
		liloimage *l=lilo->images[current.toLatin1().data()];
		if(l) {
			l->set("image", image->text().toLatin1().data(), true, true);
			l->set("label", label->text().toLatin1().data(), true, true, "\t");
			l->set("root", root->text().toLatin1().data(), true, true, "\t");
			l->set("initrd", initrd->text().toLatin1().data(), true, true, "\t");
			l->set("append", append->text().toLatin1().data(), true, true, "\t");
		}
	}
}

void Images::makeReadOnly()
{
    images->setEnabled( false );
    image->setEnabled( false );
    label->setEnabled( false );
    root->setEnabled( false );
    initrd->setEnabled( false );
    append->setEnabled( false );
    dflt->setEnabled( false );
    details->setEnabled( false );
    probe->setEnabled( false );
    check->setEnabled( false );
    addKrnl->setEnabled( false );
    addOS->setEnabled( false );
    remove->setEnabled( false );
}
