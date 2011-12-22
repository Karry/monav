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

/*
InstructionGenerator* InstructionGenerator::instance()
{
	static InstructionGenerator instructionGenerator;
	return &instructionGenerator;
}
*/

InstructionGenerator::InstructionGenerator()
{
	m_previousInstruction.init();
	m_currentInstruction.init();
	m_nextInstruction.init();
	m_audioFilenames.append( "instructions-head-straightforward" );
	m_audioFilenames.append( "instructions-turn-slightly-right" );
	m_audioFilenames.append( "instructions-turn-right" );
	m_audioFilenames.append( "instructions-turn-sharply-right" );
	m_audioFilenames.append( "instructions-turn-u" );
	m_audioFilenames.append( "instructions-turn-sharply-left" );
	m_audioFilenames.append( "instructions-turn-left" );
	m_audioFilenames.append( "instructions-turn-slightly-left" );
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
	QLocale DefaultLocale;
	m_language = DefaultLocale.name();
	m_language.truncate( 2 );
}


InstructionGenerator::~InstructionGenerator()
{

}


void InstructionGenerator::determineSpeech(){

	// qDebug() << "Previous, current and next types:" << m_previousInstruction.type << m_currentInstruction.type << m_nextInstruction.type;
	// qDebug() << "Previous, current and next exit amounts:" << m_previousInstruction.exitNumber << m_currentInstruction.exitNumber << m_nextInstruction.exitNumber;
	// qDebug() << "Previous, current and next lengths:" << m_previousInstruction.distance << m_currentInstruction.distance << m_nextInstruction.distance;
	// qDebug() << "Previous, current and next directions:" << m_previousInstruction.direction << m_currentInstruction.direction << m_nextInstruction.direction;

	if ( m_currentInstruction.type == "roundabout" ){
		// qDebug() << "No speech at all in roundabouts";
		m_currentInstruction.audiofileIndex = -1;
	}
	else if ( m_currentInstruction.type != "roundabout" && m_nextInstruction.type == "roundabout" ){
		// qDebug() << "Announcing a roundabout by providing the exit number";
		m_currentInstruction.audiofileIndex = m_nextInstruction.exitNumber + 7;
	}
	else if ( m_currentInstruction.type == "motorway" && m_nextInstruction.type == "motorway_link" ){
		// qDebug() << "Leaving the motorway";
		m_currentInstruction.audiofileIndex = 17;
	}
	else if ( m_currentInstruction.type == "trunk" && m_nextInstruction.type == "trunk_link" ){
		// qDebug() << "Leaving the trunk";
		m_currentInstruction.audiofileIndex = 18;
	}
	else if ( m_currentInstruction.branchingPossible && m_currentInstruction.direction != 0 ){
		// qDebug() << "Announdcing an ordinary turn";
		m_currentInstruction.audiofileIndex = m_currentInstruction.direction;
	}
	else{
		// qDebug() << "No speech at all required";
		m_currentInstruction.audiofileIndex = -1;
	}
	// qDebug() << "Previous, current and next audio index:" << m_previousInstruction.audiofileIndex << m_currentInstruction.audiofileIndex << m_nextInstruction.audiofileIndex;
}


void InstructionGenerator::speak(){
	if ( m_currentInstruction.audiofileIndex < 0 || m_currentInstruction.audiofileIndex >= m_audioFilenames.size() ){
		// qDebug() << "Audio file index out of range:" << m_currentInstruction.audiofileIndex;
		return;
	}

	QString audioFilename = m_audioFilenames[m_currentInstruction.audiofileIndex];
	audioFilename.prepend( "/" );
	audioFilename.prepend( m_language );
	audioFilename.prepend( ":/audio/" );
	audioFilename.append( ".wav" );
	// Required to instantiate it for signal-slot-connections
	Audio::instance();
	// emit speechRequest( audioFilename );
	m_currentInstruction.spoken = true;
	m_previousInstruction = m_currentInstruction;
}


