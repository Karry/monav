#include <stdio.h>

#include <QStringList>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QtXml/QDomDocument>

const int VERSION = 2;

enum OPERATION { CREATE_LIST, DELETE_LIST, ADD_PACKAGE, DELETE_PACKAGE, AUTOMATIC, DEFAULT };

QDomElement findPackageElement( const QDomDocument& list, QString type, QString name, QString map = "" );
QString findMapInDir( const QString &path );

bool createList( QDomDocument* list, QFile* file, QString serverPath );
bool addPackage( QDomDocument* list, QString path );
bool deletePackage( QDomDocument *list, QString path );
void addMap( QDomDocument* list, QString name, QString path );

int main( int argc, char *argv[] )
{
	QString serverPath = "";
	QString packagePath = "";
	OPERATION listOperation = DEFAULT;

	int c;
	while ( ( c = getopt( argc, argv, "c:r:a:p:d" ) ) != -1 )
	{
		switch(c)
		{
			case 'c':
				listOperation = CREATE_LIST;
				serverPath = optarg;
				break;
			case 'd':
				listOperation = DELETE_LIST;
				break;
			case 'a':
				listOperation = ADD_PACKAGE;
				packagePath = optarg;
				break;
			case 'r':
				listOperation = DELETE_PACKAGE;
				packagePath = optarg;
				break;
			case 'p':
				listOperation = AUTOMATIC;
				serverPath = optarg;
				break;
			default:
				printf( "operation not recognized\n" );
				return 0;
		}
	}

	if( argc < 2 || ( argc != 3 && listOperation != DELETE_LIST ) )
	{
		printf( "usage:\n" );
		printf( "-p server_url: create list automatically by recursively parsing mapmanager directory\n" );
		printf( "-c server_url : create list\n" );
		printf( "-d server_url : delete list\n" );
		printf( "-a package_path_and_name : add package to list\n" );
		printf( "-r package_path_and_name : delete package from list\n" );

		return 0;
	}

	if( !serverPath.endsWith( "/" ) )
		serverPath.append( "/" );

	if( packagePath.startsWith( "." ) )
		packagePath.remove( 0, 1 );

	if( packagePath.startsWith( "/" ) )
		packagePath.remove( 0, 1 );

	QString packageName = packagePath;
	packageName.remove( 0, packageName.lastIndexOf( '/' ) + 1 );
	packageName.truncate( packageName.lastIndexOf( '.' ) );

	QDomDocument list;
	QFile listFile( "packageList.xml" );

	if( listOperation == CREATE_LIST )
		return createList( &list, &listFile, serverPath );

	if( listOperation == AUTOMATIC )
	{
		if( !createList( &list, &listFile, serverPath ) )
			return 0;

		QDir dir = QDir();
		QStringList dirList = QStringList( dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name ) );

		for( int i=0; i < dirList.size(); i++)
		{
			dir.setPath( dirList[i] );
			QStringList subDirs = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );

			for( int j=0; j < subDirs.size(); j++ )
				dirList.append( dir.path() + '/' + subDirs[j] );

			QString mapName = findMapInDir( dir.path() + '/' + "Monav.ini" );

			if( !mapName.isEmpty() )
				addPackage( &list, dir.path() + '/' + "MoNav.ini" );
//			else
//				mapName = dir.dirName();

			QStringList modules = dir.entryList( QDir::Files, QDir::Type ).filter( ".mmm" );

			for( int j=0; j < modules.size(); j++ )
				addPackage( &list, dir.path() + '/' + modules[j] );
		}
	}

	else
	{
		if ( !listFile.exists() )
		{
			printf( "create list file first using -c or -p\n" );
			return 0;
		}

		if( listOperation == DELETE_LIST )
		{
			if ( !listFile.remove() )
			{
				printf( "error deleting list file\n" );
				return 0;
			}

			printf( "deleted list\n" );
			return 1;
		}


		if ( !listFile.open( QIODevice::ReadOnly ) )
		{
			printf( "failed to open list file\n" );
			return 0;
		}

		if ( !list.setContent( &listFile ) )
		{
			printf("error parsing list file\n");
			listFile.close();
			return 0;
		}
		listFile.close();
	}

	if( listOperation == DELETE_PACKAGE )
	{
		if ( !deletePackage( &list, packagePath ) )
			return 0;
	}

	if( listOperation == ADD_PACKAGE )
	{
		if ( !addPackage( &list, packagePath ) )
			return 0;
	}

	if (!listFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) return 1;
	listFile.write(list.toString().toUtf8());
	listFile.close();

	return 1;
}

QDomElement findPackageElement(
	const QDomDocument &list, QString packageType, QString packageName, QString mapName )
{
	QDomNodeList packageNodeList = list.elementsByTagName( packageType );

	for( unsigned int i=0; i < packageNodeList.length(); i++ )
	{
		QDomElement package = packageNodeList.at( i ).toElement();

		if( packageType == "map" && package.attribute( "name" ) == packageName )
				return package;

		if( packageType == "module" && package.attribute( "name" ) == packageName )
		{
			if( package.parentNode().parentNode().toElement().attribute( "name" ) == mapName )
				return package;
		}
	}

	return QDomElement();
}


