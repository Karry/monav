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
along with MoNav.  If not, see <http://www.gnu.org/licenses/>.
*/

// Also see tripinfodialog.cpp which contains the same value.
// Due to a circular dependency, it was not possible to provide a getter method.
#define INVALIDELEVATIONVALUE -20000.0

#include "logger.h"
#include "routinglogic.h"
#include "utils/qthelpers.h"
#include <QXmlStreamReader>

Logger* Logger::instance()
{
	static Logger logger;
	return &logger;
}


Logger::Logger( QObject* parent ) :
	QObject( parent )
{
	initialize();
	readGpxLog();
}


Logger::~Logger()
{
}

void Logger::cleanup()
{
	writeGpxLog();
}

void Logger::initialize()
{
	m_lastFlushTime = QDateTime::currentDateTime();

	QSettings settings( "MoNavClient" );
	m_loggingEnabled = settings.value( "LoggingEnabled", false ).toBool();
	m_tracklogPath = settings.value( "LogFilePath", QDir::homePath() ).toString();
	m_trackSegments.clear();
	m_segmentLenghts.clear();
	m_segmentMinElevations.clear();
	m_segmentMaxElevations.clear();
	m_trackSegments.append( QVector< RoutingLogic::GPSInfo >() );
	m_segmentLenghts.append( 0.0 );
	m_segmentMinElevations.append( INVALIDELEVATIONVALUE );
	m_segmentMaxElevations.append( INVALIDELEVATIONVALUE );
	m_maxSpeed = 0.0;

	m_tracklogPrefix = tr( "MoNavTrack" );
	QString tracklogFilename = m_tracklogPrefix;

	QDateTime currentDateTime = QDateTime::currentDateTime();
	tracklogFilename.append( currentDateTime.toString( "-yyyy-MM-dd" ) );
	tracklogFilename.append( ".gpx" );
	m_logFile.setFileName( fileInDirectory( m_tracklogPath, tracklogFilename ) );
}


QVector< int > Logger::polygonEndpointsTracklog()
{
	int endpoint = 0;
	QVector<int> endpoints;
	for( int i = 0; i < m_trackSegments.size(); i++ ){
		endpoint += m_trackSegments[i].size();
		endpoints.append( endpoint );
	}
	return endpoints;
}


// TODO: Why copy tons of data?!? This method possibly is called at each Gps-Update.
QVector< UnsignedCoordinate > Logger::polygonCoordsTracklog()
{
	QVector<UnsignedCoordinate> coordinates;
	for( int i = 0; i < m_trackSegments.size(); i++ ){
		for( int j = 0; j < m_trackSegments[i].size(); j++ ){
			coordinates.append( m_trackSegments[i][j].position );
		}
	}
	return coordinates;
}


void Logger::positionChanged()
{
	if ( !m_loggingEnabled )
		return;

	const RoutingLogic::GPSInfo& gpsInfo = RoutingLogic::instance()->gpsInfo();
	// TODO: Call by reference, not value?
	processGpsInfo( gpsInfo );

	int flushSecondsPassed = m_lastFlushTime.secsTo( QDateTime::currentDateTime() );
	if ( flushSecondsPassed >= 300 )
		writeGpxLog();
	emit trackChanged();
}


