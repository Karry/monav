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
	bool spoken;
	void init() {
		nameID = std::numeric_limits< unsigned >::max();
		typeID = std::numeric_limits< unsigned >::max();
		distance = std::numeric_limits< double >::max();
		direction = std::numeric_limits< int >::max();
		audiofileIndex = std::numeric_limits< int >::max();
		exitNumber = 0;
		branchingPossible = false;
		spoken = false;
	}
};


	static InstructionGenerator* instance();
	~InstructionGenerator();
	InstructionGenerator();

public slots:

	// destroys the object
	// void cleanup();
	void generate();

signals:

	void speechRequest( QString );

protected:

	void generateInstruction();
	int angle( UnsignedCoordinate first, UnsignedCoordinate second, UnsignedCoordinate third );
	void determineSpeech();
	void speak();
	double speechDistance();
	bool speechRequired();

	// QStringList m_icons;
	// QStringList m_labels;
	QStringList m_audioFilenames;
	// QVector< AbstractInstruction > m_abstractInstructions;
	AbstractInstruction m_previousInstruction;
	AbstractInstruction m_currentInstruction;
};

#endif // INSTRUCTIONGENERATOR_H
