/*
Copyright 2011 Christoph Eckert ce@christeck.de

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


#include "routinglogic.h"
#include "instructiongenerator.h"
#include "audio.h"


InstructionGenerator* InstructionGenerator::instance()
{
	static InstructionGenerator instructionGenerator;
	return &instructionGenerator;
}


InstructionGenerator::InstructionGenerator()
{
	m_previousInstruction.init();
	m_currentInstruction.init();
	m_audioFilenames.append( "instructions-turn-sharply-left" );
	m_audioFilenames.append( "instructions-turn-left" );
	m_audioFilenames.append( "instructions-turn-slightly-left" );
	m_audioFilenames.append( "instructions-head-straightforward" );
	m_audioFilenames.append( "instructions-turn-slightly-right" );
	m_audioFilenames.append( "instructions-turn-right" );
	m_audioFilenames.append( "instructions-turn-sharply-right" );
}


InstructionGenerator::~InstructionGenerator()
{

}


// Called by RoutingLogic immediately before it emits routeChanged()
void InstructionGenerator::generate()
{
	generateInstruction();
	if ( speechRequired() ){
		determineSpeech();
		speak();
		m_currentInstruction.spoken = true;
		m_previousInstruction = m_currentInstruction;
	}
}


void InstructionGenerator::determineSpeech(){
	m_currentInstruction.audiofileIndex = m_currentInstruction.direction;
}


void InstructionGenerator::speak(){
	if ( m_currentInstruction.audiofileIndex < 0 || m_currentInstruction.audiofileIndex >= m_audioFilenames.size() ){
		return;
	}
	QString audioFilename = m_audioFilenames[m_currentInstruction.audiofileIndex];
	audioFilename.prepend( ":/audio/en/" );
	audioFilename.append( ".wav" );
	// Required to instantiate it for signal-slot-connections
	Audio::instance();
	emit speechRequest( audioFilename );
	// m_previousAbstractInstruction = m_abstractInstructions[0];
	// m_previousAbstractInstruction.turnInstructionGenerated = true;
}


void InstructionGenerator::generateInstruction()
{

	QVector< IRouter::Edge > pathEdges = RoutingLogic::instance()->edges();
	QVector< IRouter::Node > pathNodes = RoutingLogic::instance()->nodes();
	IRouter* router = MapData::instance()->router();

	if ( router == NULL || pathEdges.size() < 2 || pathNodes.empty() ) {
		return;
	}


	m_currentInstruction.init();
	m_currentInstruction.typeID = pathEdges[0].type;
	m_currentInstruction.nameID = pathEdges[0].name;
	m_currentInstruction.branchingPossible = pathEdges[0].branchingPossible;
	m_currentInstruction.direction = angle( pathNodes[0].coordinate, pathNodes[pathEdges[0].length].coordinate, pathNodes[pathEdges[0].length + 1].coordinate );
	GPSCoordinate gps = pathNodes.first().coordinate.ToGPSCoordinate();
	GPSCoordinate nextGPS = pathNodes[pathEdges[0].length].coordinate.ToGPSCoordinate();
	m_currentInstruction.distance = gps.ApproximateDistance( nextGPS );



/*
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
*/
}


double InstructionGenerator::speechDistance() {
	const RoutingLogic::GPSInfo& gpsInfo = RoutingLogic::instance()->gpsInfo();
	double currentSpeed = 0;

	// Speed is in meters per second
	currentSpeed = gpsInfo.position.IsValid() ? gpsInfo.verticalSpeed : 28;

	// Some possibly reasonable values:
	//  5s	  35m	 7m/s	 25km/h	residential areas
	// 10s	 140m	14m/s	 50km/h	inner city
	// 15s	 315m	21m/s	 75km/h	primaries
	// 20s	 560m	28m/s	100km/h	trunks
	// 30s	1260m	42m/s	150km/h	highways
	// 40s	2240m	56m/s	200km/h	highways
	// Which results in a factor of about 0.7
	double speechDistance = currentSpeed * currentSpeed * 0.7;
	return speechDistance;
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
				return 3;
			else
				return 4;
		} else {
			if ( angle > 180 + sharp )
				return 5;
			else
				return 6;
		}
	} else {
		if ( angle > forward + slightly ) {
			if ( angle > 180 - sharp )
				return 0;
			else
				return 1;
		} else {
			if ( angle > forward )
				return 2;
			else
				return 3;
		}
	}
}


bool InstructionGenerator::speechRequired()
{
	bool required = true;
	// Structs do not provide an == operator
	if (
				m_currentInstruction.typeID == m_previousInstruction.typeID &&
				m_currentInstruction.nameID == m_previousInstruction.nameID &&
				m_previousInstruction.spoken
			){
		required = false;
	}
	return required;
}

