#include <klocale.h>

#include "qframe.h"

#include "kpConfig.h"
#include "kpackageSettings.h"
#include "ui_Command.h"
#include "ui_Misc.h"

kpConfig::kpConfig( KConfigSkeleton *config, QWidget *parent, const char *name ) :
  KConfigDialog( parent, name, config)
{
  QFrame *page1 = new QFrame( this );
  Ui::Command *ui = new Ui::Command();
  ui->setupUi(page1);
  addPage(page1,i18n("Command"),"",i18n("Privilege Command"));

  QFrame *page2 = new QFrame( this );
  Ui::Misc *mui = new Ui::Misc();
  mui->setupUi(page2);
  addPage(page2,i18n("Misc"),"",i18n("Misc"));
  setHelp(QString(),"kpackage");
}

