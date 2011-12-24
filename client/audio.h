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
along with MoNav.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AUDIO_H
#define AUDIO_H

#include <iostream>
#include <QObject>
#include <QDebug>
#include <QFile>
#include <QtMultimedia>
#include "routinglogic.h"

class Audio : public QObject
{

Q_OBJECT

public:

	static Audio* instance();
	~Audio();
	void speak( QString fileName );

public slots:

	void initialize();
	void finishedPlayback( QAudio::State );

// signals:

protected:

	explicit Audio( QObject* parent = 0 );
	QFile m_audioFile;
	QAudioOutput* m_audioOut;
};

#endif // AUDIO_H
