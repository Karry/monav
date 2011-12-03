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
	// connect( RoutingLogic::instance(), SIGNAL(routeChanged()), this, SLOT(routeChanged()) );
	// connect( this, SIGNAL(routeChanged()), InstructionGenerator::instance(), SLOT(routeChanged()) );
}


InstructionGenerator::InstructionGenerator()
{
	// connect( RoutingLogic::instance(), SIGNAL(routeChanged()), this, SLOT(routeChanged()) );
	unsigned m_lastNameID = 0;
	unsigned m_lastTypeID = 0;
	QString m_lastName = "";
	QString m_lastType = "";
	bool m_branchingPossible = false;
	double m_distance = std::numeric_limits< double >::max();
	int m_direction = 0;
	int m_exitNumber = 0;
}


InstructionGenerator::~InstructionGenerator()
{

}


void InstructionGenerator::generateInstructions()
{
	// qDebug() << "About to generate instructions";
	// Called by RoutingLogic before it emits routeChanged()
	generate( RoutingLogic::instance()->nodes(), RoutingLogic::instance()->edges() );
}


// This method has been ported from descriptiongenerator.h
// void descriptions( QStringList* icons, QStringList* labels, QVector< IRouter::Node > pathNodes, QVector< IRouter::Edge > pathEdges, int maxSeconds = std::numeric_limits< int >::max() )
void InstructionGenerator::generate( QVector< IRouter::Node > pathNodes, QVector< IRouter::Edge > pathEdges )
{
	m_icons.clear();
	m_labels.clear();
	m_audioSnippets.clear();

	IRouter* router = MapData::instance()->router();
	if ( router == NULL || pathEdges.empty() || pathNodes.empty() ) {
		// *icons = QStringList();
		// *labels = QStringList();
		return;
	}

	newDescription( router, pathEdges.first() );
	int seconds = 0;

	int node = 0;
	GPSCoordinate gps = pathNodes.first().coordinate.ToGPSCoordinate();
	for ( int edge = 0; edge < pathEdges.size() - 1; edge++ ) {
		node += pathEdges[edge].length;
		GPSCoordinate nextGPS = pathNodes[node].coordinate.ToGPSCoordinate();
		m_distance += gps.ApproximateDistance( nextGPS );
		gps = nextGPS;
		m_branchingPossible = pathEdges[edge].branchingPossible;
		seconds += pathEdges[edge].seconds;

		if ( m_lastType == "roundabout" && pathEdges[edge + 1].type == m_lastTypeID ) {
			if ( m_branchingPossible )
				m_exitNumber++;
			continue;
		}

		int direction = angle( pathNodes[node - 1].coordinate, pathNodes[node].coordinate, pathNodes[node +  1].coordinate );
		bool breakDescription = false;

		QString type;
		bool typeAvailable = router->GetType( &type, pathEdges[edge + 1].type );
		assert( typeAvailable );

		if ( type == "motorway_link" && m_lastType != "motorway_link" ) {
			for ( int nextEdge = edge + 2; nextEdge < pathEdges.size(); nextEdge++ ) {
				if ( pathEdges[nextEdge].type != pathEdges[edge + 1].type ) {
					for ( int otherEdge = edge + 1; otherEdge < nextEdge; otherEdge++ )
						pathEdges[otherEdge].name = pathEdges[nextEdge].name;
					break;
				}
			}
		}

		if ( ( type == "roundabout" ) != ( m_lastType == "roundabout" ) ) {
			breakDescription = true;
			if ( type != "roundabout" )
				direction = 0;
		} else {
			if ( m_branchingPossible ) {
				if ( abs( direction ) > 1 )
					breakDescription = true;
			}
			if ( m_lastNameID != pathEdges[edge + 1].name )
				breakDescription = true;
		}

		if ( breakDescription ) {
			// describe( icons, labels);
			describe();
			// TODO: maxSeconds = std::numeric_limits< int >::max() once was a parameter of this function call which I didn't understand
			if ( seconds >= std::numeric_limits< int >::max() )
				break;
			newDescription( router, pathEdges[edge + 1] );
			m_direction = direction;
		}
	}
	GPSCoordinate nextGPS = pathNodes.back().coordinate.ToGPSCoordinate();
	m_distance += gps.ApproximateDistance( nextGPS );
	// TODO: maxSeconds = std::numeric_limits< int >::max() once was a parameter of this function call which I didn't understand
	if ( seconds < std::numeric_limits< int >::max() ){
		// describe( icons, labels );
		describe();
	}
}


