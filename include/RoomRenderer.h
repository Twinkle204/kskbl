#pragma once

#include <QPainter>
#include <QVector3D>

class RoomRenderer {
public:
    RoomRenderer();

    void setCamera(float yaw, float pitch, float fov,
                   int screenW, int screenH);
    void setCameraPos(const QVector3D &pos);

    void render(QPainter &painter, int screenW, int screenH);

    void projectVertex(const QVector3D &v, int &sx, int &sy, bool &inFront);
    bool project(const QVector3D &worldPos,
                 int &sx, int &sy,
                 int screenW, int screenH) const;

    int screenWidth()  const { return m_screenW; }
    int screenHeight() const { return m_screenH; }

private:
    float m_yaw, m_pitch, m_fov;
    int m_screenW, m_screenH;
    QVector3D m_camPos;
};
