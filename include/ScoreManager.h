#pragma once

#include <QObject>
#include <QList>
#include <QVariantMap>

struct HitRecord {
    float reactionTimeMs;
    float mouseDistance;
    float score;
    qint64 timestamp;
};

class ScoreManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)
    Q_PROPERTY(int hits READ hits NOTIFY hitsChanged)
    Q_PROPERTY(int misses READ misses NOTIFY missesChanged)
    Q_PROPERTY(float accuracy READ accuracy NOTIFY accuracyChanged)

public:
    explicit ScoreManager(QObject *parent = nullptr);
    ~ScoreManager();

    void reset();

    int recordHit(float reactionTimeMs = 0.0f, float mouseDistance = 0.0f);
    void recordMiss();

    int score() const { return m_score; }
    int hits() const { return m_hits; }
    int misses() const { return m_misses; }
    float accuracy() const;

    float averageReactionTime() const;
    float totalMouseDistance() const;
    int flickScore() const;
    int trackingScore() const;
    int clickingScore() const;

    QVariantMap getGameStats() const;

signals:
    void scoreChanged();
    void hitsChanged();
    void missesChanged();
    void accuracyChanged();
    void roundEnded();

private:
    int calculateHitScore(float reactionTimeMs, float mouseDistance);

    int m_score;
    int m_hits;
    int m_misses;
    QList<HitRecord> m_hitRecords;
    QList<float> m_reactionTimes;

    qint64 m_roundStartTime;
    float m_lastSpawnTime;
};
