#include "Target.h"
#include "TargetSpawner.h"
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>

Target::Target(int id, QVector3D position, float radius, TargetType type, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_position(position)
    , m_startPosition(position)
    , m_velocity(0, 0, 0)
    , m_radius(radius)
    , m_state(TargetState::Active)
    , m_type(type)
    , m_hit(false)
    , m_expired(false)
    , m_lifetime(3.0f)
    , m_maxLifetime(3.0f)
    , m_spawnTime(0.0f)
    , m_time(0.0f)
    , m_angularSpeed(0.0f)
    , m_orbitRadius(0.0f)
    , m_angle(0.0f) {

    switch (type) {
        case TargetType::Static:
            m_color = QColor(255, 100, 100);
            break;
        case TargetType::Linear:
            m_color = QColor(100, 255, 100);
            m_velocity = QVector3D(2.0f, 0.5f, 0.0f);
            break;
        case TargetType::Sine:
            m_color = QColor(100, 100, 255);
            m_velocity = QVector3D(3.0f, 0.0f, 0.0f);
            break;
        case TargetType::Circular:
            m_color = QColor(255, 255, 100);
            m_orbitRadius = 3.0f;
            m_angularSpeed = 2.0f;
            break;
        case TargetType::RandomJump:
            m_color = QColor(255, 100, 255);
            break;
    }
}

Target::~Target() {
}

void Target::update(float deltaTime) {
    m_time += deltaTime;
    m_spawnTime += deltaTime;
    m_lifetime -= deltaTime;

    if (m_lifetime <= 0) {
        m_expired = true;
        return;
    }

    updatePosition(deltaTime);
}

void Target::updatePosition(float deltaTime) {
    switch (m_type) {
        case TargetType::Static:
            break;

        case TargetType::Linear:
            m_position += m_velocity * deltaTime;
            if (qAbs(m_position.x()) > 15.0f || qAbs(m_position.y()) > 10.0f) {
                m_velocity = -m_velocity;
            }
            break;

        case TargetType::Sine:
            m_position += m_velocity * deltaTime;
            m_position.setY(m_startPosition.y() + qSin(m_time * 3.0f) * 3.0f);
            if (qAbs(m_position.x()) > 15.0f) {
                m_velocity = -m_velocity;
            }
            break;

        case TargetType::Circular:
            m_angle += m_angularSpeed * deltaTime;
            m_position.setX(m_startPosition.x() + qCos(m_angle) * m_orbitRadius);
            m_position.setY(m_startPosition.y() + qSin(m_angle) * m_orbitRadius);
            break;

        case TargetType::RandomJump:
        if (m_time > 1.5f && QRandomGenerator::global()->bounded(60) == 0) {
                m_position.setX(m_startPosition.x() + (QRandomGenerator::global()->bounded(10) - 5) * 1.0f);
                m_position.setY(m_startPosition.y() + (QRandomGenerator::global()->bounded(6) - 3) * 1.0f);
                m_time = 0.0f;
            }
            break;
    }
}

void Target::render(QPainter &painter,
                    const QVector3D &cameraPos,
                    float cameraYaw,
                    float cameraPitch,
                    int screenWidth,
                    int screenHeight) {

    if (m_hit) return;

    QPoint screenPos = worldToScreen(m_position, cameraPos, cameraYaw, cameraPitch,
                                     screenWidth, screenHeight);

    if (screenPos.x() < -100 || screenPos.x() > screenWidth + 100 ||
        screenPos.y() < -100 || screenPos.y() > screenHeight + 100) {
        return;
    }

    float dist = (m_position - cameraPos).length();
    float visualRadius = qBound(10.0f, m_radius * 300.0f / dist, 100.0f);

    float lifetimeRatio = m_lifetime / m_maxLifetime;
    int alpha = static_cast<int>(255 * qBound(0.0f, lifetimeRatio * 2.0f, 1.0f));

    QColor targetColor = m_color;
    targetColor.setAlpha(alpha);

    QColor outerColor = targetColor.darker(150);
    outerColor.setAlpha(alpha);
    QColor innerColor = targetColor.lighter(150);

    painter.setPen(Qt::NoPen);

    QRadialGradient gradient(screenPos, visualRadius);
    gradient.setColorAt(0.0, innerColor);
    gradient.setColorAt(0.6, targetColor);
    gradient.setColorAt(1.0, outerColor);

    painter.setBrush(gradient);
    painter.drawEllipse(screenPos, static_cast<int>(visualRadius), static_cast<int>(visualRadius));

    painter.setBrush(Qt::NoBrush);
    QColor ringColor = Qt::white;
    ringColor.setAlpha(alpha);
    QPen ringPen(ringColor, 2);
    painter.setPen(ringPen);
    painter.drawEllipse(screenPos, static_cast<int>(visualRadius * 0.6f), static_cast<int>(visualRadius * 0.6f));

    if (visualRadius > 20) {
        QColor whiteColor = Qt::white;
        whiteColor.setAlpha(alpha);
        painter.setPen(QPen(whiteColor, 1));
        painter.drawEllipse(screenPos, 3, 3);
    }
}

QPoint Target::worldToScreen(const QVector3D &worldPos,
                             const QVector3D &cameraPos,
                             float yaw,
                             float pitch,
                             int screenWidth,
                             int screenHeight) const {

    QVector3D relPos = worldPos - cameraPos;

    float yawRad = qDegreesToRadians(yaw);
    float pitchRad = qDegreesToRadians(pitch);

    float x = relPos.x();
    float z = relPos.z();
    float y = relPos.y();

    float rx = x * qCos(yawRad) - z * qSin(yawRad);
    float rz = x * qSin(yawRad) + z * qCos(yawRad);

    float ry = y * qCos(pitchRad) - rz * qSin(pitchRad);
    float rz2 = y * qSin(pitchRad) + rz * qCos(pitchRad);

    if (rz2 <= 0.1f) {
        return QPoint(-10000, -10000);
    }

    float fov = 90.0f;
    float aspect = static_cast<float>(screenWidth) / screenHeight;
    float tanFov = qTan(qDegreesToRadians(fov / 2.0f));

    float sx = (rx / (rz2 * tanFov * aspect)) * (screenWidth / 2.0f) + screenWidth / 2.0f;
    float sy = (-ry / (rz2 * tanFov)) * (screenHeight / 2.0f) + screenHeight / 2.0f;

    return QPoint(static_cast<int>(sx), static_cast<int>(sy));
}
