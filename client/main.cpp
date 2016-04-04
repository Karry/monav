/*
Copyright 2010  Christian Vetter veaac.fdirct@gmail.com

This file is part of MoNav.

MoNav is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MoNav is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MoNav.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QtPlugin>
#include <QThread>

#include <cstdio>
#include <cstdlib>

#include "mainwindow.h"
#include "mapdata.h"
#include "routinglogic.h"
#include "logger.h"

#ifdef Q_WS_MAEMO_5
	#include <QMaemo5InformationBox>
#endif

Q_IMPORT_PLUGIN( MapnikRendererClient )
Q_IMPORT_PLUGIN( ContractionHierarchiesClient )
Q_IMPORT_PLUGIN( GPSGridClient )
Q_IMPORT_PLUGIN( UnicodeTournamentTrieClient )
Q_IMPORT_PLUGIN( OSMRendererClient )
Q_IMPORT_PLUGIN( QtileRendererClient )


void MessageBoxHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	if ( QApplication::instance() != NULL ) {
		const bool isGuiThread = QThread::currentThread() == QApplication::instance()->thread();
		if ( isGuiThread ) {
	#ifdef Q_WS_MAEMO_5
			switch (type) {
			case QtDebugMsg:
				//QMessageBox::information(0, "Debug message", msg, QMessageBox::Ok);
				break;
			case QtWarningMsg:
				QMaemo5InformationBox::information( NULL, msg.toLocal8Bit().constData(), QMaemo5InformationBox::NoTimeout );
				break;
			case QtCriticalMsg:
				QMaemo5InformationBox::information( NULL, msg.toLocal8Bit().constData(), QMaemo5InformationBox::NoTimeout );
				break;
			case QtFatalMsg:
				QMaemo5InformationBox::information( NULL, msg.toLocal8Bit().constData(), QMaemo5InformationBox::NoTimeout );
				exit( -1 );
			}
	#else
			switch (type) {
			case QtDebugMsg:
				break;
			case QtWarningMsg:
				printf( "MoNav warning:        %s\n", msg.toLocal8Bit().constData() );
				break;
			case QtCriticalMsg:
				printf( "MoNav critical error: %s\n", msg.toLocal8Bit().constData() );
				break;
			case QtFatalMsg:
				printf( "MoNav fatal error:    %s\n", msg.toLocal8Bit().constData() );
				exit( -1 );
        break;
			default:
        break;
			}
	#endif
		}
	}

}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	qInstallMessageHandler( MessageBoxHandler );
	a.connect( &a, SIGNAL(aboutToQuit()), MapData::instance(), SLOT(cleanup()) );
	a.connect( &a, SIGNAL(aboutToQuit()), RoutingLogic::instance(), SLOT(cleanup()) );
	a.connect( &a, SIGNAL(aboutToQuit()), Logger::instance(), SLOT(cleanup()) );
	MainWindow w;
	w.show();
	return a.exec();
}
