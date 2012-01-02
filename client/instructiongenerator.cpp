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
	initialize();
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

	// Compute edges' directions, type and name strings.
	for ( int i = 0; i < edges.size(); i++ ){
		edges[i].exitNumber = -1;
		edges[i].speechRequired = false;
		edges[i].announced = false;
		edges[i].preAnnounced = false;

		endNode += edges[i].length;
		if ( i < edges.size() -1 ){
			edges[i].direction = direction( nodes[ endNode -1 ].coordinate, nodes[ endNode ].coordinate, nodes[ endNode +1 ].coordinate );
		}
		else{
			edges[i].direction = -1;
		}

		router->GetType( &typeString, edges[i].type );
		edges[i].typeString = typeString;
		router->GetName( &nameString, edges[i].name );
		edges[i].nameString = nameString;
	}

	// Determine instructions dependent on the edges' types
	for ( int i = 0; i < edges.size() -1; i++ ){
		if ( edges[i].typeString == "roundabout" && edges[i].exitNumber < 1 ){
			// qDebug() << "Roundabout edges are treated separately";
			edges[i].speechRequired = false;
		}
		else if ( edges[i].typeString == "motorway" && edges[i +1].typeString == "motorway_link" ){
			// qDebug() << "Leaving a motorway";
			edges[i].instructionFilename = ( m_audioFilenames[17] );
			edges[i].instructionString = m_instructionStrings[17];
			edges[i].instructionIcon = m_iconFilenames[17];
			edges[i].speechRequired = true;
		}
		else if ( edges[i].typeString == "trunk" && edges[i +1].typeString == "trunk_link" ){
			// qDebug() << "Leaving a trunk";
			edges[i].instructionFilename = ( m_audioFilenames[18] );
			edges[i].instructionString = m_instructionStrings[18];
			edges[i].instructionIcon = m_iconFilenames[18];
			edges[i].speechRequired = true;
		}
		else if ( edges[i].typeString != "motorway" && edges[i].typeString != "motorway_link" && edges[i +1].typeString == "motorway_link" ){
			// qDebug() << "Entering a motorway";
			edges[i].instructionFilename = ( m_audioFilenames[19] );
			edges[i].instructionString = m_instructionStrings[19];
			edges[i].instructionIcon = m_iconFilenames[19];
			edges[i].speechRequired = true;
		}
		else if ( edges[i].typeString != "trunk" && edges[i].typeString != "trunk_link" && edges[i +1].typeString == "trunk_link" ){
			// qDebug() << "Entering a trunk";
			edges[i].instructionFilename = ( m_audioFilenames[20] );
			edges[i].instructionString = m_instructionStrings[20];
			edges[i].instructionIcon = m_iconFilenames[20];
			edges[i].speechRequired = true;
		}
		else if ( edges[i].branchingPossible && edges[i].direction == 0 && edges[i].typeString == "motorway_link" ){
			// qDebug() << "Announcing a branch on motorway or trunk links";
			edges[i].instructionFilename = ( m_audioFilenames[edges[i].direction] );
			edges[i].instructionString = m_instructionStrings[edges[i].direction];
			edges[i].instructionIcon = m_iconFilenames[edges[i].direction];
			edges[i].speechRequired = true;
		}
		else if ( edges[i].branchingPossible && edges[i].direction == 0 && edges[i].typeString == "trunk_link" ){
			// qDebug() << "Announcing \"head straightforward\" on motorway or trunk links";
			edges[i].instructionFilename = ( m_audioFilenames[edges[i].direction] );
			edges[i].instructionString = m_instructionStrings[edges[i].direction];
			edges[i].instructionIcon = m_iconFilenames[edges[i].direction];
			edges[i].speechRequired = true;
		}
		else if ( edges[i].branchingPossible && edges[i].direction != 0 ){
			// qDebug() << "Announcing an ordinary turn";
			edges[i].instructionFilename = ( m_audioFilenames[edges[i].direction] );
			edges[i].instructionString = m_instructionStrings[edges[i].direction];
			edges[i].instructionIcon = m_iconFilenames[edges[i].direction];
			edges[i].speechRequired = true;
		}
		else{
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
			// Announcing the roundabout on the edge right before the roundabout, in case there is one
			if ( firstRoundaboutEdge > 0 ){
				firstRoundaboutEdge--;
			}
			edges[ firstRoundaboutEdge ].exitNumber = exitAmount;
			edges[ firstRoundaboutEdge ].instructionFilename = ( m_audioFilenames[edges[firstRoundaboutEdge].exitNumber +7] );
			edges[ firstRoundaboutEdge ].speechRequired = true;
			exitAmount = 0;
			firstRoundaboutEdge = -1;
		}
	}
}


