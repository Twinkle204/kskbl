#pragma once

#include <QObject>
#include <QPainter>
#include <QVector3D>
#include <QMutex>

class Target;
class TargetSpawner;
class RoomRenderer;

class GameEngine : public QObject {
    Q_OBJECT

public:
    explicit GameEngine(QObject *parent = nullptr);
    ~GameEngine();

    void reset();
    void update(float deltaTime);
    void render(QPainter &painter);

    void handleMouseMove(float dx, float dy);
    bool handleClickAt(int screenX, int screenY);

    QVector3D screenToWorld(int screenX, int screenY, float depth = 10.0f) const;
    QVector3D getCameraForward() const;

    void setScreenSize(int w, int h) {
        m_screenWidth = w;
        m_screenHeight = h;
    }

    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    QVector3D getCameraPosition() const { return m_cameraPos; }
    int screenWidth() const { return m_screenWidth; }
    int screenHeight() const { return m_screenHeight; }

    QList<Target*> getActiveTargets() const;
    void renderBackground(QPainter &painter);

signals:
    void targetHit(Target *target);
    void targetMissed(Target *target);

private:
    TargetSpawner *m_targetSpawner;
    RoomRenderer *m_roomRenderer;
    float m_sensitivity;

    QList<Target*> m_targets;
    QMutex m_targetsMutex;

    int m_screenWidth;
    int m_screenHeight;

    float m_yaw;
    float m_pitch;
    QVector3D m_cameraPos;

    QVector3D screenToRay(int screenX, int screenY) const;
};
