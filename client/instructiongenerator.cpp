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
	m_previousAbstractInstruction.init();
	// m_highwayID = std::numeric_limits< unsigned >::max();
	// m_roundaboutID = std::numeric_limits< unsigned >::max();
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
void InstructionGenerator::generateInstructions()
{
	generateAbstractInstructions();
	purgeRoundabouts();
	purgeGeneral();

	if ( m_abstractInstructions.size() > 0 ){
		determineSpeech();
		generateSpeech();
	}
}


void InstructionGenerator::determineSpeech(){
	for ( int i = 0; i < m_abstractInstructions.size(); i++ ){
		m_abstractInstructions[i].audiofileIndex = m_abstractInstructions[i].direction;
	}
}


void InstructionGenerator::generateSpeech(){
	// qDebug() << "Speech Distance" << speechDistance();
	if ( m_abstractInstructions[0].distance > speechDistance() ){
		qDebug() << "Speech cancelled due to huge distance";
		return;
	}

	if ( m_abstractInstructions[0].audiofileIndex < 0 || m_abstractInstructions[0].audiofileIndex >= m_audioFilenames.size() ){
		qDebug() << "Speech cancelled due to audio filename out of range.";
		return;
	}

	QString audioFilename = m_audioFilenames[m_abstractInstructions[0].audiofileIndex];
	// qDebug() << audioFilename;
	audioFilename.prepend( ":/audio/en/" );
	audioFilename.append( ".wav" );
	// TODO: Didn't work
	// emit speechRequest( audioFilename );
	Audio::instance()->speechRequest( audioFilename );
	m_previousAbstractInstruction = m_abstractInstructions[0];
	m_previousAbstractInstruction.turnInstructionGenerated = true;
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
}


void InstructionGenerator::purgeRoundabouts()
{
	bool isRoundabout = false;
	for ( int i = 0; i < m_abstractInstructions.size()-1; i++ ){
		isRoundabout = m_abstractInstructions[i].typeID == 13 ? true : false;
		if ( isRoundabout && m_abstractInstructions[i].branchingPossible ){
			m_abstractInstructions[i].exitNumber++;
		}
		if ( isRoundabout && m_abstractInstructions[i].typeID == m_abstractInstructions[i+1].typeID ){
			m_abstractInstructions[i].branchingPossible = m_abstractInstructions[i+1].branchingPossible;
			m_abstractInstructions[i].distance += m_abstractInstructions[i+1].distance;
			if ( m_abstractInstructions[i+1].branchingPossible ){
				m_abstractInstructions[i].exitNumber++;
			}
			m_abstractInstructions.remove( i+1 );
			// TODO: Isn't this line ugly?
			// It's only necessary due to the i--
			m_abstractInstructions[i].exitNumber--;
			i--;
		}
	}
}


void InstructionGenerator::purgeGeneral()
{
	for ( int i = 0; i < m_abstractInstructions.size()-1; i++ ){
		if (
			m_abstractInstructions[i].typeID == m_abstractInstructions[i+1].typeID &&
			m_abstractInstructions[i].nameID == m_abstractInstructions[i+1].nameID &&
			m_abstractInstructions[i].direction == 0
		)
		{
			m_abstractInstructions[i].distance += m_abstractInstructions[i+1].distance;
			m_abstractInstructions[i].direction = m_abstractInstructions[i+1].direction;
			m_abstractInstructions.remove(i);
			i--;
		}
	}
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

/*
void InstructionGenerator::determineTypes()
{
	IRouter* router = MapData::instance()->router();
	if ( router == NULL ) {
		return;
	}

	QString type;
	// TODO: Risky - better ask the router for the amount of types
	for ( unsigned i = 0; i < 14; i++ ){
		router->GetType( &type, i );
		// qDebug() << "ID and type:" << i << type;
		if ( type == "highway" ){
			m_highwayID = i;
		}
		if ( type == "roundabout" ){
			m_roundaboutID = i;
		}
	}
}
*/

#ifdef CPPUNITLITE

#include "CppUnitLite/CppUnitLite.h"

/*
Current Types:
ID and type: 0 "motorway"
ID and type: 1 "motorway_link"
ID and type: 2 "trunk"
ID and type: 3 "trunk_link"
ID and type: 4 "primary"
ID and type: 5 "primary_link"
ID and type: 6 "secondary"
ID and type: 7 "secondary_link"
ID and type: 8 "tertiary"
ID and type: 9 "unclassified"
ID and type: 10 "residential"
ID and type: 11 "service"
ID and type: 12 "living_street"
ID and type: 13 "roundabout"
*/

void InstructionGenerator::createSimpleRoundabout(){

	// Residential, roundabout, roundabout, residential
	m_abstractInstructions.clear();
	AbstractInstruction instructionPopulator;
	instructionPopulator.init();
	
	// residential
	instructionPopulator.typeID = 10;
	instructionPopulator.branchingPossible = true;
	instructionPopulator.direction = 4;
	instructionPopulator.distance = 100;
	m_abstractInstructions.append( instructionPopulator );

	// roundabout no exit
	instructionPopulator.typeID = 13;
	instructionPopulator.branchingPossible = false;
	instructionPopulator.direction = 2;
	instructionPopulator.distance = 10;
	m_abstractInstructions.append( instructionPopulator );

	// roundabout exit
	instructionPopulator.typeID = 13;
	instructionPopulator.branchingPossible = true;
	instructionPopulator.direction = 4;
	instructionPopulator.distance = 10;
	m_abstractInstructions.append( instructionPopulator );

	// residential
	instructionPopulator.typeID = 10;
	instructionPopulator.branchingPossible = false;
	instructionPopulator.direction = 4;
	instructionPopulator.distance = 100;
	m_abstractInstructions.append( instructionPopulator );
}


TEST( simpleRoundabout, listcheck)
{
	// CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions[0].direction, 0);
	InstructionGenerator::instance()->createSimpleRoundabout();
	InstructionGenerator::instance()->generateInstructions();
	CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions.size(), 3);
	CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions[1].exitNumber, 1);
	CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions[1].distance, 20);
}