void InstructionGenerator::requestSpeech(){
	// This method is called by routinglogic::routeChanged().

	if ( !m_speechEnabled ){
		return;
	}

	// TODO: Make the reference to the route a member variable
	QVector< IRouter::Edge >& edges = RoutingLogic::instance()->edges();
	if ( edges.size() < 1 ){
		return;
	}

	double announceDistance1st = announceDistanceFirst();
	double announceDistance2nd = announceDistanceSecond();

	// Determine the next two edges to announce
	double branchDistance = 0;
	QVector<int> edgesToAnnounce;
	QVector<double> branchDistances;
	for ( int i = 0; i < edges.size(); i++ ){
		branchDistance += edges[i].distance;
		if ( edges[i].speechRequired ){
			edgesToAnnounce.append( i );
			branchDistances.append( branchDistance );
		}
		// Avoid traversing the complete route
		if ( branchDistance > announceDistance1st ){
			break;
		}
		else if ( edgesToAnnounce.size() == 2 ){
			break;
		}
	}

	assert( edgesToAnnounce.size() == branchDistances.size() );

	if( edgesToAnnounce.size() == 0 ){
		qDebug() << "No branches to announce foreseen\n";
		return;
	}

	int firstEdgeToAnnounce = edgesToAnnounce[0];
	int nextEdgeToAnnounce = 0;
	if( edgesToAnnounce.size() > 1 ){
		nextEdgeToAnnounce = edgesToAnnounce[1];
	}

	bool preannounceFirst = edgesToAnnounce.size() > 0;
	bool finalannounceFirst = edgesToAnnounce.size() > 0;
	bool preannounceSecond = edgesToAnnounce.size() > 1;

	// TODO: The following lines are write only - refactoring required.
	// A couple of circumstances that prevent the first branch from being (pre)announced
	if ( !edges[firstEdgeToAnnounce].speechRequired ){
		preannounceFirst = false;
		finalannounceFirst = false;
		qDebug() << "First branch does not require any speech output.";
	}
	else if ( edges[firstEdgeToAnnounce].preAnnounced ){
		preannounceFirst = false;
		qDebug() << "First branch already got preannounced.";
	}
	else if ( edges[firstEdgeToAnnounce].announced ){
		finalannounceFirst = false;
		qDebug() << "First branch already got announced.";
	}
	if ( edges[firstEdgeToAnnounce].announced ){
		finalannounceFirst = false;
		qDebug() << "First branch already got announced.";
	}
	if ( announceDistance1st < announceDistance2nd *3 ){
		preannounceFirst = false;
		qDebug() << "First branch distance is too short for being preannounced.";
	}
	if ( branchDistances[0] > announceDistance1st ){
		preannounceFirst = false;
		preannounceSecond = false;
		qDebug() << "First branch's preannounce distance was not reached yet.";
	}
	if ( branchDistances[0] > announceDistance2nd ){
		finalannounceFirst = false;
		preannounceSecond = false;
		qDebug() << "First branch's announce distance was not reached yet.";
	}

	// A couple of circumstances that prevent the next edge from being announced
	if ( nextEdgeToAnnounce < 1 ){
		preannounceSecond = false;
		qDebug() << "Second branch not available yet.";
	}
	else if ( edges[nextEdgeToAnnounce].preAnnounced ){
		preannounceSecond = false;
		qDebug() << "Second branch was already preannounced.";
	}
	if ( preannounceFirst ){
		preannounceSecond = false;
		qDebug() << "A possibly available second branch will not be preannounced as the first branch gets preannounced.";
	}

	QStringList instructions;
	// qDebug() << edges[0].preAnnounced << edges[0].announced;
	if ( preannounceFirst || finalannounceFirst ){
		instructions.append( m_distanceFilenames[distanceFileindex( branchDistances[0] )] );
		instructions.append( edges[firstEdgeToAnnounce].instructionFilename );
	}
	if ( preannounceFirst ){
		edges[firstEdgeToAnnounce].preAnnounced = true;
		qDebug() << "First branch being preannounced.";
	}
	if ( finalannounceFirst && !preannounceFirst ){
		edges[firstEdgeToAnnounce].announced = true;
		qDebug() << "First branch being announced.";
	}
	if ( preannounceSecond ){
		// Append something like "After the first turn…"
		instructions.append( m_audioFilenames[22] );
		instructions.append( m_distanceFilenames[distanceFileindex( edges[nextEdgeToAnnounce].distance )] );
		instructions.append( edges[nextEdgeToAnnounce].instructionFilename );
		edges[nextEdgeToAnnounce].preAnnounced = true;
		qDebug() << "Second branch being announced.";
	}

	if ( instructions.size() > 0 ){
		// Add announcement jingle
		instructions.prepend( m_audioFilenames[21] );
		Audio::instance()->speak( instructions );
	}
	else{
				qDebug() << "No instructions have been generated.";
	}
	qDebug() << "\n--------";
}


