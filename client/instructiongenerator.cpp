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
	m_labels.append( tr( "Head straightforward" ) );
	m_icons.append( "forward" );

	m_audioFilenames.append( "instructions-turn-slightly-right" );
	m_labels.append( tr( "Turn slightly right" ) );
	m_icons.append( "slightly_right" );

	m_audioFilenames.append( "instructions-turn-right" );
	m_labels.append( tr( "Turn right" ) );
	m_icons.append( "right" );

	m_audioFilenames.append( "instructions-turn-sharply-right" );
	m_labels.append( tr( "Turn sharply right" ) );
	m_icons.append( "sharply_right" );

	m_audioFilenames.append( "instructions-turn-u" );
	m_labels.append( tr( "Take a U-turn" ) );
	m_icons.append( "backward" );

	m_audioFilenames.append( "instructions-turn-sharply-left" );
	m_labels.append( tr( "Turn sharply left" ) );
	m_icons.append( "sharply_left" );

	m_audioFilenames.append( "instructions-turn-left" );
	m_labels.append( tr( "Turn left" ) );
	m_icons.append( "left" );

	m_audioFilenames.append( "instructions-turn-slightly-left" );
	m_labels.append( tr( "Turn slightly left" ) );
	m_icons.append( "slightly_left" );

	m_audioFilenames.append( "instructions-roundabout_01" );
	m_labels.append( tr( "Take the 1st exit" ) );
	m_icons.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_02" );
	m_labels.append( tr( "Take the 2nd exit" ) );
	m_icons.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_03" );
	m_labels.append( tr( "Take the 3rd exit" ) );
	m_icons.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_04" );
	m_labels.append( tr( "Take the 4th exit" ) );
	m_icons.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_05" );
	m_labels.append( tr( "Take the 5th exit" ) );
	m_icons.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_06" );
	m_labels.append( tr( "Take the 6th exit" ) );
	m_icons.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_07" );
	m_labels.append( tr( "Take the 7th exit" ) );
	m_icons.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_08" );
	m_labels.append( tr( "Take the 8th exit" ) );
	m_icons.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_09" );
	m_labels.append( tr( "Take the 9th exit" ) );
	m_icons.append( "roundabout" );

	m_audioFilenames.append( "instructions-leave-motorway" );
	m_labels.append( tr( "Leave the motorway" ) );
	m_icons.append( "slightly_right" );

	m_audioFilenames.append( "instructions-leave-trunk" );
	m_labels.append( tr( "Leave the trunk" ) );
	m_icons.append( "slightly_right" );

	m_audioFilenames.append( "instructions-take-motorway-ramp" );
	m_labels.append( tr( "Take the ramp to the motorway" ) );
	m_icons.append( "slightly_right" );

	m_audioFilenames.append( "instructions-take-trunk-ramp" );
	m_labels.append( tr( "Take the ramp to the trunk" ) );
	m_icons.append( "slightly_right" );

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
	IRouter* router = MapData::instance()->router();
	if ( router == NULL || edges.size() < 1 || nodes.empty() ) {
		return;
	}

	QString typeString;
	QString nameString;
	int endNode = 0;

	// Compute edges' lenths, directions, type and name strings.
	for ( int i = 0; i < edges.size(); i++ ){

		edges[i].audiofileIndex = -1;
		edges[i].exitNumber = -1;
		edges[i].distance = 0;
		edges[i].speechRequired = false;

		for ( int node = endNode; node < endNode + edges[i].length; node++ ){
			 edges[i].distance += nodes[ node ].coordinate.ToGPSCoordinate().ApproximateDistance( nodes[ node +1 ].coordinate.ToGPSCoordinate() );
		}

		endNode += edges[i].length;
		if ( i < edges.size() -1 ){
			edges[i].direction = angle( nodes[ endNode -1 ].coordinate, nodes[ endNode ].coordinate, nodes[ endNode +1 ].coordinate );
		}
		else{
			edges[i].direction = -1;
		}

		router->GetType( &typeString, edges[i].type );
		edges[i].typeString = typeString;
		router->GetName( &nameString, edges[i].name );
		edges[i].nameString = nameString;
	}

	// Determine audio samples dependent on the edges' types
	for ( int i = 0; i < edges.size() -1; i++ ){
		if ( edges[i].typeString == "roundabout" && edges[i].exitNumber < 1 ){
			// qDebug() << "Roundabout edges are treated below";
			edges[i].audiofileIndex = -1;
			edges[i].speechRequired = false;
		}
		else if ( edges[i].typeString == "motorway" && edges[i +1].typeString == "motorway_link" ){
			// qDebug() << "Leaving the motorway";
			edges[i].audiofileIndex = 17;
			edges[i].speechRequired = true;
		}
		else if ( edges[i].typeString == "trunk" && edges[i +1].typeString == "trunk_link" ){
			// qDebug() << "Leaving the trunk";
			edges[i].audiofileIndex = 18;
			edges[i].speechRequired = true;
		}
		else if ( edges[i].typeString != "motorway" && edges[i].typeString != "motorway_link" && edges[i +1].typeString == "motorway_link" ){
			// qDebug() << "Entering the motorway";
			edges[i].audiofileIndex = 19;
			edges[i].speechRequired = true;
		}
		else if ( edges[i].typeString != "trunk" && edges[i].typeString != "trunk_link" && edges[i +1].typeString == "trunk_link" ){
			// qDebug() << "Entering the trunk";
			edges[i].audiofileIndex = 20;
			edges[i].speechRequired = true;
		}
		else if ( edges[i].branchingPossible && edges[i].direction == 0 && edges[i].typeString == "motorway_link" ){
			// qDebug() << "Announcing a branch on motorway or trunk links";
			edges[i].audiofileIndex = edges[i].direction;
			edges[i].speechRequired = true;
		}
		else if ( edges[i].branchingPossible && edges[i].direction == 0 && edges[i].typeString == "trunk_link" ){
			// qDebug() << "Announcing a branch on motorway or trunk links";
			edges[i].audiofileIndex = edges[i].direction;
			edges[i].speechRequired = true;
		}
		else if ( edges[i].branchingPossible && edges[i].direction != 0 ){
			// qDebug() << "Announcing an ordinary turn";
			edges[i].audiofileIndex = edges[i].direction;
			edges[i].speechRequired = true;
		}
		else{
			// qDebug() << "No speech required" << edges[i].branchingPossible;
			edges[i].speechRequired = false;
		}
	}

	// Roundabout detection
	int exitAmount = 0;
	int firstRoundaboutEdge = -1;
	for ( int i = 0; i < edges.size(); i++ ){
		if ( edges[i].typeString == "roundabout" ){
			if ( firstRoundaboutEdge == -1 ){
				firstRoundaboutEdge = i;
			}
			if ( edges[i].branchingPossible ){
				exitAmount++;
			}
		}
		else if ( exitAmount > 0 ){
			if ( firstRoundaboutEdge > 0 ){
				// Announcing the roundabout on the edge before the roundabout
				firstRoundaboutEdge--;
			}
			edges[ firstRoundaboutEdge ].exitNumber = exitAmount;
			edges[ firstRoundaboutEdge ].audiofileIndex = edges[firstRoundaboutEdge].exitNumber +7;
			edges[ firstRoundaboutEdge ].speechRequired = true;
			exitAmount = 0;
			firstRoundaboutEdge = -1;
		}
	}
}


