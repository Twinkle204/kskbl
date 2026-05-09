#include "GameEngine.h"
#include "Target.h"
#include "TargetSpawner.h"
#include "RoomRenderer.h"

#include <QPainter>
#include <QtMath>

GameEngine::GameEngine(QObject *parent)
    : QObject(parent)
    , m_targetSpawner(new TargetSpawner(this))
    , m_roomRenderer(new RoomRenderer())
    , m_sensitivity(0.15f)
    , m_screenWidth(1280)
    , m_screenHeight(720)
    , m_yaw(0.0f)
    , m_pitch(0.0f)
    , m_cameraPos(0.0f, 0.0f, 10.0f) {
}

GameEngine::~GameEngine() {
}

void GameEngine::reset() {
    m_targets.clear();
    m_targetSpawner->reset();
    m_yaw = 0.0f;
    m_pitch = 0.0f;
    m_cameraPos = QVector3D(0.0f, 0.0f, 10.0f);
}

void GameEngine::update(float deltaTime) {
    m_targetSpawner->update(deltaTime);

    QMutexLocker locker(&m_targetsMutex);
    m_targets = m_targetSpawner->activeTargets();

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
    QMutexLocker locker(&m_targetsMutex);
    const auto targetsCopy = m_targets;

    for (Target *target : targetsCopy) {
        target->render(painter,
                       m_cameraPos,
                       m_yaw,
                       m_pitch,
                       m_screenWidth,
                       m_screenHeight);
    }
}

void GameEngine::renderBackground(QPainter &painter) {
    m_roomRenderer->setCamera(m_yaw, m_pitch, 90.0f,
                              m_screenWidth, m_screenHeight);
    m_roomRenderer->setCameraPos(m_cameraPos);
    m_roomRenderer->render(painter, m_screenWidth, m_screenHeight);
}

void GameEngine::handleMouseMove(float dx, float dy) {
    m_yaw += dx * m_sensitivity;
    m_pitch += dy * m_sensitivity;

    const float PITCH_LIMIT = 80.0f;
    if (m_pitch > PITCH_LIMIT)  m_pitch = PITCH_LIMIT;
    if (m_pitch < -PITCH_LIMIT) m_pitch = -PITCH_LIMIT;

    while (m_yaw >= 360.0f)  m_yaw -= 360.0f;
    while (m_yaw < 0.0f)     m_yaw += 360.0f;
}

bool GameEngine::handleClickAt(int screenX, int screenY) {
    QMutexLocker locker(&m_targetsMutex);
    const auto targetsCopy = m_targets;

    QVector3D ray = screenToRay(screenX, screenY);

    Target *closestHit = nullptr;
    float closestT = 999999.0f;

    for (Target *target : targetsCopy) {
        if (target->isHit()) continue;

        QVector3D oc = m_cameraPos - target->position();
        float a = QVector3D::dotProduct(ray, ray);
        float b = 2.0f * QVector3D::dotProduct(oc, ray);
        float c = QVector3D::dotProduct(oc, oc) - target->radius() * target->radius();
        float disc = b * b - 4.0f * a * c;

        if (disc < 0.0f) continue;

        float t = (-b - qSqrt(disc)) / (2.0f * a);
        if (t < 0.0f) continue;

        if (t < closestT) {
            closestT = t;
            closestHit = target;
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
    QVector3D ray = screenToRay(screenX, screenY);
    return m_cameraPos + ray * depth;
}

QVector3D GameEngine::getCameraForward() const {
    float yawRad   = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);
    return QVector3D(
        qCos(pitchRad) * qSin(yawRad),
        qSin(pitchRad),
        -qCos(pitchRad) * qCos(yawRad)
    ).normalized();
}

QVector3D GameEngine::screenToRay(int screenX, int screenY) const {
    float aspect = (float)m_screenWidth / (float)m_screenHeight;
    float halfFov = qDegreesToRadians(90.0f / 2.0f);
    float tanFov = qTan(halfFov);

    float nx = (float)(m_screenWidth - screenX) / (float)m_screenWidth * 2.0f - 1.0f;
    float ny = 1.0f - (float)screenY / (float)m_screenHeight * 2.0f;

    float dx = nx * tanFov * aspect;
    float dy = ny * tanFov;

    float yawRad   = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);
    float sx = qCos(pitchRad) * qSin(yawRad);
    float sy = qSin(pitchRad);
    float sz = -qCos(pitchRad) * qCos(yawRad);
    QVector3D forward(sx, sy, sz);

    QVector3D worldUp(0.0f, 1.0f, 0.0f);
    QVector3D right = QVector3D::crossProduct(worldUp, forward).normalized();
    QVector3D up = QVector3D::crossProduct(forward, right).normalized();

    QVector3D ray = (forward + right * dx + up * dy).normalized();
    return ray;
}

QList<Target*> GameEngine::getActiveTargets() const {
    return m_targets;
}
