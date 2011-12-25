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
	double distance;
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
	d->distance = -1;
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


QVector< IRouter::Edge > RoutingLogic::edges()
{
	// TODO: Pass a const reference?
	return d->pathEdges;
}


QVector< IRouter::Node > RoutingLogic::route() const
{
	return d->pathNodes;
}


double RoutingLogic::routeDistance()
{
	// TODO: Change this, as the route meanwhile contains the edge's lengths
	double distance = 0.0;
	GPSCoordinate current;
	GPSCoordinate next;
	for( int i = 0; i < d->pathNodes.size() -1; i++ ){
		// TODO: Better use an iterator?
		current = d->pathNodes[i].coordinate.ToGPSCoordinate();
		next = d->pathNodes[i +1].coordinate.ToGPSCoordinate();
		distance += current.Distance( next );
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
	// TODO: Use vector maths to determine whether the position is still on track, and whether the position moves with or opposite to the route.
	d->source = coordinate;
	emit sourceChanged();

	if ( route().size() < 1 ){
		return;
	}

	double currentDistance = std::numeric_limits< double >::max();
	UnsignedCoordinate currentCoord;
	int nodeToKeep = -1;

	for ( int i = 0; i < route().size(); i++ ){
		currentCoord = route()[i].coordinate;
		currentDistance = currentCoord.ToGPSCoordinate().ApproximateDistance( coordinate.ToGPSCoordinate() );
		if ( currentDistance < 60 ){
			nodeToKeep = i;
			break;
		}
	}
	if ( nodeToKeep > -1 ){
		truncateRoute( nodeToKeep );
	}
	else{
		computeRoute();
	}
	emit routeChanged();
}



/*
Did not do the job due to coordOnSegment failing
void RoutingLogic::setSource( UnsignedCoordinate coordinate )
{
	if ( route().size() < 1 ){
		d->source = coordinate;
		emit sourceChanged();
		return;
	}
	qDebug() << "Source coords:" << coordinate.x << coordinate.y;

	double distToNearestSegment = std::numeric_limits< double >::max();
	double currentDistance = std::numeric_limits< double >::max();
	UnsignedCoordinate coordOnNearestSeg;
	UnsignedCoordinate currentCoord;
	int nodeToKeep = 0;

	for ( int i = 1; i <= d->pathNodes.size(); i++ ){
		currentCoord = coordOnSegment( i, coordinate );
		currentDistance = currentCoord.ToGPSCoordinate().ApproximateDistance( coordinate.ToGPSCoordinate() );
		qDebug() << currentDistance;
		if ( currentDistance <= distToNearestSegment ){
			distToNearestSegment = currentDistance;
			coordOnNearestSeg = currentCoord;
			nodeToKeep = i -1;
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
		emit routeChanged();
	}
	else if ( !oppositeHeading ){
		truncateRoute( nodeToKeep );
		if ( d->pathNodes.size() > 1 ){
			d->pathNodes[0] = coordOnNearestSeg;
		}
		d->source = coordOnNearestSeg;
		emit routeChanged();
	}

	emit sourceChanged();
	// qDebug() << "\n";
}
*/

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
		// qDebug() << "Routing:" << time.elapsed() << "ms";

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

	d->distance = waypoints.first().ToGPSCoordinate().ApproximateDistance( waypoints.last().ToGPSCoordinate() );
	InstructionGenerator::instance()->createInstructions( d->pathEdges, d->pathNodes );
	emit routeChanged();
}


void RoutingLogic::clearRoute()
{
	d->distance = -1;
	d->travelTime = -1;
	d->pathEdges.clear();
	d->pathNodes.clear();
	emit routeChanged();
}

/*
Did not work as expected
UnsignedCoordinate RoutingLogic::coordOnSegment( int NodeId, UnsignedCoordinate newSource )
{
	double ax = d->pathNodes[NodeId - 1].coordinate.x;
	double ay = d->pathNodes[NodeId - 1].coordinate.y;
	double bx = d->pathNodes[NodeId].coordinate.x;
	double by = d->pathNodes[NodeId].coordinate.y;
	double cx = newSource.x;
	double cy = newSource.y;


	// Example coords where the third one should have its nearest point on the line at the 2nd coord:
	// 562530383 367550754
	// 562531311 367552386
	// 562532175 367553922
	// Unfortunately they don't.

	// ax = 562530383;
	// ay = 367550754;
	// bx = 562531311;
	// by = 367552386;
	// cx = 562532175;
	// cy = 367553922;

	double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
	double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
	double r = r_numerator / r_denomenator;

	double px = ax + r*(bx-ax);
	double py = ay + r*(by-ay);
	double s =  ((ay-cy)*(bx-ax)-(ax-cx)*(by-ay) ) / r_denomenator;

	double distanceLine = fabs(s)*sqrt(r_denomenator);

	// (xx,yy) is the point on the lineSegment closest to (cx,cy)
	double xx = px;
	double yy = py;
	UnsignedCoordinate coordOnSegment;
	coordOnSegment.x = px;
	coordOnSegment.y = py;
	qDebug() << "\n";
	qDebug() << (int)ax << (int)ay;
	qDebug() << (int)bx << (int)by;
	qDebug() << (int)px << (int)py;
	qDebug() << "\n";
	double distanceSegment = 0;
	if ( (r >= 0) && (r <= 1) ){
		distanceSegment = distanceLine;
	}
	else{
		double dist1 = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
		double dist2 = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
		if (dist1 < dist2){
			xx = ax;
			yy = ay;
			distanceSegment = sqrt(dist1);
			}
		else{
			xx = bx;
			yy = by;
			distanceSegment = sqrt(dist2);
		}
	}

	return coordOnSegment;
}
*/

/*
UnsignedCoordinate RoutingLogic::coordOnSegment( int NodeId, UnsignedCoordinate newSource )
{
	// Vector magic adapted from
	// http://benc45.wordpress.com/2008/05/08/point-line-segment-distance/

	double ax = d->pathNodes[NodeId - 1].coordinate.x;
	double ay = d->pathNodes[NodeId - 1].coordinate.y;
	double bx = d->pathNodes[NodeId].coordinate.x;
	double by = d->pathNodes[NodeId].coordinate.y;
	// double cx = d->source.x;
	// double cy = d->source.y;
	double cx = newSource.x;
	double cy = newSource.y;

	double r_numerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
	double r_denomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
	double r = r_numerator / r_denomenator;
	double px = ax + r*(bx-ax);
	double py = ay + r*(by-ay);

	UnsignedCoordinate unsignedOnSeg;
	unsignedOnSeg.x = px;
	unsignedOnSeg.y = py;
	return unsignedOnSeg;
}
*/

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

