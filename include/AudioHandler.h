#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>


//定义了一个QObject的子类AudioHandeler，用来处理声音相关事项
class AudioHandler : public QObject {
    Q_OBJECT

public:
    explicit AudioHandler(QObject *parent = nullptr);
    ~AudioHandler();

    //击中音效
    void playHitSound();
    //未命中音效
    void playMissSound();
    //游玩时背景音乐
    void playBackgroundMusic();
    //暂停时背景音乐
    void stopBackgroundMusic();
    //静音
    void setMuted(bool muted);
    //判断是否静音
    bool isMuted() const { return m_muted; }
    //音量控制
    void setVolume(int volume);
    //返回音量数值
    int volume() const { return m_volume; }

private:
    //QMediaPlayer用于播放本地音频与视屏，QAudioOutput用于管理音频输出设备与音量通道
    //用三个QMediaPlayer，QAudioOutput分别来处理音频状态，避免互相干扰
    QMediaPlayer *m_hitPlayer;
    QAudioOutput *m_hitAudioOutput;

    QMediaPlayer *m_missPlayer;
    QAudioOutput *m_missAudioOutput;

    QMediaPlayer *m_bgMusicPlayer;
    QAudioOutput *m_bgAudioOutput;

    bool m_muted;
    int m_volume;
};