bool Logger::readGpxLog()
{
	if ( !m_logFile.open( QIODevice::ReadOnly | QIODevice::Text ) ){
		return false;
	}

	QXmlStreamReader gpxReader( &m_logFile );
	QXmlStreamAttributes currentAttributes;
	QXmlStreamAttribute currentAttribute;
	double lat = 200.0;
	double lon = 200.0;
	RoutingLogic::GPSInfo gpsInfo;
	gpsInfo.altitude = INVALIDELEVATIONVALUE;
	GPSCoordinate gpsCoordinate;

	enum FileStatus{ insideFile = 0, insideGpx = 1, insideTrack = 2, insideTracksegment = 3, insideTrackpoint = 4, insideWaypoint = 5 };
	FileStatus fileStatus = insideFile;

	// Reading the file "line by line"
	while ( !gpxReader.atEnd() )
	{
		gpxReader.readNext();
		if( gpxReader.hasError() )
		{
			qCritical() << tr( "Error while reading\n%1\n%2" ).arg( m_logFile.fileName() ).arg( gpxReader.errorString() );
			m_logFile.close();
			return false;
		}

		if ( gpxReader.isStartElement() && gpxReader.name().toString() == "trkseg" )
		{
			if( m_trackSegments[0].size() > 0 ){
				m_trackSegments.append( QVector< RoutingLogic::GPSInfo >() );
				m_segmentLenghts.append( 0.0 );
				m_segmentMinElevations.append( INVALIDELEVATIONVALUE );
				m_segmentMaxElevations.append( INVALIDELEVATIONVALUE );
			}
			fileStatus = insideTracksegment;
			continue;
		}
		else if ( gpxReader.isEndElement() && gpxReader.name().toString() == "trkseg" )
		{
			fileStatus = insideTrack;
			continue;
		}

		else if ( gpxReader.isStartElement() && gpxReader.name().toString() == "trkpt" )
		{
			fileStatus = insideTrackpoint;
			currentAttributes = gpxReader.attributes();
			for( int i = 0; i < currentAttributes.size(); i++ ){
				if( currentAttributes[i].name() == "lat" )
					lat = currentAttributes[i].value().toString().toDouble();
				if( currentAttributes[i].name() == "lon" )
					lon = currentAttributes[i].value().toString().toDouble();
			}
			gpsCoordinate.latitude = lat;
			gpsCoordinate.longitude = lon;
			gpsInfo.position = UnsignedCoordinate( gpsCoordinate );
			gpsInfo.horizontalAccuracy = 0;
			continue;
		}
		else if ( gpxReader.isStartElement() && fileStatus == insideTrackpoint && gpxReader.name().toString() == "ele")
		{
			gpsInfo.altitude = gpxReader.readElementText().toDouble();
			gpsInfo.verticalAccuracy = 0;
			continue;
		}
		else if ( gpxReader.isStartElement() && fileStatus == insideTrackpoint && gpxReader.name().toString() == "time")
		{
			// TODO: Currently when using a tracklog from a Garmin eTrex, the timestamp gets lost due to its format
			gpsInfo.timestamp = QDateTime::fromString( gpxReader.readElementText(), Qt::ISODate ); // 2011-05-07T15:37:42
			continue;
		}
		else if ( gpxReader.isEndElement() && gpxReader.name().toString() == "trkpt" )
		{
			fileStatus = insideTracksegment;
			processGpsInfo( gpsInfo );
			// Reinitialize the object to make it invalid.
			gpsInfo = RoutingLogic::GPSInfo();
			gpsInfo.altitude = INVALIDELEVATIONVALUE;
			continue;
		}
	}
	m_logFile.close();
	emit trackChanged();
	return true;
}


void Logger::processGpsInfo( RoutingLogic::GPSInfo gpsInfo )
{
	if( !gpsInfo.position.IsValid() ){
		return;
	}
	// As a tracklog does not provide accuracy, it gets set to 0 while reading the tracklog.
	// Indoor results: Horizontal accuracy: 652.64 , Vertical accuracy: 32767.5
	// Outdoor results: Horizontal accuracy: Between 27 and 37 , Vertical accuracy: Between 35 and 50
	if ( gpsInfo.horizontalAccuracy > 50 )
		return;
	if ( gpsInfo.verticalAccuracy > 50 )
		gpsInfo.altitude = INVALIDELEVATIONVALUE;

	QString checkValue = QString::number( gpsInfo.altitude );
	if( checkValue == "nan" || checkValue == "inf" )
		gpsInfo.altitude = INVALIDELEVATIONVALUE;

	// Filling cache variables
	if( m_trackSegments.last().size() > 0 ){
		double speed = 0.0;
		double distance = 0.0;
		QDateTime startTime = m_trackSegments.last().last().timestamp;
		QDateTime endTime = gpsInfo.timestamp;
		double seconds = 	startTime.secsTo( endTime );
		distance = gpsInfo.position.ToGPSCoordinate().Distance( m_trackSegments.last().last().position.ToGPSCoordinate() );
		m_segmentLenghts.last() += distance;
		if( seconds > 0 )
			speed = (distance * 36) / (seconds * 10);

		if( speed > m_maxSpeed ){
			m_maxSpeed = speed;
		}
	}

	// At this point, all elevations should have been set to something meaningful
	if( m_segmentMinElevations.last() == INVALIDELEVATIONVALUE || gpsInfo.altitude < m_segmentMinElevations.last() ){
		m_segmentMinElevations.last() = gpsInfo.altitude;
	}
	if( m_segmentMaxElevations.last() == INVALIDELEVATIONVALUE || gpsInfo.altitude > m_segmentMaxElevations.last() ){
		m_segmentMaxElevations.last() = gpsInfo.altitude;
	}
	m_trackSegments.last().append( gpsInfo );
}


