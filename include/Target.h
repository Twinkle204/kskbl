#pragma once

#include <QObject>
#include <QVector3D>
#include <QColor>
#include <QPainter>
#include <QTimer>

enum class TargetState {
    Active,
    Hit,
    Expired
};

enum class TargetType {
    Static,
    Linear,
    Sine,
    Circular,
    RandomJump
};

class Target : public QObject {
    Q_OBJECT

public:
    Target(int id, QVector3D position, float radius, TargetType type = TargetType::Static, QObject *parent = nullptr);
    ~Target();

    void update(float deltaTime);

    void render(QPainter &painter,
                const QVector3D &cameraPos,
                float cameraYaw,
                float cameraPitch,
                int screenWidth,
                int screenHeight);

    int id() const { return m_id; }
    QVector3D position() const { return m_position; }
    float radius() const { return m_radius; }
    bool isHit() const { return m_hit; }
    void setHit(bool hit) { m_hit = hit; }
    bool isExpired() const { return m_expired; }

    TargetType type() const { return m_type; }
    QColor color() const { return m_color; }

    float lifetime() const { return m_lifetime; }
    void setLifetime(float lifetime) { m_lifetime = lifetime; }

    float spawnTime() const { return m_spawnTime; }

private:
    void updatePosition(float deltaTime);
    QPoint worldToScreen(const QVector3D &worldPos,
                         const QVector3D &cameraPos,
                         float yaw,
                         float pitch,
                         int screenWidth,
                         int screenHeight) const;

    int m_id;
    QVector3D m_position;
    QVector3D m_startPosition;
    QVector3D m_velocity;
    float m_radius;
    TargetState m_state;
    TargetType m_type;
    QColor m_color;
    bool m_hit;
    bool m_expired;

    float m_lifetime;
    float m_maxLifetime;
    float m_spawnTime;
    float m_time;

    float m_angularSpeed;
    float m_orbitRadius;
    float m_angle;
};