void InstructionGenerator::requestSpeech(){
	if ( !m_speechEnabled ){
		qDebug() << "Speech is disabled.";
		return;
	}

	QVector< IRouter::Edge >& edges = RoutingLogic::instance()->edges();
	if ( edges.size() < 1 ){
		qDebug() << "No edges present.";
		return;
	}

	if ( !edges[0].speechRequired ){
		qDebug() << "No speech necessary on" << edges[0].typeString << edges[0].nameString << edges[0].speechRequired;
		return;
	}

	if ( edges[0].distance > speechDistance() ){
		qDebug() << edges[0].distance << "greater" << speechDistance();
		return;
	}

	QString audioFilename = m_audioFilenames[ edges[0].audiofileIndex ];

	audioFilename.prepend( "/" );
	audioFilename.prepend( "m_language" );
	audioFilename.prepend( ":/audio/" );
	audioFilename.append( ".wav" );
	Audio::instance()->speak( audioFilename );
	edges[0].speechRequired = false;
}


double InstructionGenerator::speechDistance() {

	const RoutingLogic::GPSInfo& gpsInfo = RoutingLogic::instance()->gpsInfo();
	double currentSpeed = 0;

	// Speed is in meters per second
	// 10 m per second are equal to 36 km/h
	currentSpeed = gpsInfo.position.IsValid() ? gpsInfo.groundSpeed : 10;

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
	return speechDistance;
}


