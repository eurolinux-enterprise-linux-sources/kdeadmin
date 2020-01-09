/*
   Copyright 2000 Peter Putzer <putzer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <qtimer.h>
#include <qfileinfo.h>
#include <QLayout>

//Added by qt3to4:
#include <Q3Frame>

#include <kdebug.h>
#include <kiconloader.h>
#include <kdialog.h>
#include <klocale.h>

#include "ksv_core.h"
#include "RunlevelAuthIcon.h"
#include <QLabel>

RunlevelAuthIcon::RunlevelAuthIcon (const QString& servicesPath, const QString& runlevelPath,
									QWidget* parent, const char* name)
  : KAuthIcon (parent),
	mTimer (new QTimer (this)),
	mServicesInfo (new QFileInfo (servicesPath)),
    mRLInfo (new QFileInfo* [ksv::runlevelNumber]),
	mOld (false),
	mInterval (1000),
    mCheckEnabled(false)
{
  lockText = i18n("Editing disabled - please check your permissions");
  openLockText = i18n("Editing enabled");

  lockLabel->setText (lockText);
  lockLabel->hide();

  lockPM = UserIcon ("ksysv_locked");
  openLockPM = UserIcon ("ksysv_unlocked");

  lockBox->setPixmap (lockPM);

  lockBox->setMargin (1);
  lockBox->setFrameStyle (Q3Frame::NoFrame);
  lockBox->setFixedSize (lockBox->sizeHint());

  connect (mTimer, SIGNAL (timeout()), this, SLOT (timerEvent()));
  mTimer->start (mInterval);

  for (int i = 0; i < ksv::runlevelNumber; ++i)
	{
	  mRLInfo[i] = new QFileInfo ((runlevelPath + "/rc%1.d").arg(i));
	}

  updateStatus();
  layout->activate();
}

RunlevelAuthIcon::~RunlevelAuthIcon ()
{
  delete mServicesInfo;

  for (int i = 0; i < ksv::runlevelNumber; ++i)
	{
	  delete mRLInfo[i];
	}

  delete[] mRLInfo;
}

void RunlevelAuthIcon::updateStatus ()
{
  if (!mCheckEnabled)
    return;

  const bool res = status();

  if (mOld != res)
	{
	  lockBox->setPixmap (res ? openLockPM : lockPM);
	  lockLabel->setText (res ? openLockText : lockText);

	  this->setToolTip( lockLabel->text());

      mOld = res;
	  emit authChanged (res);
	}
  else
    mOld = res;
}

bool RunlevelAuthIcon::status () const
{
  bool result = mServicesInfo->isWritable();

  for (int i = 0; i < ksv::runlevelNumber; ++i)
    result = result && mRLInfo[i]->isWritable();

  return result;
}

void RunlevelAuthIcon::timerEvent ()
{
  for (int i = 0; i < ksv::runlevelNumber; ++i)
	{
	  mRLInfo[i]->refresh();
	}
  
  mServicesInfo->refresh();

  updateStatus();
}

void RunlevelAuthIcon::setServicesPath (const QString& path)
{
  mTimer->stop();

  mServicesInfo->setFile (path);

  mTimer->start(mInterval);
}

void RunlevelAuthIcon::setRunlevelPath (const QString& path)
{
  mTimer->stop();

  for (int i = 0; i < ksv::runlevelNumber; ++i)
	{
	  mRLInfo[i]->setFile ((path + "/rc%1.d").arg(i));
	}

  mTimer->start(mInterval);
}

void RunlevelAuthIcon::setRefreshInterval (int interval)
{
  mInterval = interval;

  mTimer->stop();
  mTimer->start (mInterval);
}

void RunlevelAuthIcon::setCheckEnabled (bool on)
{
  kDebug(3000) << "enabling authicon " << on;
  mCheckEnabled = on;

  // refresh everything
  mOld = !status();
  timerEvent();
}

#include "RunlevelAuthIcon.moc"
