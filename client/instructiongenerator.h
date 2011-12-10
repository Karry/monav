/*
Copyright 2010 Christoph Eckert ce@christeck.de

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

struct AbstractInstruction {
	unsigned nameID;
	unsigned typeID;
	double distance;
	int direction;
	int audiofileIndex;
	int exitNumber;
	bool branchingPossible;
	bool turnInstructionGenerated;
	void init() {
		nameID = std::numeric_limits< unsigned >::max();
		typeID = std::numeric_limits< unsigned >::max();
		distance = std::numeric_limits< double >::max();
		direction = std::numeric_limits< int >::max();
		audiofileIndex = std::numeric_limits< int >::max();
		exitNumber = 0;
		branchingPossible = false;
		turnInstructionGenerated = false;
	}
};


	static InstructionGenerator* instance();
	~InstructionGenerator();
	InstructionGenerator();

public slots:

	// destroys the object
	// void cleanup();
	void generateInstructions();

signals:

	void speechRequest( QString );

protected:

	void generateAbstractInstructions();
	int angle( UnsignedCoordinate first, UnsignedCoordinate second, UnsignedCoordinate third );
	void purgeRoundabouts();
	void purgeGeneral();
	void determineSpeech();
	void generateSpeech();
	double speechDistance();
	void determineTypes();

	double m_totalDistance;
	double m_totalSeconds;
	unsigned m_highwayID;
	unsigned m_roundaboutID;
	QStringList m_icons;
	QStringList m_labels;
	// QStringList m_audioSnippets;
	QStringList m_audioFilenames;
	QVector< AbstractInstruction > m_abstractInstructions;
	AbstractInstruction m_previousAbstractInstruction;

#ifdef CPPUNITLITE
	void createSimpleRoundabout();
	void createSimpleString();
	void createMultiString();
	void createSimpleTurns();
	// Scheme: group, test, base class
	friend class listcheck_simpleRoundaboutTest;
	friend class listcheck_simpleStringTest;
	friend class listcheck_multiStringTest;
	friend class speechId_simpleTurnsTest;
#endif // CPPUNITLITE

};

#endif // INSTRUCTIONGENERATOR_H