void InstructionGenerator::createSimpleString(){

	// branchless, branching, branchless, name changed + branching, name change + branchless
	m_abstractInstructions.clear();
	AbstractInstruction instructionPopulator;
	instructionPopulator.init();
	
	// Create first branchless segment
	instructionPopulator.typeID = 0;
	instructionPopulator.branchingPossible = false;
	instructionPopulator.direction = 0;
	instructionPopulator.distance = 500;
	instructionPopulator.nameID = 0;
	m_abstractInstructions.append( instructionPopulator );

	// Add a branch segment
	instructionPopulator.branchingPossible = true;
	m_abstractInstructions.append( instructionPopulator );

	// add a branchless segment
	instructionPopulator.branchingPossible = false;
	m_abstractInstructions.append( instructionPopulator );

	// Change edge's name, add branch
	instructionPopulator.nameID = 1;
	instructionPopulator.branchingPossible = true;
	m_abstractInstructions.append( instructionPopulator );

	// Change edge's name
	instructionPopulator.branchingPossible = false;
	instructionPopulator.nameID = 2;
	m_abstractInstructions.append( instructionPopulator );
}


TEST( simpleString, listcheck)
{
	InstructionGenerator::instance()->createSimpleString();
	InstructionGenerator::instance()->generateInstructions();
	CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions[0].typeID, 0 );
	CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions[0].typeID, InstructionGenerator::instance()->m_abstractInstructions[1].typeID);
	CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions[0].typeID, InstructionGenerator::instance()->m_abstractInstructions[2].typeID);
	CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions.size(), 3 );
}


void InstructionGenerator::createMultiString(){

	m_abstractInstructions.clear();
	AbstractInstruction instructionPopulator;
	instructionPopulator.init();
	
	// Create first branchless segment
	instructionPopulator.typeID = 0;
	instructionPopulator.branchingPossible = false;
	instructionPopulator.direction = 0;
	instructionPopulator.distance = 500;
	instructionPopulator.nameID = 0;
	m_abstractInstructions.append( instructionPopulator );

	// Add a branch segment
	instructionPopulator.branchingPossible = true;
	m_abstractInstructions.append( instructionPopulator );

	// add a branchless segment
	instructionPopulator.branchingPossible = false;
	m_abstractInstructions.append( instructionPopulator );

	// Change edge's name, add branch
	instructionPopulator.nameID = 1;
	instructionPopulator.branchingPossible = true;
	m_abstractInstructions.append( instructionPopulator );

	// Change edge's name
	instructionPopulator.branchingPossible = false;
	instructionPopulator.nameID = 2;
	m_abstractInstructions.append( instructionPopulator );


	// Repeat this for all the other types
	int size = m_abstractInstructions.size();
	// Avoid type 13 == roundabout
	for ( int ID = 1; ID < 13; ID++ ){
		for ( int index = 0; index < size; index++ ){
			instructionPopulator = m_abstractInstructions[index];
			instructionPopulator.typeID = ID;
			m_abstractInstructions.append( instructionPopulator );
		}
	}
}


TEST( multiString, listcheck)
{
	InstructionGenerator::instance()->createMultiString();
	InstructionGenerator::instance()->generateInstructions();

	CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions.size(), 39 );
}


void InstructionGenerator::createSimpleTurns(){

	m_abstractInstructions.clear();
	AbstractInstruction instructionPopulator;
	instructionPopulator.init();
	
	// Create residential turns from sharply left to sharply right
	instructionPopulator.typeID = 10;
	instructionPopulator.branchingPossible = true;
	instructionPopulator.distance = 500;

	for ( int i = 0; i <= 6; i++ ){
		instructionPopulator.direction = i;
		instructionPopulator.nameID = i;
		m_abstractInstructions.append( instructionPopulator );
	}
}


TEST( simpleTurns, speechId)
{
	InstructionGenerator::instance()->createSimpleTurns();
	InstructionGenerator::instance()->generateInstructions();

	for ( int i = 0; i <= 6; i++ ){
		CHECK_EQUAL(InstructionGenerator::instance()->m_abstractInstructions[i].audiofileIndex, i );
	}
}


#endif // CPPUNITLITE