void InstructionGenerator::createInstructions( QVector< IRouter::Edge >& edges, QVector< IRouter::Node >& nodes )
{
	// Do not make this a member variable, as the router can change during runtime
	IRouter* router = MapData::instance()->router();
	if ( router == NULL || edges.size() < 2 || nodes.empty() ) {
		return;
	}

	QString typeString;
	QString nameString;
	int endNode = 0;

	for ( int i = 0; i < edges.size(); i++ ){

		edges[i].audiofileIndex = -1;
		edges[i].exitNumber = -1;

		edges[i].distance = 0;
		for ( int node = endNode; node < endNode + edges[i].length; node++ ){
			 edges[i].distance += nodes[ node ].coordinate.ToGPSCoordinate().ApproximateDistance( nodes[ node +1 ].coordinate.ToGPSCoordinate() );
		}

		endNode += edges[i].length;
		if ( i < edges.size() -2 ){
			edges[i].direction = angle( nodes[ endNode -1 ].coordinate, nodes[ endNode ].coordinate, nodes[ endNode +1 ].coordinate );
		}
		else{
			edges[i].direction = -1;
		}

		edges[i].spoken = false;
		router->GetType( &typeString, edges[i].type );
		edges[i].typeString = typeString;
		router->GetName( &nameString, edges[i].name );
		edges[i].nameString = nameString;
	}
}


/*
bool InstructionGenerator::speechRequired()
{
	bool required = true;
	// Structs do not provide an == operator
	if (
				m_currentInstruction.type == m_previousInstruction.type &&
				m_currentInstruction.name == m_previousInstruction.name &&
				m_currentInstruction.direction == m_previousInstruction.direction &&
				// !m_currentInstruction.branchingPossible &&
				m_previousInstruction.spoken
			){
		required = false;
		// qDebug() << "Edge already announced.";
	}
	else if ( m_currentInstruction.type == "motorway_link" || m_currentInstruction.type == "trunk_link" || m_currentInstruction.type == "primary_link" ){
		if ( m_currentInstruction.branchingPossible ){
			required = true;
		}
	}
	else if ( m_currentInstruction.distance > speechDistance() ){
		required = false;
		// qDebug() << "Speech dist not reached (current, required):" << m_currentInstruction.distance << "" << speechDistance();
	}
	return required;
}
*/


double InstructionGenerator::speechDistance() {
	const RoutingLogic::GPSInfo& gpsInfo = RoutingLogic::instance()->gpsInfo();
	double currentSpeed = 0;

	// Speed is in meters per second
	currentSpeed = gpsInfo.position.IsValid() ? gpsInfo.groundSpeed : 2;

	// Some possibly reasonable values (0.2 seconds per km/h):
	//  5s	  35m	 7m/s	 25km/h	residential areas
	// 10s	 140m	14m/s	 50km/h	inner city
	// 15s	 315m	21m/s	 75km/h	primaries
	// 20s	 560m	28m/s	100km/h	trunks
	// 30s	1260m	42m/s	150km/h	highways
	// 40s	2240m	56m/s	200km/h	highways
	// Which results in a factor of about 0.7
	// Reduced to 0.6 due to reality check
	double speechDistance = currentSpeed * currentSpeed * 0.6;
#ifdef CPPUNITLITE
	if ( speechDistance < 10 ){
		speechDistance = 50;
	}
#endif // CPPUNITLITE
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
				return 5;
			else
				return 6;
		} else {
			if ( angle > forward )
				return 7;
			else
				return 0;
		}
	}
}


#ifdef CPPUNITLITE

#include "CppUnitLite/CppUnitLite.h"

void InstructionGenerator::createInsideRoundabout(){
	m_previousInstruction.init();
	m_currentInstruction.init();
	m_nextInstruction.init();

	m_currentInstruction.branchingPossible = true;
	m_currentInstruction.direction = 7;
	m_currentInstruction.distance = 20;
	m_currentInstruction.type = "roundabout";
	m_currentInstruction.name = "";
	m_currentInstruction.exitNumber = 0;
	m_currentInstruction.spoken = false;
}

TEST( InsideRoundabout, AudioIndex)
{
	instructionGenerator.createInsideRoundabout();
	instructionGenerator.determineSpeech();
	CHECK_EQUAL( instructionGenerator.m_currentInstruction.audiofileIndex, -1 );
}


void InstructionGenerator::createAnnounceRoundabout(){
	m_previousInstruction.init();
	m_currentInstruction.init();
	m_nextInstruction.init();

	m_currentInstruction.branchingPossible = true;
	m_currentInstruction.direction = 2;
	m_currentInstruction.distance = 100;
	m_currentInstruction.type = "unclassified";
	m_currentInstruction.name = "";
	m_currentInstruction.exitNumber = 0;
	m_currentInstruction.spoken = false;

	m_nextInstruction.branchingPossible = true;
	m_nextInstruction.direction = 7;
	m_nextInstruction.distance = 20;
	m_nextInstruction.type = "roundabout";
	m_nextInstruction.name = "";
	m_nextInstruction.exitNumber = 3;
	m_nextInstruction.spoken = false;
}

