#!/bin/sh
$EXTRACTRC --cstart='#' *.ui >> ./ui.py
$XGETTEXT --language=Python *.py -o $podir/system-config-printer-kde.pot
rm -f ui.py
