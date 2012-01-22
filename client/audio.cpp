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

#include "audio.h"
#include "instructiongenerator.h"

Audio::Audio( QObject* parent ) :
	QObject( parent )
{
	initialize();
}

Audio::~Audio()
{
	delete m_audioOut;
}

Audio* Audio::instance()
{
	static Audio audio;
	return &audio;
}

void Audio::initialize()
{
	// The files are RIFF (little-endian), WAVE audio, Microsoft PCM, Signed 16 bit, stereo 8000 Hz
	m_format.setFrequency(8000);
	m_format.setChannels(2);
	m_format.setSampleSize(16);
	m_format.setCodec("audio/pcm");
	m_format.setByteOrder(QAudioFormat::LittleEndian);
	m_format.setSampleType(QAudioFormat::SignedInt);

	QAudioDeviceInfo deviceInfo( QAudioDeviceInfo::defaultOutputDevice() );
	if ( !deviceInfo.isFormatSupported( m_format ) )
	{
		qDebug()<< "raw audio format not supported by backend, cannot play audio.";
		qDebug()<< "\nDevice" << deviceInfo.deviceName() << "prefers:";
		qDebug()<<"Byte Order:" << deviceInfo.preferredFormat().byteOrder() << ", found" << m_format.byteOrder();
		qDebug()<<"Channel:" << deviceInfo.preferredFormat().channels() << ", found" << m_format.channels();
		qDebug()<<"Codec:" << deviceInfo.preferredFormat().codec() << ", found" << m_format.codec();
		qDebug()<<"Frequency:" << deviceInfo.preferredFormat().frequency() << ", found" << m_format.frequency();
		qDebug()<<"Sample Size:" << deviceInfo.preferredFormat().sampleSize() << ", found" << m_format.sampleSize();
		qDebug()<<"Sample Type:" << deviceInfo.preferredFormat().sampleType() << ", found" << m_format.sampleType();
		qDebug()<< "\nDevice" << deviceInfo.deviceName() << "supports:";
		qDebug()<<"Byte Orders:" << deviceInfo.supportedByteOrders();
		qDebug()<<"Channels:" << deviceInfo.supportedChannels();
		qDebug()<<"Codecs:" << deviceInfo.supportedCodecs();
		qDebug()<<"Frequencies:" << deviceInfo.supportedFrequencies();
		qDebug()<<"Sample Sizes:" << deviceInfo.supportedSampleSizes();
		qDebug()<<"Sample Types:" << deviceInfo.supportedSampleTypes();
	}
	m_audioOut = new QAudioOutput( m_format, this );
	connect(m_audioOut,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));
}

void Audio::speak( QStringList fileNames )
{
	m_audioFilenames.append( fileNames );
	process();
}


void Audio::process()
{
	qDebug() << "Start processing files:" << m_audioFile.fileName();
	if ( m_audioFilenames.size() < 1 ){
		return;
	}

	if ( m_audioFile.isOpen() ){
		return;
	}

	m_audioFile.setFileName( m_audioFilenames[0] );
	m_audioFilenames.pop_front();

	if ( !m_audioFile.open( QIODevice::ReadOnly ) ){
		return;
	}

	if ( m_audioOut->state() == QAudio::StoppedState ){
		m_audioOut->start( &m_audioFile );
	}
	else if ( m_audioOut->state() == QAudio::SuspendedState ){
		m_audioOut->resume();
	}
	qDebug() << "End processing files.\n----\n\n";
}


void Audio::stateChanged( QAudio::State state )
{
	if ( state == QAudio::IdleState ){
		m_audioFile.close();
		if ( m_audioFilenames.size() < 1 ){
			m_audioOut->stop();
		}
		else {
			process();
		}
	}
}

