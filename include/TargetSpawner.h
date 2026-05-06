#pragma once

#include <QObject>
#include <QTimer>
#include <QList>
#include <QVector3D>
#include "Target.h"

class TargetSpawner : public QObject {
    Q_OBJECT

public:
    explicit TargetSpawner(QObject *parent = nullptr);
    ~TargetSpawner();

    void reset();
    void update(float deltaTime);

    QList<Target*> activeTargets() const { return m_activeTargets; }
    void setSpawnInterval(float interval) { m_spawnInterval = interval; }
    float spawnInterval() const { return m_spawnInterval; }
    void setMaxTargets(int max) { m_maxTargets = max; }
    int maxTargets() const { return m_maxTargets; }

    int targetIdCounter() const { return m_targetIdCounter; }

signals:
    void targetSpawned(Target *target);
    void targetExpired(Target *target);

private:
    Target* spawnTarget();
    QVector3D getRandomSpawnPosition() const;
    TargetType getRandomTargetType() const;

    QList<Target*> m_activeTargets;
    int m_targetIdCounter;
    float m_spawnTimer;
    float m_spawnInterval;
    int m_maxTargets;
    float m_spawnRangeMin;
    float m_spawnRangeMax;
};