double InstructionGenerator::currentSpeed() {
	// Speed is in kilometers per hour.
	// In case there is no GPS signal, -1 is returned.
	double currentSpeed = RoutingLogic::instance()->groundSpeed();
	if ( currentSpeed < 0 ){
		currentSpeed = 80;
	}
	return currentSpeed;
}


double InstructionGenerator::announceDistanceFirst() {

	// Speed is in kilometers per hour.
	// In case there is no GPS signal, -1 is returned.
	double speed = currentSpeed();

	// Some possibly reasonable values (0.2 seconds per km/h):
	//  1s	0.27m	1.4m/s	5km/h	pedestrian
	// 2.5s	0.7m	3.5m/s	12.5km/h	cyclist
	//  5s	  35m	 7m/s	 25km/h	residential areas
	// 10s	 140m	14m/s	 50km/h	inner city
	// 15s	 315m	21m/s	 75km/h	primaries
	// 20s	 560m	28m/s	100km/h	trunks
	// 30s	1260m	42m/s	150km/h	highways
	// 40s	2222m	56m/s	200km/h	highways

	// 0.2 seconds per km/h, but at least 3 seconds respectively 10m before a branch
	double seconds = speed * 0.2;
	if ( seconds < 5.0 ){
		seconds = 5.0;
	}

	return announceDistance( speed, seconds );
}


double InstructionGenerator::announceDistanceSecond() {
	return announceDistance( currentSpeed(), 5.0 );
}


double InstructionGenerator::announceDistance( double currentSpeed, int seconds ) {

	// Speed is in kilometers per hour.
	double speechDistance = currentSpeed * seconds * 1000 / 3600;
	if ( speechDistance < 10 ){
		speechDistance = 10;
	}

// qDebug() << "Speed, speech distance:" << currentSpeed << speechDistance;

	return speechDistance;
}


void InstructionGenerator::instructions( QStringList* labels, QStringList* icons, int instructionAmount )
{

	QVector< IRouter::Edge >& edges = RoutingLogic::instance()->edges();
	if ( edges.size() < 1 ){
		return;
	}

	QStringList instructions;
	QStringList images;
	double distance = 0.0;
	int amount = 0;

	for ( int i = 0; i < edges.size(); i++ ){
		distance += edges[i].distance;
		if ( edges[i].instructionString != "" ){
			instructions.append( edges[i].instructionString );
			if ( i < edges.size() -1 && edges[i +1].nameString != "" && edges[i +1].typeString != "roundabout" ){
				instructions.last().append( " " ).append( tr( "into %1" ).arg( edges[i +1].nameString ) );
			}
			instructions.last().append( distanceString( distance ) );
			images.append( edges[i].instructionIcon );
			distance = 0.0;
			amount++;
			if ( amount == instructionAmount ){
				break;
			}
		}
	}

	*labels = instructions;
	*icons = images;
}