TEST( AnnounceRoundabout, AudioIndex)
{
	instructionGenerator.createAnnounceRoundabout();
	instructionGenerator.determineSpeech();
	CHECK_EQUAL( instructionGenerator.m_currentInstruction.audiofileIndex, 10 );
}


void InstructionGenerator::createStraightforwardTurn(){
	m_previousInstruction.init();
	m_currentInstruction.init();
	m_nextInstruction.init();

	m_currentInstruction.branchingPossible = true;
	m_currentInstruction.direction = 0;
	m_currentInstruction.distance = 100;
	m_currentInstruction.type = "residential";
	m_currentInstruction.name = "Mozartstrasse";
	m_currentInstruction.exitNumber = 0;
	m_currentInstruction.spoken = false;
}

TEST( StraightforwardTurn, AudioIndex)
{
	instructionGenerator.createStraightforwardTurn();
	instructionGenerator.determineSpeech();
	CHECK_EQUAL( instructionGenerator.m_currentInstruction.audiofileIndex, -1 );
}


void InstructionGenerator::createLeaveMotorway(){
	m_previousInstruction.init();
	m_currentInstruction.init();
	m_nextInstruction.init();

	m_currentInstruction.branchingPossible = true;
	m_currentInstruction.direction = 0;
	m_currentInstruction.distance = 100;
	m_currentInstruction.type = "motorway";
	m_currentInstruction.name = "A5";
	m_currentInstruction.exitNumber = 0;
	m_currentInstruction.spoken = false;

	m_nextInstruction.branchingPossible = true;
	m_nextInstruction.direction = 0;
	m_nextInstruction.distance = 100;
	m_nextInstruction.type = "motorway_link";
	m_nextInstruction.name = "";
	m_nextInstruction.exitNumber = 0;
	m_nextInstruction.spoken = false;
}

TEST( LeaveMotorway, AudioIndex)
{
	instructionGenerator.createLeaveMotorway();
	instructionGenerator.determineSpeech();
	CHECK_EQUAL( instructionGenerator.m_currentInstruction.audiofileIndex, 17 );
}


void InstructionGenerator::createMotorwayLinkBranch(){
	m_previousInstruction.init();
	m_currentInstruction.init();
	m_nextInstruction.init();

	m_currentInstruction.branchingPossible = true;
	m_currentInstruction.direction = 1;
	m_currentInstruction.distance = 100;
	m_currentInstruction.type = "motorway_link";
	m_currentInstruction.name = "";
	m_currentInstruction.exitNumber = 0;
	m_currentInstruction.spoken = false;

	m_nextInstruction.branchingPossible = true;
	m_nextInstruction.direction = 0;
	m_nextInstruction.distance = 100;
	m_nextInstruction.type = "motorway_link";
	m_nextInstruction.name = "";
	m_nextInstruction.exitNumber = 0;
	m_nextInstruction.spoken = false;
}

TEST( MotorwayLinkBranch, AudioIndex)
{
	instructionGenerator.createMotorwayLinkBranch();
	instructionGenerator.determineSpeech();
	// 1 slightly right, 7 slightly left
	CHECK_EQUAL( instructionGenerator.m_currentInstruction.audiofileIndex, 1 );
	CHECK_EQUAL( instructionGenerator.speechRequired(), true );
}


void InstructionGenerator::createDontSpeakAgain(){
	m_previousInstruction.init();
	m_currentInstruction.init();
	m_nextInstruction.init();

	m_previousInstruction.branchingPossible = true;
	m_previousInstruction.direction = 2;
	m_previousInstruction.distance = 100;
	m_previousInstruction.type = "residential";
	m_previousInstruction.name = "Heinrichstrasse";
	m_previousInstruction.exitNumber = 0;
	m_previousInstruction.spoken = true;

	m_currentInstruction.branchingPossible = true;
	m_currentInstruction.direction = 2;
	m_currentInstruction.distance = 95;
	m_currentInstruction.type = "residential";
	m_currentInstruction.name = "Heinrichstrasse";
	m_currentInstruction.exitNumber = 0;
	m_currentInstruction.spoken = false;
}

TEST( DontSpeakAgain, SpeechRequired)
{
	instructionGenerator.createDontSpeakAgain();
	instructionGenerator.determineSpeech();
	CHECK_EQUAL( instructionGenerator.speechRequired(), false );
}

#endif // CPPUNITLITE

