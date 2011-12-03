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

	static InstructionGenerator* instance();
	~InstructionGenerator();
	InstructionGenerator();

public slots:

	// destroys the object
	// void cleanup();
	void generateInstructions();

signals:

protected:

	void generate( QVector< IRouter::Node > pathNodes, QVector< IRouter::Edge > pathEdges );
	void newDescription( IRouter* router, const IRouter::Edge& edge );
	// Ported from descriptiongenerator.h
	int angle( UnsignedCoordinate first, UnsignedCoordinate second, UnsignedCoordinate third );
	// void InstructionGenerator::describe( QStringList* icons, QStringList* labels );
	void describe();

	QStringList m_icons;
	QStringList m_labels;
	QStringList m_audioSnippets;
	
	// Ported from descriptiongenerator.h
	unsigned m_lastNameID;
	unsigned m_lastTypeID;
	QString m_lastName;
	QString m_lastType;
	bool m_branchingPossible;
	double m_distance;
	int m_direction;
	int m_exitNumber;
};

#endif // INSTRUCTIONGENERATOR_H