// TODO: Rename this method which was ported from descriptiongenerator.h
void InstructionGenerator::newDescription( IRouter* router, const IRouter::Edge& edge )
{
	if ( m_lastNameID != edge.name ) {
		m_lastNameID = edge.name;
		bool nameAvailable = router->GetName( &m_lastName, m_lastNameID );
		assert( nameAvailable );
	}
	if ( m_lastTypeID != edge.type ) {
		m_lastTypeID = edge.type;
		bool typeAvailable = router->GetType( &m_lastType, m_lastTypeID );
		assert( typeAvailable );
	}
	m_branchingPossible = edge.branchingPossible;
	m_distance = 0;
	m_direction = 0;
	m_exitNumber = m_lastType == "roundabout" ? 1 : 0;
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


// void InstructionGenerator::describe( QStringList* icons, QStringList* labels )
void InstructionGenerator::describe()
{
	if ( m_exitNumber != 0 ) {
		m_icons.push_back( QString( ":/images/directions/roundabout.png" ) );
		m_labels.push_back( QString( "Enter the roundabout." ) );
		m_icons.push_back( QString( ":/images/directions/roundabout_exit%1.png" ).arg( m_exitNumber ) );
		m_labels.push_back( QString( "Take the %1. exit." ).arg( m_exitNumber ) );
		m_exitNumber = 0;
		return;
	}

	QString name = m_lastName;

		switch ( m_direction ) {
		case 0:
			break;
		case 1:
			{
				m_icons.push_back( ":/images/directions/slightly_right.png" );
				m_labels.push_back( "Keep slightly right" );
				break;
			}
		case 2:
			{
				m_icons.push_back( ":/images/directions/right.png" );
				m_labels.push_back( "Turn right" );
				break;
			}
		case 3:
			{
				m_icons.push_back( ":/images/directions/sharply_right.png" );
				m_labels.push_back( "Turn sharply right" );
				break;
			}
		case -1:
			{
				m_icons.push_back( ":/images/directions/slightly_left.png" );
				m_labels.push_back( "Keep slightly left" );
				break;
			}
		case -2:
			{
				m_icons.push_back( ":/images/directions/left.png" );
				m_labels.push_back( "Turn left" );
				break;
			}
		case -3:
			{
				m_icons.push_back( ":/images/directions/sharply_left.png" );
				m_labels.push_back( "Turn sharply left" );
				break;
			}
		}
		if ( m_direction != 0 ) {
			if ( !name.isEmpty() )
				m_labels.back() += " into " + name + ".";
			else
				m_labels.back() += ".";
		}

		if ( m_lastType == "motorway_link" ) {
			if ( m_direction == 0 ) {
				m_icons.push_back( ":/images/directions/forward.png" );
				m_labels.push_back( "" );
			}
			if ( !name.isEmpty() )
				m_labels.last() = "Take the ramp towards " + name + ".";
			else
				m_labels.last() = "Take the ramp.";
		}

	if ( m_distance > 20 ) {
		QString distance;
		if ( m_distance < 100 )
			distance = QString( "%1m" ).arg( ( int ) m_distance );
		else if ( m_distance < 1000 )
			distance = QString( "%1m" ).arg( ( int ) m_distance / 10 * 10 );
		else if ( m_distance < 10000 )
			distance = QString( "%1.%2km" ).arg( ( int ) m_distance / 1000 ).arg( ( ( int ) m_distance / 100 ) % 10 );
		else
			distance = QString( "%1km" ).arg( ( int ) m_distance / 1000 );

		m_icons.push_back( ":/images/directions/forward.png" );
		if ( !name.isEmpty() )
			m_labels.push_back( ( "Continue on " + name + " for " + distance + "." ) );
		else
			m_labels.push_back( ( "Continue for " + distance + "." ) );
	}
}
