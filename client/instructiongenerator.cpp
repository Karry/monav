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


InstructionGenerator::InstructionGenerator()
{
	QSettings settings( "MoNavClient" );
	m_speechEnabled = settings.value( "SpeechEnabled", true ).toBool();

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


InstructionGenerator* InstructionGenerator::instance()
{
	static InstructionGenerator instructionGenerator;
	return &instructionGenerator;
}


void InstructionGenerator::createInstructions( QVector< IRouter::Edge >& edges, QVector< IRouter::Node >& nodes )
{
	// m_pathEdges = edges;

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

	// Roundabout detection
	// TODO: Isn't it ugly?
	int exitAmount = 0;
	int preRoundaboutEdge = -1;
	for ( int i = 0; i < edges.size(); i++ ){
		if ( edges[i].typeString == "roundabout" ){
			if ( exitAmount == 0 ){
				preRoundaboutEdge = i -1;
			}
			exitAmount++;
		}
		else if ( exitAmount > 0 ){
			if ( preRoundaboutEdge < 0 ){
				preRoundaboutEdge = 0;
			}
			edges[ preRoundaboutEdge ].exitNumber = exitAmount;
			// qDebug() << "Announce exit" << edges[ preRoundaboutEdge ].exitNumber << "on edge" << preRoundaboutEdge;
			exitAmount = 0;
			preRoundaboutEdge = -1;
		}
	}


	// TODO: Refactoring
	for ( int i = 0; i < edges.size() -1; i++ ){
		if ( edges[i].typeString == "roundabout" && edges[i].exitNumber < 1 ){
			// qDebug() << "No speech at all in roundabouts";
			edges[i].audiofileIndex = -1;
		}
		else if ( edges[i].typeString == "motorway" && edges[i +1].typeString == "motorway_link" ){
			// qDebug() << "Leaving the motorway";
			edges[i].audiofileIndex = 17;
		}
		else if ( edges[i].typeString == "trunk" && edges[i +1].typeString == "trunk_link" ){
			// qDebug() << "Leaving the trunk";
			edges[i].audiofileIndex = 18;
		}
		else if ( edges[i].branchingPossible && edges[i].direction != 0 ){
			// qDebug() << "Announcing an ordinary turn";
			edges[i].audiofileIndex = edges[i].direction;
		}
		else{
			// qDebug() << "No speech at all required";
			edges[i].audiofileIndex = -1;
		}
	}
}


void InstructionGenerator::requestSpeech(){

	if ( !m_speechEnabled ){
		return;
	}

	// TODO: Use a reference
	QVector< IRouter::Edge > edges = RoutingLogic::instance()->edges();

	if ( edges[0].audiofileIndex < 0 || edges[0].audiofileIndex >= m_audioFilenames.size() ){
		return;
	}

	if ( edges[0].spoken ){
		return;
	}

	if ( edges[0].distance > speechDistance() ){
		return;
	}

	QString audioFilename = m_audioFilenames[ edges[0].audiofileIndex ];
	audioFilename.prepend( "/" );
	audioFilename.prepend( m_language );
	audioFilename.prepend( ":/audio/" );
	audioFilename.append( ".wav" );
	Audio::instance()->speak( audioFilename );
	edges[0].spoken = true;
}


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
	// Mainly for debugging on desktop machines with no GPS receiver available
	if ( speechDistance < 10 ){
		speechDistance = 10;
	}
	return speechDistance;
}


void InstructionGenerator::routeChanged()
{
	requestSpeech();
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


void InstructionGenerator::setSpeechEnabled( bool enabled )
{
	m_speechEnabled = enabled;
	QSettings settings( "MoNavClient" );
	settings.setValue( "SpeechEnabled", m_speechEnabled );
}


bool InstructionGenerator::speechEnabled()
{
	return m_speechEnabled;
}

