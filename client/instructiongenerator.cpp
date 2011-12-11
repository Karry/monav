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
	m_nextInstruction.init();
	m_audioFilenames.append( "instructions-turn-sharply-left" );
	m_audioFilenames.append( "instructions-turn-left" );
	m_audioFilenames.append( "instructions-turn-slightly-left" );
	m_audioFilenames.append( "instructions-head-straightforward" );
	m_audioFilenames.append( "instructions-turn-slightly-right" );
	m_audioFilenames.append( "instructions-turn-right" );
	m_audioFilenames.append( "instructions-turn-sharply-right" );
	m_audioFilenames.append( "instructions-roundabout_01" );
	m_audioFilenames.append( "instructions-roundabout_02" );
	m_audioFilenames.append( "instructions-roundabout_03" );
	m_audioFilenames.append( "instructions-roundabout_04" );
	m_audioFilenames.append( "instructions-roundabout_05" );
	m_audioFilenames.append( "instructions-roundabout_06" );
	m_audioFilenames.append( "instructions-roundabout_07" );
	m_audioFilenames.append( "instructions-roundabout_08" );
	m_audioFilenames.append( "instructions-roundabout_09" );
	m_audioFilenames.append( "instructions-leave-motorway" );
	m_audioFilenames.append( "instructions-leave-trunk" );
}


InstructionGenerator::~InstructionGenerator()
{

}


// Called by RoutingLogic immediately before it emits routeChanged()
void InstructionGenerator::generate()
{
	generateInstructions();
	if ( speechRequired() ){
		determineSpeech();
		speak();
		m_currentInstruction.spoken = true;
		m_previousInstruction = m_currentInstruction;
	}
}


void InstructionGenerator::determineSpeech(){
	// qDebug() << "current and next types:" << m_currentInstruction.type << m_nextInstruction.type;
	if ( m_nextInstruction.type == "roundabout" ){
		m_currentInstruction.audiofileIndex = m_nextInstruction.exitNumber + 6;
	}
	else if ( m_currentInstruction.type == "motorway" && m_nextInstruction.type == "motorway_link" ){
		m_currentInstruction.audiofileIndex = 16;
	}
	else if ( m_currentInstruction.type == "trunk" && m_nextInstruction.type == "trunk_link" ){
		m_currentInstruction.audiofileIndex = 17;
	}
	else{
		m_currentInstruction.audiofileIndex = m_currentInstruction.direction;
	}
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
}


void InstructionGenerator::generateInstructions()
{

	QVector< IRouter::Edge > m_pathEdges = RoutingLogic::instance()->edges();
	QVector< IRouter::Node > m_pathNodes = RoutingLogic::instance()->nodes();

	// Do not make this a member variable, as the router can change during runtime
	IRouter* router = MapData::instance()->router();
	if ( router == NULL || m_pathEdges.size() < 2 || m_pathNodes.empty() ) {
		return;
	}

	QString typeString;
	QString nameString;

	m_currentInstruction.init();
	m_nextInstruction.init();

	router->GetType( &typeString, m_pathEdges[0].type );
	m_currentInstruction.type = typeString;
	router->GetName( &nameString, m_pathEdges[0].name );
	m_currentInstruction.name = nameString;

	m_currentInstruction.branchingPossible = m_pathEdges[0].branchingPossible;
	m_currentInstruction.direction = angle( m_pathNodes[0].coordinate, m_pathNodes[m_pathEdges[0].length].coordinate, m_pathNodes[m_pathEdges[0].length + 1].coordinate );
	GPSCoordinate gps = m_pathNodes.first().coordinate.ToGPSCoordinate();
	GPSCoordinate nextGPS = m_pathNodes[m_pathEdges[0].length].coordinate.ToGPSCoordinate();
	m_currentInstruction.distance = gps.ApproximateDistance( nextGPS );

// typeString.clear();
// nameString.clear();

	router->GetType( &typeString, m_pathEdges[1].type );
	m_nextInstruction.type = typeString;
	router->GetName( &nameString, m_pathEdges[1].name );
	m_nextInstruction.name = nameString;

// qDebug() << "current and next types:" << m_currentInstruction.type << m_nextInstruction.type;

	if ( m_nextInstruction.type == "roundabout" ){
		for ( int i = 1; i < m_pathEdges.size(); i++ ){
			router->GetType( &typeString, m_pathEdges[i].type );
			if ( typeString == "roundabout" ){
				if ( m_pathEdges[i].branchingPossible ){
					m_nextInstruction.exitNumber++;
				}
			}
			else{
				break;
			}
		}
	}
}


bool InstructionGenerator::speechRequired()
{
	bool required = true;
	// Required to avoid speech output at startup
	if (
	    m_previousInstruction.type == "" &&
	    m_previousInstruction.name == "" &&
	    m_previousInstruction.direction == std::numeric_limits< int >::max()
	    ){
		required = false;
		m_previousInstruction = m_currentInstruction;
	}
	// Structs do not provide an == operator
	else if (
				m_currentInstruction.type == m_previousInstruction.type &&
				m_currentInstruction.name == m_previousInstruction.name &&
				m_previousInstruction.spoken
			){
		required = false;
	}
	qDebug() << "Speech Required:" << required;
	return required;
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



