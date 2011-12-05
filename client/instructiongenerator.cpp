/*
Copyright 2010 Christoph Eckert ce@christeck.de

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
along with MoNav. If not, see <http://www.gnu.org/licenses/>.
*/

#include "instructiongenerator.h"
#include "routinglogic.h"

InstructionGenerator* InstructionGenerator::instance()
{
	static InstructionGenerator instructionGenerator;
	return &instructionGenerator;
}


InstructionGenerator::InstructionGenerator()
{
	m_previousAbstractInstruction.init();
}


InstructionGenerator::~InstructionGenerator()
{

}


void InstructionGenerator::generateInstructions()
{
	// Called by RoutingLogic before it emits routeChanged()
	generateAbstractInstructions();
}


void InstructionGenerator::generateAbstractInstructions()
{

	QVector< IRouter::Edge > pathEdges = RoutingLogic::instance()->edges();
	QVector< IRouter::Node > pathNodes = RoutingLogic::instance()->nodes();
	IRouter* router = MapData::instance()->router();

	if ( router == NULL || pathEdges.empty() || pathNodes.empty() ) {
		return;
	}

	m_abstractInstructions.clear();
	AbstractInstruction abstractInstruction;
	abstractInstruction.init();
	m_totalDistance = 0;
	m_totalSeconds = 0;

	GPSCoordinate gps = pathNodes.first().coordinate.ToGPSCoordinate();
	unsigned int node = 0;

	for ( int edge = 0; edge < pathEdges.size(); edge++ ) {
		node += pathEdges[edge].length;
		GPSCoordinate nextGPS = pathNodes[node].coordinate.ToGPSCoordinate();
		abstractInstruction.distance = gps.ApproximateDistance( nextGPS );
		gps = nextGPS;
		m_totalDistance += abstractInstruction.distance;
		m_totalSeconds += pathEdges[edge].seconds;
		abstractInstruction.branchingPossible = pathEdges[edge].branchingPossible;
		abstractInstruction.typeID = pathEdges[edge].type;
		abstractInstruction.nameID = pathEdges[edge].name;
		if ( edge < pathEdges.size() -1 ){
			abstractInstruction.direction = angle( pathNodes[node - 1].coordinate, pathNodes[node].coordinate, pathNodes[node +  1].coordinate );
		}
		m_abstractInstructions.append( abstractInstruction );
	}
	purgeInstructions( m_abstractInstructions );
	
	for ( int i = 0; i < m_abstractInstructions.size(); i++ ){
		QString type, name;

		bool typeAvailable = router->GetType( &type, m_abstractInstructions[i].typeID );
		assert( typeAvailable );

		bool nameAvailable = router->GetName( &name, m_abstractInstructions[i].nameID );
		assert( nameAvailable );

		qDebug() << type << name << (int)m_abstractInstructions[i].distance;
		
	}
}


void InstructionGenerator::purgeInstructions( QVector< AbstractInstruction > &abstractInstructions)
{
	bool deleteCurrentIndex = false;
	for ( int i = 0; i < abstractInstructions.size()-1; i++ ){
		// Roundabouts
		if (
			abstractInstructions[i].typeID == 13 &&
			abstractInstructions[i+1].typeID == 13
		)
		{
			// qDebug() << "Unifying roundabout";
			abstractInstructions[i+1].distance += abstractInstructions[i].distance;
			abstractInstructions[i+1].direction = abstractInstructions[i].direction;
			if ( abstractInstructions[i].branchingPossible )
			{
				abstractInstructions[i+1].branchingPossible = true;
				abstractInstructions[i+1].exitNumber += 1;
			}
			deleteCurrentIndex = true;
		}
		// General collection
		if (
			abstractInstructions[i].typeID == abstractInstructions[i+1].typeID &&
			abstractInstructions[i].nameID == abstractInstructions[i+1].nameID &&
			abstractInstructions[i].direction == 0
		)
		{
			abstractInstructions[i+1].distance += abstractInstructions[i].distance;
			// abstractInstructions[i+1].direction = abstractInstructions[i].direction;
			deleteCurrentIndex = true;
		}
		if ( deleteCurrentIndex ){
			// qDebug() << "Removing" << i << "of type" << abstractInstructions[i].typeID << "Name ID" << abstractInstructions[i].nameID;
			abstractInstructions.remove(i);
			i--;
			deleteCurrentIndex = false;
		}
	}
}


int InstructionGenerator::angle( UnsignedCoordinate first, UnsignedCoordinate second, UnsignedCoordinate third ) {
	double x1 = ( double ) second.x - first.x; // a = (x1,y1)
	double y1 = ( double ) second.y - first.y;
	double x2 = ( double ) third.x - second.x; // b = (x2, y2 )
	double y2 = ( double ) third.y - second.y;
	int angle = ( atan2( y1, x1 ) - atan2( y2, x2 ) ) * 180 / M_PI + 720;
	angle %= 360;
	static const int forward = 10;
	static const int sharp = 45;
	static const int slightly = 20;
	if ( angle > 180 ) {
		if ( angle > 360 - forward - slightly ) {
			if ( angle > 360 - forward )
				return 0;
			else
				return 1;
		} else {
			if ( angle > 180 + sharp )
				return 2;
			else
				return 3;
		}
	} else {
		if ( angle > forward + slightly ) {
			if ( angle > 180 - sharp )
				return -3;
			else
				return -2;
		} else {
			if ( angle > forward )
				return -1;
			else
				return 0;
		}
	}
}


