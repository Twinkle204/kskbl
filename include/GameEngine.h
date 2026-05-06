#pragma once

#include <QObject>
#include <QPainter>
#include <QVector3D>
#include <QQuaternion>
#include <QList>

class Target;
class TargetSpawner;

class GameEngine : public QObject {
    Q_OBJECT

public:
    explicit GameEngine(QObject *parent = nullptr);
    ~GameEngine();

    void reset();

    void update(float deltaTime);
    void render(QPainter &painter);

    void handleMouseMove(int dx, int dy);
    bool handleClick(int screenX, int screenY);

    QVector3D screenToWorld(int screenX, int screenY, float depth = 10.0f) const;
    QVector3D getCameraForward() const;

    void setCameraRotation(float yaw, float pitch);
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }

    QList<Target*> getActiveTargets() const;

signals:
    void targetHit(Target *target);
    void targetMissed(Target *target);

private:
    void updateCamera();

    TargetSpawner *m_targetSpawner;

    float m_yaw;
    float m_pitch;
    float m_sensitivity;

    QVector3D m_cameraPos;
    QQuaternion m_cameraRotation;

    QList<Target*> m_targets;

    int m_fov;
    int m_screenWidth;
    int m_screenHeight;
};
