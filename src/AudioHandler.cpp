#include "AudioHandler.h"

AudioHandler::AudioHandler(QObject *parent)
    : QObject(parent)
    , m_hitPlayer(new QMediaPlayer(this))
    , m_hitAudioOutput(new QAudioOutput(this))
    , m_missPlayer(new QMediaPlayer(this))
    , m_missAudioOutput(new QAudioOutput(this))
    , m_bgMusicPlayer(new QMediaPlayer(this))
    , m_bgAudioOutput(new QAudioOutput(this))
    , m_muted(false)
    , m_volume(80) {

    m_hitPlayer->setAudioOutput(m_hitAudioOutput);
    m_missPlayer->setAudioOutput(m_missAudioOutput);
    m_bgMusicPlayer->setAudioOutput(m_bgAudioOutput);

    m_hitAudioOutput->setVolume(m_volume / 100.0f);
    m_missAudioOutput->setVolume(m_volume / 100.0f);
    m_bgAudioOutput->setVolume(m_volume / 100.0f * 0.3f);

    m_bgMusicPlayer->setLoops(QMediaPlayer::Infinite);
}

AudioHandler::~AudioHandler() {
}

void AudioHandler::playHitSound() {
    if (m_muted) return;
}

void AudioHandler::playMissSound() {
    if (m_muted) return;
}

void AudioHandler::playBackgroundMusic() {
    if (m_muted) return;
    m_bgMusicPlayer->play();
}

void AudioHandler::stopBackgroundMusic() {
    m_bgMusicPlayer->stop();
}

void AudioHandler::setMuted(bool muted) {
    m_muted = muted;
    if (muted) {
        m_hitPlayer->stop();
        m_missPlayer->stop();
        m_bgMusicPlayer->stop();
    }
}

void AudioHandler::setVolume(int volume) {
    m_volume = qBound(0, volume, 100);
    m_hitAudioOutput->setVolume(m_volume / 100.0f);
    m_missAudioOutput->setVolume(m_volume / 100.0f);
    m_bgAudioOutput->setVolume(m_volume / 100.0f * 0.3f);
}
