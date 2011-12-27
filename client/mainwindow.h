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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "utils/coordinates.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:

	explicit MainWindow( QWidget* parent = 0 );
	~MainWindow();

public slots:
	void displayInstructions();

protected slots:

	void informationLoaded();
	void mapLoaded();
	void modulesLoaded();
	void modulesCancelled();
	void tripinfoCancelled();

	void dataLoaded();

	void mouseClicked( ProjectedCoordinate clickPos );

	void addZoom();
	void subtractZoom();
	void setZoom( int zoom );

	void settingsGeneral();
	void settingsRenderer();
	void settingsRouter();
	void settingsGPSLookup();
	void settingsAddressLookup();
	void settingsGPS();

	void bookmarks();
	void addresses();
	void alterRoute( UnsignedCoordinate );
	void remove();
	void gpsLocation();
	void gpsCoordinate();

	void setModeSourceSelection();
	void setModeViaNone();
	void setModeViaInsert();
	void setModeViaAppend();
	void setModeTargetSelection();
	void setModeInstructions();
	void setModeless();

	void showInstructionList();
	// void displayInstructions();

	void displayMapChooser();
	void displayModuleChooser();
	void displayTripinfo();
	void hideControls();

	void about();
	void projectPage();

protected:

	virtual void resizeEvent( QResizeEvent* event );
	void createActions();
	void populateMenus();
	void populateToolbars();
	void connectSlots();
	void resizeIcons();

#ifdef Q_WS_MAEMO_5
	void grabZoomKeys( bool grab );
	void keyPressEvent( QKeyEvent* event );
#endif

	struct PrivateImplementation;
	PrivateImplementation* d;

	Ui::MainWindow* m_ui;
};

#endif // MAINWINDOW_H
