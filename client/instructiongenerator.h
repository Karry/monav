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
	int exitNumber;
	bool branchingPossible;
	bool followInstructionGenerated;
	bool turnInstructionGenerated;
	void init() {
		nameID = std::numeric_limits< unsigned >::max();
		typeID = std::numeric_limits< unsigned >::max();
		distance = std::numeric_limits< double >::max();
		direction = std::numeric_limits< int >::max();
		exitNumber = std::numeric_limits< int >::max();
		branchingPossible = false;
		followInstructionGenerated = false;
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
	void describe();
	void purgeInstructions();
	void generateSpeech();

	double m_totalDistance;
	double m_totalSeconds;
	QStringList m_icons;
	QStringList m_labels;
	QStringList m_audioSnippets;
	QVector< AbstractInstruction > m_abstractInstructions;
	AbstractInstruction m_previousAbstractInstruction;
};

#endif // INSTRUCTIONGENERATOR_H
