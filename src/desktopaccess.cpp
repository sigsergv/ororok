/*
 * desktopaccess.cpp
 *
 *  Created on: Apr 9, 2010
 *      Author: Sergei Stolyarov
 */

#include "desktopaccess.h"

#include <QtDebug>

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#define SHIFT_MOD 1
#endif

bool Ororok::isShiftKeyPressed()
{
#ifdef Q_WS_WIN
	return false;
#endif

#ifdef Q_WS_X11
	Display * d = QX11Info::display();
	XkbStateRec state;

	if (!XkbGetState(d, XkbUseCoreKbd, &state)) {
		return false;
	}

	qDebug() << "sss" << state.mods;

	return static_cast<bool>(state.mods & SHIFT_MOD);
#endif
}
