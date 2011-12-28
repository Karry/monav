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

#ifndef ROUTINGLOGIC_H
#define ROUTINGLOGIC_H

#include "utils/coordinates.h"
#include "interfaces/irouter.h"
#include "instructiongenerator.h"

#include <QObject>
#include <QStringList>
#include <QVector>
#include <QDateTime>
#include <limits>

#ifndef NOQTMOBILE
#include <QGeoPositionInfoSource>
QTM_USE_NAMESPACE
#endif

class RoutingLogic : public QObject
{

	Q_OBJECT

public:

	struct GPSInfo {
		UnsignedCoordinate position;
		double altitude;
		double heading;
		double groundSpeed;
		double verticalSpeed;
		double horizontalAccuracy;
		double verticalAccuracy;
		QDateTime timestamp;
	};

	static RoutingLogic* instance();
	// the waypoints used to calculate the route, excluding the source
	QVector< UnsignedCoordinate > waypoints() const;
	UnsignedCoordinate source() const;
	// target == last waypoint
	UnsignedCoordinate target() const;
	// route description via its nodes' coordinates
	QVector< IRouter::Node > route() const;
	// returns the distance from source to target in meters
	double routeDistance();
	// returns the speed according to the information of the GPS subsystem
	double groundSpeed();
	// is the source linked to the GPS reciever?
	bool gpsLink() const;
	const GPSInfo gpsInfo() const;
	// clears the waypoints, target, and route
	void clear();
	QVector< IRouter::Edge >& edges();


signals:

	void routeChanged();
	void waypointsChanged();
	void gpsInfoChanged();
	void sourceChanged();

public slots:

	void setWaypoints( QVector< UnsignedCoordinate > waypoints );
	// If the coordinate is not valid the waypoint id is removed
	void setWaypoint( int id, UnsignedCoordinate coordinate );
	void setClickedSource( UnsignedCoordinate coordinate );
	// sets the target coordine == last waypoint. Inserts a waypoint if necessary.
	void setTarget( UnsignedCoordinate target );
	// Unlinks the source coordinate from the GPS subsystem, so clicking routes on the map becomes possible.
	void setGPSLink( bool linked );

	// destroys this object
	void cleanup();

protected:

	RoutingLogic();
	~RoutingLogic();
	UnsignedCoordinate coordOnSegment( int NodeId, UnsignedCoordinate newSource );
	void truncateRoute( int maxIndex );
	void computeRoute();
	void clearRoute();
	void setSource( UnsignedCoordinate coordinate );
	void calculateEdgeDistance( int index );

	struct PrivateImplementation;
	PrivateImplementation* const d;

protected slots:

	void dataLoaded();

#ifndef NOQTMOBILE
	void positionUpdated( const QGeoPositionInfo& update );
#endif
};

#endif // ROUTINGLOGIC_H
