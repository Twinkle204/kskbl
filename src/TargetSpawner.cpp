#include "TargetSpawner.h"
#include "Target.h"

#include <QRandomGenerator>
#include <QtMath>

TargetSpawner::TargetSpawner(QObject *parent)
    : QObject(parent)
    , m_targetIdCounter(0)
    , m_spawnTimer(0.0f)
    , m_spawnInterval(1.5f)
    , m_maxTargets(5)
    , m_spawnRangeMin(5.0f)
    , m_spawnRangeMax(25.0f) {
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

    float radius = 0.5f;
    if (type == TargetType::RandomJump) {
        radius = 0.8f;
    } else if (type == TargetType::Circular) {
        radius = 0.6f;
    }

    Target *target = new Target(m_targetIdCounter++, position, radius, type, this);
    target->setLifetime(3.0f + QRandomGenerator::global()->bounded(2.0f));

    return target;
}

QVector3D TargetSpawner::getRandomSpawnPosition() const {
    float distance = m_spawnRangeMin + QRandomGenerator::global()->bounded(m_spawnRangeMax - m_spawnRangeMin);
    float angle = QRandomGenerator::global()->bounded(360.0f);
    float height = -5.0f + QRandomGenerator::global()->bounded(10.0f);

    float x = distance * qCos(qDegreesToRadians(angle));
    float y = height;
    float z = distance * qSin(qDegreesToRadians(angle));

    return QVector3D(x, y, z);
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
