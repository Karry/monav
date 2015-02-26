#ifndef SERVERLOGIC_H
#define SERVERLOGIC_H

#include <QString>
#include <QList>
#include <QUrl>
#include <QDomDocument>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class DirectoryUnpacker;
class QProgressDialog;

class ServerLogic : public QObject
{
	Q_OBJECT

	public:

		struct Server
		{
			QString name;
			QUrl url;
		};

		struct PackageInfo
		{
			QString path;
			QString server;
			QString dir;
			qint64 size;
			unsigned timestamp;

			bool operator==( const PackageInfo& other ) const
			{
				return this->path == other.path;
			}

			bool operator<( const PackageInfo& other ) const
			{
				return this->server < other.server;
			}

		};

                enum ERROR_TYPE { UNDEFINED_ERROR, LIST_DL_ERROR, PACKAGE_DL_ERROR };
		enum OPERATION { INACTIVE, LIST_DL, PACKAGE_CHK, PACKAGE_DL };

		ServerLogic();
		~ServerLogic();

		void setOp( ServerLogic::OPERATION operation );
		const ServerLogic::OPERATION& getOp();

		void clearPackagesToLoad();
		void addPackagesToLoad( const QList< PackageInfo >& packageLocations );
		void removePackagesToLoad( const QList< ServerLogic::PackageInfo > &packageLocations );
		const QList< PackageInfo >& packagesToLoad() const;

		const QDomDocument& packageList() const;

	signals:

		void loadedList();
		void checkedPackage( QString info );
		void loadedPackage( QString info );
		void error( ServerLogic::ERROR_TYPE type = UNDEFINED_ERROR, QString message = "" );

	public slots:

		void connectNetworkManager();
		bool loadPackage();
		bool checkPackage();
		void loadPackageList( const QUrl &url );

	protected slots:

		void finished( QNetworkReply* reply );
		void cleanUp( ServerLogic::ERROR_TYPE type = UNDEFINED_ERROR, QString message = "" );

	protected:

		QString m_localDir;
		QDomDocument m_packageList;
		OPERATION m_currentOp;
		DirectoryUnpacker *m_unpacker;
		QNetworkAccessManager* m_network;

		QList< PackageInfo > m_packagesToLoad;
		int m_packageIndex;

		QDomElement findElement( QString packageType, QString packageName, QString mapName = "" );
};

#endif // SERVERLOGIC_H