void InstructionGenerator::instructions( QStringList* labels, QStringList* icons, int maxSeconds )
{
	QVector< IRouter::Edge >& edges = RoutingLogic::instance()->edges();
	if ( edges.size() < 1 ){
		return;
	}

	QStringList instructions;
	QStringList images;
	double distance = 0;

	for ( int i = 0; i < edges.size(); i++ ){
		distance += edges[i].distance;
		if ( edges[i].audiofileIndex >= 0 && edges[i].audiofileIndex < m_labels.size() ){
			instructions.append( m_labels[ edges[i].audiofileIndex ] );
			if ( i < edges.size() -1 && edges[i +1].nameString != "" && edges[i +1].typeString != "roundabout" ){
				instructions.last().append( tr( " into %1" ).arg( edges[i +1].nameString ) );
			}
			instructions.last().append( distanceString( distance ) );
			distance = 0;
			images.append( m_icons[ edges[i].audiofileIndex ] );
			images.last().append( ".png" );
			images.last().prepend( ":/images/directions/" );
		}
	}

	*labels = instructions;
	*icons = images;
}


int InstructionGenerator::angle( UnsignedCoordinate first, UnsignedCoordinate second, UnsignedCoordinate third ) {
	double x1 = ( double ) second.x - first.x; // a = (x1,y1)
	double y1 = ( double ) second.y - first.y;
	double x2 = ( double ) third.x - second.x; // b = (x2, y2 )
	double y2 = ( double ) third.y - second.y;
	int direction = 0;
	// Counterclockwise angle
	int angle = ( atan2( y1, x1 ) - atan2( y2, x2 ) ) * 180 / M_PI + 720;
	angle %= 360;

	if ( angle > 5.0 && angle <= 45.0 ){
		direction = 7;
	}
	else if ( angle > 45.0  && angle <= 135 ){
		direction = 6;
	}
	else if ( angle > 135 && angle <= 175 ){
		direction = 5;
	}
	else if ( angle > 175 && angle <= 185 ){
		direction = 4;
	}
	else if ( angle > 185 && angle <= 225 ){
		direction = 3;
	}
	else if ( angle > 225 && angle <= 315 ){
		direction = 2;
	}
	else if ( angle > 315 && angle <= 355.0 ){
		direction = 1;
	}

	return direction;
}

QString InstructionGenerator::distanceString( double distance )
{
	// TODO: i18n
	QString distanceString;
	QString unit;

	if ( distance > 20 ) {
		if ( distance < 100 )
			distanceString = QString( "%1m" ).arg( ( int ) distance );
		else if ( distance < 1000 )
			distanceString = QString( "%1m" ).arg( ( int ) distance / 10 * 10 );
		else if ( distance < 10000 )
			distanceString = QString( "%1.%2km" ).arg( ( int ) distance / 1000 ).arg( ( ( int ) distance / 100 ) % 10 );
		else
			distanceString = QString( "%1km" ).arg( ( int ) distance / 1000 );
	}
	if ( distanceString != "" ){
		distanceString.prepend( " in " );
	}
	return distanceString;
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

