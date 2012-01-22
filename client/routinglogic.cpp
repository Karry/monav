/*
Copyright 2010  Christian Vetter veaac.fdirct@gmail.com

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
along with MoNav.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "routinglogic.h"
#include "instructiongenerator.h"
#include "mapdata.h"
#include "utils/qthelpers.h"
#include "logger.h"
#ifndef NOQTMOBILE
#include "gpsdpositioninfosource.h"
#endif // NOQTMOBILE

#include <limits>
#include <algorithm>
#include <QtDebug>
#include <QSettings>
#include <QDir>

#include "audio.h"


struct RoutingLogic::PrivateImplementation {
	GPSInfo gpsInfo;
	UnsignedCoordinate source;
	QVector< UnsignedCoordinate > waypoints;
	QVector< IRouter::Node > pathNodes;
	QVector< IRouter::Edge > pathEdges;
	double travelTime;
	bool linked;
#ifndef NOQTMOBILE
	// the current GPS source
	QGeoPositionInfoSource* gpsSource;
#endif
};


RoutingLogic::RoutingLogic() :
		d( new PrivateImplementation )
{
	d->linked = true;
	d->travelTime = -1;

	d->gpsInfo.altitude = -1;
	d->gpsInfo.groundSpeed = -1;
	d->gpsInfo.verticalSpeed = -1;
	d->gpsInfo.heading = -1;
	d->gpsInfo.horizontalAccuracy = -1;
	d->gpsInfo.verticalAccuracy = -1;

	QSettings settings( "MoNavClient" );
	settings.beginGroup( "Routing" );
	d->source.x = settings.value( "source.x", d->source.x ).toUInt();
	d->source.y = settings.value( "source.y", d->source.y ).toUInt();

#ifndef NOQTMOBILE
	d->gpsSource = QGeoPositionInfoSource::createDefaultSource( this );
	if ( d->gpsSource == NULL )
		d->gpsSource = GpsdPositionInfoSource::create( this );
	if ( d->gpsSource == NULL ) {
		qDebug() << "No GPS Sensor found! GPS Updates are not available";
	} else {
		// prevent QtMobility from cheating
		// documentation states that it would provide updates as fast as possible if nothing was specified
		// nevertheless, it did provide only one every 5 seconds on the N900
		// with this setting on every second
		d->gpsSource->setUpdateInterval( 1000 );
		d->gpsSource->startUpdates();
		connect( d->gpsSource, SIGNAL(positionUpdated(QGeoPositionInfo)), this, SLOT(positionUpdated(QGeoPositionInfo)) );
	}
#endif

	for ( int i = 0; i < settings.value( "amountRoutepoints", 0 ).toInt(); i++ ) {
		UnsignedCoordinate coordinate;
		bool ok = true;
		coordinate.x = settings.value( QString( "routepointx%1" ).arg( i + 1 ), coordinate.x ).toUInt( &ok );
		if ( !ok )
			continue;
		coordinate.y = settings.value( QString( "routepointy%1" ).arg( i + 1 ), coordinate.y ).toUInt( &ok );
		if ( !ok )
			continue;
		if ( coordinate.IsValid() )
			d->waypoints.append( coordinate );
	}

	connect( this, SIGNAL(gpsInfoChanged()), Logger::instance(), SLOT(positionChanged()) );
	connect( MapData::instance(), SIGNAL(dataLoaded()), this, SLOT(dataLoaded()) );
	connect( this, SIGNAL(routeChanged()), InstructionGenerator::instance(), SLOT(requestSpeech()) );
	computeRoute();
	emit waypointsChanged();
}


RoutingLogic::~RoutingLogic()
{
	delete d;
}


void RoutingLogic::cleanup()
{
	disconnect();
#ifndef NOQTMOBILE
	if ( d->gpsSource != NULL )
		d->gpsSource->deleteLater();
#endif
	QSettings settings( "MoNavClient" );
	settings.beginGroup( "Routing" );
	settings.setValue( "source.x", d->source.x );
	settings.setValue( "source.y", d->source.y );
	settings.setValue( "amountRoutepoints", d->waypoints.size() );
	for ( int i = 0; i < d->waypoints.size(); i++ ){
		settings.setValue( QString( "routepointx%1" ).arg( i + 1 ), d->waypoints[i].x );
		settings.setValue( QString( "routepointy%1" ).arg( i + 1 ), d->waypoints[i].y );
	}
}


RoutingLogic* RoutingLogic::instance()
{
	static RoutingLogic routingLogic;
	return &routingLogic;
}


#ifndef NOQTMOBILE
void RoutingLogic::positionUpdated( const QGeoPositionInfo& update )
{
	if ( !update.isValid() )
		return;

	GPSCoordinate gps;
	gps.latitude = update.coordinate().latitude();
	gps.longitude = update.coordinate().longitude();
	d->gpsInfo.position = UnsignedCoordinate( gps );
	d->gpsInfo.altitude = update.coordinate().altitude();
	d->gpsInfo.timestamp = update.timestamp();
	if ( update.hasAttribute( QGeoPositionInfo::Direction ) )
		d->gpsInfo.heading = update.attribute( QGeoPositionInfo::Direction );
	if ( update.hasAttribute( QGeoPositionInfo::GroundSpeed ) )
		d->gpsInfo.groundSpeed = update.attribute( QGeoPositionInfo::GroundSpeed );
	if ( update.hasAttribute( QGeoPositionInfo::VerticalSpeed ) )
		d->gpsInfo.verticalSpeed = update.attribute( QGeoPositionInfo::VerticalSpeed );
	if ( update.hasAttribute( QGeoPositionInfo::HorizontalAccuracy ) )
		d->gpsInfo.horizontalAccuracy = update.attribute( QGeoPositionInfo::HorizontalAccuracy );
	if ( update.hasAttribute( QGeoPositionInfo::VerticalAccuracy ) )
		d->gpsInfo.verticalAccuracy = update.attribute( QGeoPositionInfo::VerticalAccuracy );
	emit gpsInfoChanged();

	if ( gpsLink() ){
		setSource( d->gpsInfo.position );
	}
}
#endif


QVector< UnsignedCoordinate > RoutingLogic::waypoints() const
{
	return d->waypoints;
}


UnsignedCoordinate RoutingLogic::source() const
{
	return d->source;
}


UnsignedCoordinate RoutingLogic::target() const
{
	if ( d->waypoints.empty() )
		return UnsignedCoordinate();
	return d->waypoints.last();
}


bool RoutingLogic::gpsLink() const
{
	return d->linked;
}


const RoutingLogic::GPSInfo RoutingLogic::gpsInfo() const
{
	return d->gpsInfo;
}


QVector< IRouter::Edge >& RoutingLogic::edges()
{
	QVector< IRouter::Edge >& edgeReference = d->pathEdges;
	return edgeReference;
}


QVector< IRouter::Node > RoutingLogic::route() const
{
	return d->pathNodes;
}


double RoutingLogic::routeDistance()
{
	double distance = 0.0;
	for( int i = 0; i < d->pathEdges.size(); i++ ){
		distance += d->pathEdges[i].distance;
	}
	return distance;
}


double RoutingLogic::groundSpeed()
{
	return d->gpsInfo.groundSpeed;
}


void RoutingLogic::clear()
{
	d->waypoints.clear();
	computeRoute();
}


void RoutingLogic::setWaypoints( QVector<UnsignedCoordinate> waypoints )
{
	bool changed = waypoints != d->waypoints;
	d->waypoints = waypoints;
	if ( changed )
	{
		computeRoute();
		emit waypointsChanged();
	}
}


void RoutingLogic::setWaypoint( int id, UnsignedCoordinate coordinate )
{
	if ( d->waypoints.size() <= id )
		d->waypoints.resize( id + 1 );
	d->waypoints[id] = coordinate;

	while ( !d->waypoints.empty() && !d->waypoints.back().IsValid() )
		d->waypoints.pop_back();

	computeRoute();
	emit waypointsChanged();
}


void RoutingLogic::setClickedSource( UnsignedCoordinate coordinate )
{
	setGPSLink( false );
	setSource( coordinate );
}


void RoutingLogic::setSource( UnsignedCoordinate coordinate )
{
	if ( route().size() < 1 ){
		d->source = coordinate;
		emit sourceChanged();
		return;
	}

	double distToNearestSegment = std::numeric_limits< double >::max();
	double currentDistance = std::numeric_limits< double >::max();
	UnsignedCoordinate coordOnNearestSeg;
	UnsignedCoordinate currentCoord;
	int nodeToKeep = 0;

	for ( int i = 0; i < d->pathNodes.size() -1; i++ ){
		currentCoord = coordOnSegment( i, coordinate );
		// coordOnSegment() already knows the distance, but it's computed based on unsigned coordinates
		currentDistance = currentCoord.ToGPSCoordinate().ApproximateDistance( coordinate.ToGPSCoordinate() );
		if ( currentDistance <= distToNearestSegment ){
			distToNearestSegment = currentDistance;
			coordOnNearestSeg = currentCoord;
			nodeToKeep = i;
		}
		// Assuming the segment nearest to the new source position was the former one
		else{
			break;
		}
	}

	bool sourceNearRoute = false;
	if ( distToNearestSegment < 30 ){
		sourceNearRoute = true;
	}

	// TODO: coordOnSegment() already computed this bool
	bool oppositeHeading = false;
	if ( coordOnNearestSeg.x == d->pathNodes[nodeToKeep].coordinate.x && coordOnNearestSeg.y == d->pathNodes[nodeToKeep].coordinate.y ){
		oppositeHeading = true;
	}

	if ( oppositeHeading && sourceNearRoute ){
		d->source = coordinate;
	}
	else if ( oppositeHeading && !sourceNearRoute ){
		d->source = coordinate;
		computeRoute();
	}
	else if ( !oppositeHeading ){
		truncateRoute( nodeToKeep );
		if ( d->pathNodes.size() > 1 ){
			d->pathNodes[0] = coordOnNearestSeg;
		}
		d->source = coordOnNearestSeg;
		calculateEdgeDistance( 0 );
		emit routeChanged();
	}

	emit sourceChanged();
}


void RoutingLogic::setTarget( UnsignedCoordinate target )
{
	int index = d->waypoints.empty() ? 0 : d->waypoints.size() - 1;
	setWaypoint( index, target );
}


void RoutingLogic::setGPSLink( bool linked )
{
	if ( linked == d->linked )
		return;
	d->linked = linked;
	if ( d->gpsInfo.position.IsValid() ) {
		// TODO: Better use setSource?
		d->source = d->gpsInfo.position;
		emit sourceChanged();
		computeRoute();
	}
}


void RoutingLogic::truncateRoute( int nodeToKeep )
{
	// Edge nodes always 1 less than nodes.
	for ( int i = 0; i < nodeToKeep; i++ ){
		d->pathEdges[0].length --;
		if ( d->pathEdges[0].length == 0 ){
			d->pathEdges.pop_front();
		}
		d->pathNodes.pop_front();
	}
	// TODO: Check whether the destination was reached.
}


void RoutingLogic::calculateEdgeDistance( int index ){

	assert( index < d->pathEdges.size() );

	int firstNodeIndex = 0;
	int lastNodeIndex = 0;

	for ( int i = 0; i <= index; i++ ){
		lastNodeIndex += d->pathEdges[i].length;
		firstNodeIndex = lastNodeIndex - d->pathEdges[i].length;
	}

	d->pathEdges[index].distance = 0.0;
	for ( int i = firstNodeIndex; i < lastNodeIndex; i++ ){
		d->pathEdges[index].distance += d->pathNodes[i].coordinate.ToGPSCoordinate().ApproximateDistance( d->pathNodes[i +1].coordinate.ToGPSCoordinate() );
	}
}


void RoutingLogic::computeRoute()
{
	IGPSLookup* gpsLookup = MapData::instance()->gpsLookup();
	if ( gpsLookup == NULL )
		return;
	IRouter* router = MapData::instance()->router();
	if ( router == NULL )
		return;

	if ( !d->source.IsValid() ) {
		clearRoute();
		return;
	}

	QVector< UnsignedCoordinate > waypoints;
	int passedRoutepoint = 0;

	waypoints.push_back( d->source );
	for ( int i = 0; i < d->waypoints.size(); i++ ) {
		if ( d->waypoints[i].IsValid() )
			waypoints.push_back( d->waypoints[i] );
		if ( waypoints[0].ToGPSCoordinate().ApproximateDistance( d->waypoints[i].ToGPSCoordinate() ) < 50 && i == 0 ) {
			waypoints.remove( 1, waypoints.size() - 1 );
			passedRoutepoint = i + 1;
		}
	}

	if ( passedRoutepoint > 0 )
	{
		d->waypoints.remove( 0, passedRoutepoint );
		emit waypointsChanged();
	}

	if ( waypoints.size() < 2 ) {
		clearRoute();
		return;
	}

	QVector< IGPSLookup::Result > gps;

	for ( int i = 0; i < waypoints.size(); i++ ) {
		Timer time;
		IGPSLookup::Result result;
		bool found = gpsLookup->GetNearestEdge( &result, waypoints[i], 1000 );
		// qDebug() << "GPS Lookup:" << time.elapsed() << "ms";

		if ( !found ) {
			clearRoute();
			return;
		}

		gps.push_back( result );
	}

	d->pathNodes.clear();
	d->pathEdges.clear();

	for ( int i = 1; i < waypoints.size(); i++ ) {
		QVector< IRouter::Node > nodes;
		QVector< IRouter::Edge > edges;
		double travelTime;

		Timer time;
		bool found = router->GetRoute( &travelTime, &nodes, &edges, gps[i - 1], gps[i] );
		// qDebug() << "Route recomputed in" << time.elapsed() << "ms";

		if ( found ) {
			if ( i == 1 ) {
				d->pathNodes = nodes;
				d->pathEdges = edges;
			} else {
				for ( int j = 1; j < nodes.size(); j++ )
					d->pathNodes.push_back( nodes[j] );
				for ( int j = 1; j < edges.size(); j++ )
					d->pathEdges.push_back( edges[j] );
			}
			d->travelTime += travelTime;
		} else {
			d->travelTime = -1;
			break;
		}
	}

	for ( int i = 0; i < d->pathEdges.size(); i++ ){
		calculateEdgeDistance( i );
	}

	InstructionGenerator::instance()->createInstructions( d->pathEdges, d->pathNodes );
	emit routeChanged();
}


void RoutingLogic::clearRoute()
{
	d->travelTime = -1;
	d->pathEdges.clear();
	d->pathNodes.clear();
	emit routeChanged();
}

UnsignedCoordinate RoutingLogic::coordOnSegment( int NodeId, UnsignedCoordinate newSource )
{
	double ax = d->pathNodes[NodeId].coordinate.x;
	double ay = d->pathNodes[NodeId].coordinate.y;
	double bx = d->pathNodes[NodeId +1].coordinate.x;
	double by = d->pathNodes[NodeId +1].coordinate.y;
	double px = newSource.x;
	double py = newSource.y;

	// http://vb-helper.com/howto_distance_point_to_line.html
	// Reused with kind permission of Rod Stephens to Christoph Eckert 2011-12-26. Thanks!
	double dx = bx - ax;
	double dy = by - ay;
	double near_x = 0;
	double near_y = 0;
	double DistToSegment = 0;
	if ( dx == 0 && dy == 0 ){
		// Line length == 0
		dx = px - ax;
		dy = py - ay;
		near_x = ax;
		near_y = ay;
		DistToSegment = sqrt(dx * dx + dy * dy);
	}
	else{
		// Calculate the t that minimizes the distance.
		double t = ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy);

		// Check whether this represents one of the segment's
		// end points or a point somewhere between both points.
		if ( t < 0 ){
			dx = px - ax;
			dy = py - ay;
			near_x = ax;
			near_y = ay;
		}
		else if( t > 1 ){
			dx = px - bx;
			dy = py - by;
			near_x = bx;
			near_y = by;
		}
		else{
			near_x = ax + t * dx;
			near_y = ay + t * dy;
			dx = px - near_x;
			dy = py - near_y;
		}
		DistToSegment = sqrt(dx * dx + dy * dy);
	}

	UnsignedCoordinate resultingCoord;
	resultingCoord.x = near_x;
	resultingCoord.y = near_y;
	return resultingCoord;
}


void RoutingLogic::dataLoaded()
{
	if ( !d->source.IsValid() )
	{
		const MapData::MapPackage& package = MapData::instance()->information();
		d->source.x = ( ( double ) package.max.x + package.min.x ) / 2;
		d->source.y = ( ( double ) package.max.y + package.min.y ) / 2;
		emit sourceChanged();
	}
	computeRoute();
}