bool Logger::writeGpxLog()
{
	QDateTime currentDateTime = QDateTime::currentDateTime();
	// Create a backup while writing to avoid data loss in case the battery drained etc.
	QString backupFilename;
	backupFilename = m_logFile.fileName().remove( m_logFile.fileName().size() -4, 4 );
	backupFilename.append( currentDateTime.toString( "-hhmmss" ) );
	backupFilename = backupFilename.append( tr( "-bck" ) );
	backupFilename = backupFilename.append( ".gpx" );
	// Always do this before opening the logfile, as the logfile got closed otherwise.
	m_logFile.copy( backupFilename );

	if ( !m_logFile.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) ){
		m_loggingEnabled = false;
		qCritical() << "Logger: Cannot write " << m_logFile.fileName() << ". Logging disabled.";
		return false;
	}

	QString trackName = m_tracklogPrefix;
	trackName.append( currentDateTime.toString( " yyyy-MM-dd" ) );

	enum FileStatus{ insideFile = 0, insideGpx = 1, insideTrack = 2, insideTracksegment = 3, insideTrackpoint = 4, insideWaypoint = 5 };
	FileStatus fileStatus = insideFile;

	QXmlStreamWriter gpxWriter(&m_logFile);
	gpxWriter.setAutoFormatting(true);
	gpxWriter.setAutoFormattingIndent( 2 );
	gpxWriter.writeStartDocument();
	gpxWriter.writeStartElement( "gpx" );
	gpxWriter.writeAttribute( "creator", "MoNav Tracklogger" );
	gpxWriter.writeAttribute( "version", "1.0" );
	gpxWriter.writeNamespace( "http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd","schemaLocation" );
	// fileStatus = insideGpx;
	gpxWriter.writeStartElement( "trk" );
	gpxWriter.writeTextElement( "name", trackName );
	fileStatus = insideTrack;

	QVector< RoutingLogic::GPSInfo > currentSegment;
	for( int h = 0; h < m_trackSegments.size(); h++ )
	{
		currentSegment = m_trackSegments[h];
		for( int i = 0; i < currentSegment.size(); i++ )
		{
			if( currentSegment[i].position.IsValid() && fileStatus == insideTrack ){
				gpxWriter.writeStartElement( "trkseg" );
				fileStatus = insideTracksegment;
			}
			if( currentSegment[i].position.IsValid() && fileStatus == insideTracksegment ){
				gpxWriter.writeStartElement( "trkpt" );
				gpxWriter.writeAttribute( "lat", QString::number( currentSegment[i].position.ToGPSCoordinate().latitude, 'f', 6 ) );
				gpxWriter.writeAttribute( "lon", QString::number( currentSegment[i].position.ToGPSCoordinate().longitude, 'f', 6 ) );
				if( currentSegment[i].altitude != INVALIDELEVATIONVALUE )
					gpxWriter.writeTextElement( "ele", QString::number( currentSegment[i].altitude, 'f', 0 ) );
				// TODO: See the reader method for problems with different timestamp formats. I recommend to write UTC.
				// http://www.routeconverter.de/forum/archive/index.php/thread-519.html
				if( currentSegment[i].timestamp.isValid() )
					gpxWriter.writeTextElement( "time", currentSegment[i].timestamp.toString( Qt::ISODate ) );
				gpxWriter.writeEndElement();
			}
			if( i == currentSegment.size()-1 && fileStatus == insideTracksegment ){
				gpxWriter.writeEndElement();
				fileStatus = insideTrack;
			}
		}
	}

	gpxWriter.writeEndDocument();
	m_logFile.close();
	// Remove the backup copy in case everything seems to be ok
	if( QFile::exists ( backupFilename ) )
		m_logFile.remove( backupFilename );
	m_lastFlushTime = QDateTime::currentDateTime();
	return true;
}