int InstructionGenerator::direction( UnsignedCoordinate first, UnsignedCoordinate second, UnsignedCoordinate third ) {
	// TODO: It would be *very* useful to take more than one node of an edge into account.
	// There are too many occasions where the current approach either leads to no or to too much instructions.
	double x1 = ( double ) second.x - first.x; // a = (x1,y1)
	double y1 = ( double ) second.y - first.y;
	double x2 = ( double ) third.x - second.x; // b = (x2, y2 )
	double y2 = ( double ) third.y - second.y;
	int direction = 0;
	// Counterclockwise angle
	int angle = ( atan2( y1, x1 ) - atan2( y2, x2 ) ) * 180 / M_PI + 720;
	angle %= 360;
	// qDebug() << "Angle:" << angle;
	if ( angle > 8.0 && angle <= 45.0 ){
		direction = 7;
	}
	else if ( angle > 45.0  && angle <= 135 ){
		direction = 6;
	}
	else if ( angle > 135 && angle <= 172 ){
		direction = 5;
	}
	else if ( angle > 172 && angle <= 188 ){
		direction = 4;
	}
	else if ( angle > 188 && angle <= 225 ){
		direction = 3;
	}
	else if ( angle > 225 && angle <= 315 ){
		direction = 2;
	}
	else if ( angle > 315 && angle <= 352.0 ){
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
		distanceString.prepend( " " );
		distanceString.prepend( tr( "in" ) );
		distanceString.prepend( " " );
	}
	return distanceString;
}


