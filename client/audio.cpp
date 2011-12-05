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
	QAudioFormat format;
	// The files are RIFF (little-endian), WAVE audio, Microsoft PCM, Signed 16 bit, stereo 8000 Hz
	format.setFrequency(8000);
	format.setChannels(2);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	QAudioDeviceInfo deviceInfo(QAudioDeviceInfo::defaultOutputDevice());
	if (!deviceInfo.isFormatSupported(format))
	{
		qDebug()<< "raw audio format not supported by backend, cannot play audio.";
		qDebug()<< "\nDevice" << deviceInfo.deviceName() << "prefers:";
		qDebug()<<"Byte Order:" << deviceInfo.preferredFormat().byteOrder() << ", found" << format.byteOrder();
		qDebug()<<"Channel:" << deviceInfo.preferredFormat().channels() << ", found" << format.channels();
		qDebug()<<"Codec:" << deviceInfo.preferredFormat().codec() << ", found" << format.codec();
		qDebug()<<"Frequency:" << deviceInfo.preferredFormat().frequency() << ", found" << format.frequency();
		qDebug()<<"Sample Size:" << deviceInfo.preferredFormat().sampleSize() << ", found" << format.sampleSize();
		qDebug()<<"Sample Type:" << deviceInfo.preferredFormat().sampleType() << ", found" << format.sampleType();
		qDebug()<< "\nDevice" << deviceInfo.deviceName() << "supports:";
		qDebug()<<"Byte Orders:" << deviceInfo.supportedByteOrders();
		qDebug()<<"Channels:" << deviceInfo.supportedChannels();
		qDebug()<<"Codecs:" << deviceInfo.supportedCodecs();
		qDebug()<<"Frequencies:" << deviceInfo.supportedFrequencies();
		qDebug()<<"Sample Sizes:" << deviceInfo.supportedSampleSizes();
		qDebug()<<"Sample Types:" << deviceInfo.supportedSampleTypes();
	}

	m_audioOut = new QAudioOutput(format, this);
	connect(m_audioOut,SIGNAL(stateChanged(QAudio::State)),SLOT(finishedPlayback(QAudio::State)));
	connect(InstructionGenerator::instance(),SIGNAL(speechRequest(QString)),this,SLOT(speechRequest(QString)));
}

void Audio::speechRequest( QString fileName )
{
	// qDebug() << "Speakink";
	if ( m_audioFile.isOpen() ){
		qDebug() << "Sorry, I'm still busy playing audio.";
		return;
	}

	if ( fileName == "" ){
		qDebug() << "Audio filename empty - no audio playback.";
		return;
	}

	m_audioFile.setFileName( fileName );

	if ( !m_audioFile.open( QIODevice::ReadOnly ) )
	{
		qDebug() << "Cannot read audio data" << fileName << "so no audio playback.";
		return;
	}
	qDebug() << fileName;
	m_audioOut->start( &m_audioFile );
}


void Audio::finishedPlayback( QAudio::State state )
{
	if( state == QAudio::IdleState )
	{
		m_audioOut->stop();
		m_audioFile.close();
	}
}