void Logger::clearTracklog()
{
	initialize();
	writeGpxLog();
	readGpxLog();
}


double Logger::trackDistance()
{
	double distance = 0.0;
	for( int i = 0; i < m_segmentLenghts.size(); i++ )
		distance += m_segmentLenghts[i];
	return distance;
}


double Logger::trackMinElevation()
{
	double elevation = INVALIDELEVATIONVALUE;
	for( int i = 0; i < m_segmentMinElevations.size(); i++ ){
		if( elevation == INVALIDELEVATIONVALUE || m_segmentMinElevations[i] < elevation )
			elevation = m_segmentMinElevations[i];
	}
	return elevation;
}


double Logger::trackMaxElevation()
{
	double elevation = INVALIDELEVATIONVALUE;
	for( int i = 0; i < m_segmentMaxElevations.size(); i++ ){
		if( m_segmentMaxElevations[i] > elevation )
			elevation = m_segmentMaxElevations[i];
	}
	return elevation;
}


double Logger::maxSpeed()
{
	return m_maxSpeed;
}


double Logger::averageSpeed()
{
	QDateTime startTime;
	QDateTime endTime;
	double seconds = 0;
	double distance = 0.0;
	double speed = 0.0;
	for( int i = 0; i < m_trackSegments.size(); i++ ){
		if( m_trackSegments[i].size() < 2 )
			continue;

		startTime = m_trackSegments[i].first().timestamp;
		endTime = m_trackSegments[i].last().timestamp;
		seconds = startTime.secsTo( endTime );
		distance = m_segmentLenghts[i];
		if( seconds > 0 ){
			speed += (distance * 36) / (seconds * 10);
		}
	}
	return speed / m_trackSegments.size();
}


QDateTime Logger::trackStartTime()
{
	QDateTime startTime;
	if( m_trackSegments[0].size() > 0 )
		startTime = m_trackSegments[0][0].timestamp;
	return startTime;
}


int Logger::trackDuration()
{
	QDateTime startTime;
	QDateTime endTime;
	int seconds = 0;

	for( int i = 0; i < m_trackSegments.size(); i++ ){
		if( m_trackSegments[i].size() < 1 )
			continue;

		startTime = m_trackSegments[i].first().timestamp;
		endTime = m_trackSegments[i].last().timestamp;
		seconds += startTime.secsTo( endTime );
	}
	return seconds;
}


const QVector< QVector< RoutingLogic::GPSInfo > >& Logger::currentTrack()
{
	return m_trackSegments;
}


bool Logger::loggingEnabled()
{
	return m_loggingEnabled;
}


void Logger::setLoggingEnabled(bool enable)
{
	// Avoid a new logfile is created in case nothing changed.
	if ( m_loggingEnabled == enable )
		return;
	QSettings settings( "MoNavClient" );
	settings.setValue( "LoggingEnabled", enable );
	writeGpxLog();
	initialize();
	readGpxLog();
}


QString Logger::directory()
{
	return m_tracklogPath;
}


void Logger::setDirectory( QString directory )
{
	// Avoid a new logfile is created in case nothing changed.
	if ( m_tracklogPath == directory )
		return;
	QSettings settings( "MoNavClient" );
	settings.setValue( "LogFilePath", directory );
	writeGpxLog();
	initialize();
	readGpxLog();
}