QString findMapInDir( const QString& path )
{
	QString mapPath = path.left( path.lastIndexOf( '/' ) + 1 ) + "MoNav.ini";

	if( !QFile::exists( mapPath ) )
		return "";

	QSettings mapFile( mapPath, QSettings::IniFormat );

	return mapFile.value( "name", "" ).toString();
}

bool createList( QDomDocument* list, QFile* listFile, QString serverPath )
{
	if ( listFile->exists() )
	{
		printf( "a previous list file exists on the server, delete it first using -r server_path\n" );
		return false;
	}

	QDomElement rootElement = list->createElement( "root" );
	rootElement.setAttribute( "path", serverPath );
	rootElement.setAttribute( "timestamp", 0 );
	rootElement.setAttribute( "version", VERSION );
	list->appendChild(rootElement);

	if ( !listFile->open( QIODevice::ReadWrite | QIODevice::Truncate ) )
	{
		printf( "failed to open list file\n" );
		return false;
	}
	listFile->write( list->toString().toUtf8() );
	listFile->close();

	printf( "created list\n" );
	return true;
}


void addMap( QDomDocument* list, QString name, QString path )
{
	int timestamp = list->documentElement().attribute( "timestamp" ).toInt() + 1;
	list->documentElement().setAttribute( "timestamp", timestamp );

	QDomElement mapElement = list->createElement( "map" );
	mapElement.setAttribute( "timestamp", timestamp );
	mapElement.setAttribute( "size", QFile( path ).size() );
	mapElement.setAttribute( "path", path );
	mapElement.setAttribute( "name", name );
	list->documentElement().appendChild( mapElement );

	printf( "added map entry: "  + name.toUtf8() + " from " + path.toUtf8() + "\n" );
}


bool addPackage( QDomDocument* list, QString path )
{
	QString map = findMapInDir( path );

	if( map.isEmpty() )
	{
		printf( "no map file in dir (required for parsing)\n" );
		printf( "package path: " + path.toUtf8() + "\n" );
		return false;
	}

	QDomElement mapElement = findPackageElement( *list, "map", map );

	if( path.endsWith( "MoNav.ini" ) )
	{
		if( !mapElement.isNull() )
		{
			printf( "map already in list\n" );
			return false;
		}

		addMap( list, map, path );
	}

	else if( path.endsWith( ".mmm" ) )
	{
		if( mapElement.isNull() )
			addMap( list, map, path );

		QString package = path.right( path.size() - path.lastIndexOf( '/' ) - 1 );
		QStringList packageAttributes = package.split( "_" );
		QString type = packageAttributes[0];
		QString name = packageAttributes[1].replace( ".mmm", "" );

		if( !findPackageElement( *list, type, name, map ).isNull() )
		{
			printf( "package already in list\n" );
			return false;
		}

		int timestamp = list->documentElement().attribute( "timestamp" ).toInt() + 1;
		list->documentElement().setAttribute( "timestamp", timestamp );
		mapElement.setAttribute( "timestamp", timestamp );

		QDomElement typeElement = mapElement.firstChildElement( type );
		if( typeElement.isNull() )
		{
			typeElement = list->createElement( type );
			mapElement.appendChild(typeElement);
		}
		typeElement.setAttribute( "timestamp", timestamp );

		QDomElement moduleElement = list->createElement( "module" );
		moduleElement.setAttribute( "timestamp", timestamp );
		moduleElement.setAttribute( "size", QFile( path ).size() );
		moduleElement.setAttribute( "name" , name );
		typeElement.appendChild( moduleElement );

		QDomText pathNode = list->createTextNode( path );
		moduleElement.appendChild(pathNode);

		printf( "added module entry: " + path.toUtf8() + "\n" );
	}

	else
	{
		printf( "unrecognized package format\n" );
		return false;
	}

	return true;
}

bool deletePackage( QDomDocument *list, QString path )
{
	QString map = findMapInDir( path );

	if( path.endsWith( "MoNav.ini" ) )
	{
		QDomElement mapElement = findPackageElement( *list, "map", map );

		if( mapElement.isNull() )
		{
			printf( "map not in list\n" );
			return false;
		}

		list->documentElement().removeChild( mapElement );
		printf( "deleted map entry\n" );
	}

	else if( path.endsWith( ".mmm" ) )
	{
		QString name = path.split("_")[1];

		QDomElement moduleElement = findPackageElement( *list, "module", name, map );

		if( moduleElement.isNull() )
		{
			printf("module not in list\n");
			return 1;
		}

		QDomElement parentElement = moduleElement.parentNode().toElement();
		parentElement.removeChild( moduleElement );

		while( !parentElement.hasChildNodes() )
		{
			moduleElement = parentElement;
			parentElement = parentElement.parentNode().toElement();
			parentElement.removeChild( moduleElement );
		}

		printf( "deleted module entry\n" );
	}

	else
	{
		printf( "unrecognized package format\n" );
		return false;
	}

	return true;
}