int InstructionGenerator::distanceFileindex( double distance )
{
	// TODO: i18n
	int distanceFileindex = 0;

	if ( distance < 35 )
		distanceFileindex = 0;
	else if ( distance < 75 )
		distanceFileindex = 1;
	else if ( distance < 150 )
		distanceFileindex = 2;
	else if ( distance < 350 )
		distanceFileindex = 3;
	else if ( distance < 650 )
		distanceFileindex = 4;
	else if ( distance < 900 )
		distanceFileindex = 5;
	else if ( distance < 1500 )
		distanceFileindex = 6;
	else
		distanceFileindex = 7;

	return distanceFileindex;
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


void InstructionGenerator::initialize()
{
	QSettings settings( "MoNavClient" );
	m_speechEnabled = settings.value( "SpeechEnabled", true ).toBool();

	// index 0
	m_audioFilenames.append( "instructions-head-straightforward" );
	m_instructionStrings.append( tr( "Head straightforward" ) );
	m_iconFilenames.append( "forward" );

	m_audioFilenames.append( "instructions-turn-slightly-right" );
	m_instructionStrings.append( tr( "Turn slightly right" ) );
	m_iconFilenames.append( "slightly_right" );

	m_audioFilenames.append( "instructions-turn-right" );
	m_instructionStrings.append( tr( "Turn right" ) );
	m_iconFilenames.append( "right" );

	m_audioFilenames.append( "instructions-turn-sharply-right" );
	m_instructionStrings.append( tr( "Turn sharply right" ) );
	m_iconFilenames.append( "sharply_right" );

	m_audioFilenames.append( "instructions-turn-u" );
	m_instructionStrings.append( tr( "Take a U-turn" ) );
	m_iconFilenames.append( "backward" );

	// index 5
	m_audioFilenames.append( "instructions-turn-sharply-left" );
	m_instructionStrings.append( tr( "Turn sharply left" ) );
	m_iconFilenames.append( "sharply_left" );

	m_audioFilenames.append( "instructions-turn-left" );
	m_instructionStrings.append( tr( "Turn left" ) );
	m_iconFilenames.append( "left" );

	m_audioFilenames.append( "instructions-turn-slightly-left" );
	m_instructionStrings.append( tr( "Turn slightly left" ) );
	m_iconFilenames.append( "slightly_left" );

	m_audioFilenames.append( "instructions-roundabout_01" );
	m_instructionStrings.append( tr( "Take the 1st exit" ) );
	m_iconFilenames.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_02" );
	m_instructionStrings.append( tr( "Take the 2nd exit" ) );
	m_iconFilenames.append( "roundabout" );

	// index 10
	m_audioFilenames.append( "instructions-roundabout_03" );
	m_instructionStrings.append( tr( "Take the 3rd exit" ) );
	m_iconFilenames.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_04" );
	m_instructionStrings.append( tr( "Take the 4th exit" ) );
	m_iconFilenames.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_05" );
	m_instructionStrings.append( tr( "Take the 5th exit" ) );
	m_iconFilenames.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_06" );
	m_instructionStrings.append( tr( "Take the 6th exit" ) );
	m_iconFilenames.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_07" );
	m_instructionStrings.append( tr( "Take the 7th exit" ) );
	m_iconFilenames.append( "roundabout" );

	// index 15
	m_audioFilenames.append( "instructions-roundabout_08" );
	m_instructionStrings.append( tr( "Take the 8th exit" ) );
	m_iconFilenames.append( "roundabout" );

	m_audioFilenames.append( "instructions-roundabout_09" );
	m_instructionStrings.append( tr( "Take the 9th exit" ) );
	m_iconFilenames.append( "roundabout" );

	m_audioFilenames.append( "instructions-leave-motorway" );
	m_instructionStrings.append( tr( "Leave the motorway" ) );
	m_iconFilenames.append( "slightly_right" );

	m_audioFilenames.append( "instructions-leave-trunk" );
	m_instructionStrings.append( tr( "Leave the trunk" ) );
	m_iconFilenames.append( "slightly_right" );

	m_audioFilenames.append( "instructions-take-motorway-ramp" );
	m_instructionStrings.append( tr( "Take the ramp to the motorway" ) );
	m_iconFilenames.append( "slightly_right" );

	// index 20
	m_audioFilenames.append( "instructions-take-trunk-ramp" );
	m_instructionStrings.append( tr( "Take the ramp to the trunk" ) );
	m_iconFilenames.append( "slightly_right" );

	// index 21
	m_audioFilenames.append( "instructions-announce" );
	// index 22
	m_audioFilenames.append( "instructions-and-then" );

	m_distanceFilenames.append( "instructions-distance-now" );
	m_distanceFilenames.append( "instructions-distance-50m" );
	m_distanceFilenames.append( "instructions-distance-100m" );
	m_distanceFilenames.append( "instructions-distance-200m" );
	m_distanceFilenames.append( "instructions-distance-500m" );
	m_distanceFilenames.append( "instructions-distance-800m" );
	m_distanceFilenames.append( "instructions-distance-1km" );
	m_distanceFilenames.append( "instructions-distance-2km" );
	
	QLocale DefaultLocale;
	m_language = DefaultLocale.name();
	m_language.truncate( 2 );

	for ( int i = 0; i < m_distanceFilenames.size(); i++ ){
		m_distanceFilenames[i].append( ".wav" );
		m_distanceFilenames[i].prepend( "/" );
		m_distanceFilenames[i].prepend( m_language );
		m_distanceFilenames[i].prepend( ":/audio/" );
	}

	for ( int i = 0; i < m_audioFilenames.size(); i++ ){
		m_audioFilenames[i].append( ".wav" );
		m_audioFilenames[i].prepend( "/" );
		m_audioFilenames[i].prepend( m_language );
		m_audioFilenames[i].prepend( ":/audio/" );
	}

	for ( int i = 0; i < m_iconFilenames.size(); i++ ){
		m_iconFilenames[i].append( ".png" );
		m_iconFilenames[i].prepend( ":/images/directions/" );
	}
}

