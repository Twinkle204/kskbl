#include "TargetSpawner.h"
#include "Target.h"

#include <QRandomGenerator>

TargetSpawner::TargetSpawner(QObject *parent)
    : QObject(parent)
    , m_targetIdCounter(0)
    , m_spawnTimer(0.0f)
    , m_spawnInterval(1.5f)
    , m_maxTargets(5) {
}

TargetSpawner::~TargetSpawner() {
    qDeleteAll(m_activeTargets);
    m_activeTargets.clear();
}

void TargetSpawner::reset() {
    qDeleteAll(m_activeTargets);
    m_activeTargets.clear();
    m_targetIdCounter = 0;
    m_spawnTimer = 0.0f;
}

void TargetSpawner::update(float deltaTime) {
    m_spawnTimer += deltaTime;

    if (m_spawnTimer >= m_spawnInterval && m_activeTargets.size() < m_maxTargets) {
        Target *newTarget = spawnTarget();
        if (newTarget) {
            m_activeTargets.append(newTarget);
            emit targetSpawned(newTarget);
        }
        m_spawnTimer = 0.0f;
    }

    for (auto it = m_activeTargets.begin(); it != m_activeTargets.end(); ) {
        if ((*it)->isExpired() || (*it)->isHit()) {
            emit targetExpired(*it);
            delete *it;
            it = m_activeTargets.erase(it);
        } else {
            ++it;
        }
    }
}

Target* TargetSpawner::spawnTarget() {
    QVector3D position = getRandomSpawnPosition();
    TargetType type = getRandomTargetType();

    float radius = 1.0f;
    Target *target = new Target(m_targetIdCounter++, position, radius, type, this);
    target->setLifetime(3.0f + QRandomGenerator::global()->bounded(2.0f));

    return target;
}

QVector3D TargetSpawner::getRandomSpawnPosition() const {
    float rx = -18.0f + QRandomGenerator::global()->bounded(36.0f);
    float ry = -5.0f + QRandomGenerator::global()->bounded(10.0f);
    float rz = -50.0f + QRandomGenerator::global()->bounded(30.0f);
    return QVector3D(rx, ry, rz);
}

TargetType TargetSpawner::getRandomTargetType() const {
    int rand = QRandomGenerator::global()->bounded(100);

    if (rand < 40) {
        return TargetType::Static;
    } else if (rand < 60) {
        return TargetType::Linear;
    } else if (rand < 75) {
        return TargetType::Sine;
    } else if (rand < 90) {
        return TargetType::Circular;
    } else {
        return TargetType::RandomJump;
    }
}
