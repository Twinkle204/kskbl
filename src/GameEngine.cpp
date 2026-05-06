#include "GameEngine.h"
#include "Target.h"
#include "TargetSpawner.h"

#include <QPainter>
#include <QtMath>

GameEngine::GameEngine(QObject *parent)
    : QObject(parent)
    , m_targetSpawner(new TargetSpawner(this))
    , m_yaw(0.0f)
    , m_pitch(0.0f)
    , m_sensitivity(0.2f)
    , m_cameraPos(0.0f, 2.0f, 5.0f)
    , m_fov(90)
    , m_screenWidth(1280)
    , m_screenHeight(720) {
}

GameEngine::~GameEngine() {
}

void GameEngine::reset() {
    m_yaw = 0.0f;
    m_pitch = 0.0f;
    m_cameraRotation = QQuaternion();
    m_targets.clear();
    m_targetSpawner->reset();
}

void GameEngine::update(float deltaTime) {
    m_targetSpawner->update(deltaTime);

    m_targets = m_targetSpawner->activeTargets();

    for (Target *target : m_targets) {
        target->update(deltaTime);
    }

    for (auto it = m_targets.begin(); it != m_targets.end(); ) {
        if ((*it)->isExpired()) {
            emit targetMissed(*it);
            delete *it;
            it = m_targets.erase(it);
        } else {
            ++it;
        }
    }
}

void GameEngine::render(QPainter &painter) {
    for (Target *target : m_targets) {
        target->render(painter, m_cameraPos, m_yaw, m_pitch,
                       m_screenWidth, m_screenHeight);
    }
}

void GameEngine::handleMouseMove(int dx, int dy) {
    m_yaw += dx * m_sensitivity;
    m_pitch += dy * m_sensitivity;

    m_pitch = qBound(-89.0f, m_pitch, 89.0f);

    if (m_yaw > 360.0f) m_yaw -= 360.0f;
    if (m_yaw < -360.0f) m_yaw += 360.0f;

    updateCamera();
}

void GameEngine::updateCamera() {
    float yawRad = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);

    float x = qCos(pitchRad) * qSin(yawRad);
    float y = qSin(pitchRad);
    float z = qCos(pitchRad) * qCos(yawRad);

    QVector3D forward(x, y, z);
    m_cameraRotation = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(QVector3D(0, 0, 1), forward.normalized()),
                                                       qDegreesToRadians(m_yaw));
}

bool GameEngine::handleClick(int screenX, int screenY) {
    QVector3D rayDir = screenToWorld(screenX, screenY, 20.0f);
    QVector3D rayOrigin = m_cameraPos;

    Target *closestHit = nullptr;
    float closestDist = 999999.0f;

    for (Target *target : m_targets) {
        if (target->isHit()) continue;

        QVector3D toTarget = target->position() - rayOrigin;
        float proj = QVector3D::dotProduct(toTarget, rayDir.normalized());

        if (proj < 0) continue;

        QVector3D closest = rayOrigin + rayDir.normalized() * proj;
        float dist = (closest - target->position()).length();

        if (dist <= target->radius()) {
            float actualDist = (target->position() - rayOrigin).length();
            if (actualDist < closestDist) {
                closestDist = actualDist;
                closestHit = target;
            }
        }
    }

    if (closestHit) {
        closestHit->setHit(true);
        emit targetHit(closestHit);
        return true;
    }

    return false;
}

QVector3D GameEngine::screenToWorld(int screenX, int screenY, float depth) const {
    float x = (2.0f * screenX / m_screenWidth) - 1.0f;
    float y = 1.0f - (2.0f * screenY / m_screenHeight);

    float aspect = (float)m_screenWidth / m_screenHeight;
    float fovRad = qDegreesToRadians((float)m_fov / 2.0f);

    float tanFov = qTan(fovRad);
    float dx = x * aspect * tanFov * depth;
    float dy = y * tanFov * depth;

    float yawRad = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);

    QVector3D dir(
        qCos(pitchRad) * qSin(yawRad),
        qSin(pitchRad),
        qCos(pitchRad) * qCos(yawRad)
    );

    return m_cameraPos + dir * depth;
}

QVector3D GameEngine::getCameraForward() const {
    float yawRad = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);

    return QVector3D(
        qCos(pitchRad) * qSin(yawRad),
        qSin(pitchRad),
        qCos(pitchRad) * qCos(yawRad)
    ).normalized();
}

QList<Target*> GameEngine::getActiveTargets() const {
    return m_targets;
}
