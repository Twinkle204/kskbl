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
    , m_angle(0.0f)
    , m_screenX(0.5f)
    , m_screenY(0.5f) {

    m_color = QColor(220, 40, 40);

    switch (type) {
        case TargetType::Linear:
            m_velocity = QVector3D(2.0f, 1.0f, 0.0f);
            break;
        case TargetType::Sine:
            m_velocity = QVector3D(3.0f, 0.0f, 0.0f);
            break;
        case TargetType::Circular:
            m_orbitRadius = 3.0f;
            m_angularSpeed = 2.0f;
            break;
        case TargetType::RandomJump:
            break;
        case TargetType::Static:
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
    float xMin = -20.0f;
    float xMax = 20.0f;
    float yMin = -7.0f;
    float yMax = 7.0f;

    switch (m_type) {
        case TargetType::Static:
            break;

        case TargetType::Linear:
            m_position += m_velocity * deltaTime;
            if (m_position.x() < xMin || m_position.x() > xMax) {
                m_velocity.setX(-m_velocity.x());
            }
            if (m_position.y() < yMin || m_position.y() > yMax) {
                m_velocity.setY(-m_velocity.y());
            }
            break;

        case TargetType::Sine:
            m_position += m_velocity * deltaTime;
            m_position.setY(m_startPosition.y() + qSin(m_time * 3.0f) * 3.0f);
            if (m_position.x() < xMin || m_position.x() > xMax) {
                m_velocity.setX(-m_velocity.x());
            }
            break;

        case TargetType::Circular:
            m_angle += m_angularSpeed * deltaTime;
            m_position.setX(m_startPosition.x() + qCos(m_angle) * m_orbitRadius);
            m_position.setY(m_startPosition.y() + qSin(m_angle) * m_orbitRadius);
            break;

        case TargetType::RandomJump:
            if (m_time > 1.5f && QRandomGenerator::global()->bounded(60) == 0) {
                float jx = xMin + 5.0f + QRandomGenerator::global()->bounded(xMax - xMin - 10.0f);
                float jy = yMin + 3.0f + QRandomGenerator::global()->bounded(yMax - yMin - 6.0f);
                m_position.setX(jx);
                m_position.setY(jy);
                m_startPosition = m_position;
                m_time = 0.0f;
            }
            break;
    }

    m_position.setX(qBound(xMin, m_position.x(), xMax));
    m_position.setY(qBound(yMin, m_position.y(), yMax));
}

void Target::render(QPainter &painter,
                    const QVector3D &cameraPos,
                    float cameraYaw,
                    float cameraPitch,
                    int screenWidth,
                    int screenHeight) {
    if (m_hit) return;

    float yawRad   = qDegreesToRadians(cameraYaw);
    float pitchRad = qDegreesToRadians(cameraPitch);
    float fx = qCos(pitchRad) * qSin(yawRad);
    float fy = qSin(pitchRad);
    float fz = -qCos(pitchRad) * qCos(yawRad);
    QVector3D forward(fx, fy, fz);

    QVector3D worldUp(0.0f, 1.0f, 0.0f);
    QVector3D right = QVector3D::crossProduct(worldUp, forward).normalized();
    QVector3D up = QVector3D::crossProduct(forward, right).normalized();

    QVector3D rel = m_position - cameraPos;
    float cz = QVector3D::dotProduct(rel, forward);
    if (cz <= 0.1f) return;

    float cx = QVector3D::dotProduct(rel, right);
    float cy = QVector3D::dotProduct(rel, up);

    float aspect = (float)screenWidth / (float)screenHeight;
    float halfFov = qDegreesToRadians(90.0f / 2.0f);
    float tanFov = qTan(halfFov);

    float nx = cx / (cz * tanFov * aspect);
    float ny = cy / (cz * tanFov);

    float sx = (1.0f - nx) * 0.5f * screenWidth;
    float sy = (1.0f + ny) * 0.5f * screenHeight;

    if (sx < -200 || sx > screenWidth + 200 || sy < -200 || sy > screenHeight + 200) return;

    m_screenX = sx / screenWidth;
    m_screenY = sy / screenHeight;

    float screenDist = cz;
    float angularRadius = m_radius / screenDist;
    float visualRadius = angularRadius * screenHeight / (2.0f * tanFov);
    visualRadius = qBound(6.0f, visualRadius, 80.0f);

    float lifetimeRatio = m_lifetime / m_maxLifetime;
    int alpha = static_cast<int>(255 * qBound(0.0f, lifetimeRatio * 2.0f, 1.0f));

    QColor targetColor = m_color;
    targetColor.setAlpha(alpha);

    painter.save();
    painter.setPen(Qt::NoPen);

    QRadialGradient gradient(QPointF(sx, sy), visualRadius);
    gradient.setColorAt(0.0, QColor(255, 120, 120, alpha));
    gradient.setColorAt(0.5, targetColor);
    gradient.setColorAt(1.0, QColor(120, 20, 20, alpha));

    painter.setBrush(gradient);
    painter.drawEllipse(QPointF(sx, sy), (int)visualRadius, (int)visualRadius);

    QColor ringColor(255, 255, 255, alpha);
    QPen ringPen(ringColor, 2);
    painter.setPen(ringPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPointF(sx, sy), (int)(visualRadius / 2.0f), (int)(visualRadius / 2.0f));

    if (visualRadius > 15) {
        QColor dotColor(255, 255, 200, alpha);
        painter.setPen(QPen(dotColor, 1));
        painter.drawEllipse(QPointF(sx, sy), 3, 3);
    }

    painter.restore();
}

QPoint Target::worldToScreen(const QVector3D &,
                             const QVector3D &,
                             float,
                             float,
                             int,
                             int) const {
    return QPoint(0, 0);
}
