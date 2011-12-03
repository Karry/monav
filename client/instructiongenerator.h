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
#include <QObject>
#include <QDebug>

class InstructionGenerator : public QObject
{

Q_OBJECT

public:

	static InstructionGenerator* instance();
	~InstructionGenerator();

public slots:

	// destroys the object
	// void cleanup();

signals:

protected:

};

#endif // INSTRUCTIONGENERATOR_H
