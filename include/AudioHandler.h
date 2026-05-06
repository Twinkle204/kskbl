#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>

class AudioHandler : public QObject {
    Q_OBJECT

public:
    explicit AudioHandler(QObject *parent = nullptr);
    ~AudioHandler();

    void playHitSound();
    void playMissSound();
    void playBackgroundMusic();
    void stopBackgroundMusic();

    void setMuted(bool muted);
    bool isMuted() const { return m_muted; }

    void setVolume(int volume);
    int volume() const { return m_volume; }

private:
    QMediaPlayer *m_hitPlayer;
    QAudioOutput *m_hitAudioOutput;

    QMediaPlayer *m_missPlayer;
    QAudioOutput *m_missAudioOutput;

    QMediaPlayer *m_bgMusicPlayer;
    QAudioOutput *m_bgAudioOutput;

    bool m_muted;
    int m_volume;
};
