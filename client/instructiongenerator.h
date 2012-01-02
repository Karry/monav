/*
Copyright 2011 Christoph Eckert ce@christeck.de

This file is part of MoNav.

MoNav is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MoNav is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MoNav. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INSTRUCTIONGENERATOR_H
#define INSTRUCTIONGENERATOR_H

#include <iostream>
#include "interfaces/irouter.h"
#include "mapdata.h"

#include <QObject>
#include <QDebug>
#include <QVector>
#include <QStringList>

class InstructionGenerator : public QObject
{

Q_OBJECT

public:

	static InstructionGenerator* instance();
	~InstructionGenerator();
	void createInstructions( QVector< IRouter::Edge >&, QVector< IRouter::Node >& );
	void setSpeechEnabled( bool enabled );
	bool speechEnabled();
	void instructions( QStringList* labels, QStringList* icons, int instructionAmount = std::numeric_limits< int >::max() );

public slots:

	// destroys the object
	// void cleanup();
	void requestSpeech();

signals:

	void speechRequest( QString );

protected:

	InstructionGenerator();
	int direction( UnsignedCoordinate first, UnsignedCoordinate second, UnsignedCoordinate third );
	// int nextEdgeToAnnounce();
	double announceDistanceFirst();
	double announceDistanceSecond();
	double currentSpeed();
	double announceDistance( double speed, int seconds );
	QString distanceString( double distance );
	int distanceFileindex(double distance);
	void initialize();
	QStringList m_audioFilenames;
	QStringList m_distanceFilenames;
	QStringList m_iconFilenames;
	QStringList m_instructionStrings;
	QString m_language;
	bool m_speechEnabled;

#ifdef CPPUNITLITE
	void createInsideRoundabout();
	void createStraightforwardTurn();
	void createLeaveMotorway();
	void createAnnounceRoundabout();
	void createDontSpeakAgain();
	void createMotorwayLinkBranch();
	// Scheme: group, test, base class
	friend class AudioIndex_InsideRoundaboutTest;
	friend class AudioIndex_StraightforwardTurnTest;
	friend class AudioIndex_LeaveMotorwayTest;
	friend class AudioIndex_MotorwayLinkBranchTest;
	friend class AudioIndex_AnnounceRoundaboutTest;
	friend class SpeechRequired_DontSpeakAgainTest;
#endif // CPPUNITLITE
};

#endif // INSTRUCTIONGENERATOR_H
