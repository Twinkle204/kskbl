#include "RoomRenderer.h"
#include <QtMath>

RoomRenderer::RoomRenderer()
    : m_yaw(0.0f)
    , m_pitch(0.0f)
    , m_fov(90.0f)
    , m_screenW(1280)
    , m_screenH(720)
    , m_camPos(0.0f, 0.0f, 0.0f) {
}

void RoomRenderer::setCamera(float yaw, float pitch, float fov,
                             int screenW, int screenH) {
    m_yaw   = yaw;
    m_pitch = pitch;
    m_fov   = fov;
    m_screenW = screenW;
    m_screenH = screenH;
}

void RoomRenderer::setCameraPos(const QVector3D &) {
}

bool RoomRenderer::project(const QVector3D &worldPos, int &sx, int &sy,
                           int screenW, int screenH) const {
    float yawRad   = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);
    float fx = qCos(pitchRad) * qSin(yawRad);
    float fy = qSin(pitchRad);
    float fz = -qCos(pitchRad) * qCos(yawRad);
    QVector3D forward(fx, fy, fz);

    QVector3D worldUp(0.0f, 1.0f, 0.0f);
    QVector3D right = QVector3D::crossProduct(worldUp, forward).normalized();
    QVector3D up    = QVector3D::crossProduct(forward, right).normalized();

    QVector3D rel = worldPos - m_camPos;
    float cz = QVector3D::dotProduct(rel, forward);
    if (cz <= 0.1f) return false;

    float cx = QVector3D::dotProduct(rel, right);
    float cy = QVector3D::dotProduct(rel, up);

    float aspect  = (float)screenW / (float)screenH;
    float halfFov = qDegreesToRadians(m_fov / 2.0f);
    float tanFov  = qTan(halfFov);

    float nx = cx / (cz * tanFov * aspect);
    float ny = cy / (cz * tanFov);

    sx = static_cast<int>((1.0f - nx) * 0.5f * screenW);
    sy = static_cast<int>((1.0f + ny) * 0.5f * screenH);
    return true;
}

void RoomRenderer::projectVertex(const QVector3D &v, int &sx, int &sy, bool &inFront) {
    float yawRad   = qDegreesToRadians(m_yaw);
    float pitchRad = qDegreesToRadians(m_pitch);
    float fx = qCos(pitchRad) * qSin(yawRad);
    float fy = qSin(pitchRad);
    float fz = -qCos(pitchRad) * qCos(yawRad);
    QVector3D forward(fx, fy, fz);

    QVector3D worldUp(0.0f, 1.0f, 0.0f);
    QVector3D right = QVector3D::crossProduct(worldUp, forward).normalized();
    QVector3D up    = QVector3D::crossProduct(forward, right).normalized();

    QVector3D rel = v - m_camPos;
    float cx = QVector3D::dotProduct(rel, right);
    float cy = QVector3D::dotProduct(rel, up);
    float cz = QVector3D::dotProduct(rel, forward);

    inFront = (cz > 0.1f);
    if (!inFront) {
        sx = -99999;
        sy = -99999;
        return;
    }

    float aspect  = (float)m_screenW / (float)m_screenH;
    float halfFov = qDegreesToRadians(m_fov / 2.0f);
    float tanFov  = qTan(halfFov);

    float nx = cx / (cz * tanFov * aspect);
    float ny = cy / (cz * tanFov);

    sx = static_cast<int>((1.0f - nx) * 0.5f * m_screenW);
    sy = static_cast<int>((1.0f + ny) * 0.5f * m_screenH);
}

static inline int clampi(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void RoomRenderer::render(QPainter &painter, int screenW, int screenH) {
    m_screenW = screenW;
    m_screenH = screenH;

    painter.fillRect(0, 0, screenW, screenH, QColor(12, 14, 18));

    const float W     = 60.0f;
    const float H     = 30.0f;
    const float depth = 50.0f;

    QVector3D tl(-W, -H, -depth);
    QVector3D tr( W, -H, -depth);
    QVector3D br( W,  H, -depth);
    QVector3D bl(-W,  H, -depth);

    int sx[4], sy[4];
    bool ok[4];
    projectVertex(tl, sx[0], sy[0], ok[0]);
    projectVertex(tr, sx[1], sy[1], ok[1]);
    projectVertex(br, sx[2], sy[2], ok[2]);
    projectVertex(bl, sx[3], sy[3], ok[3]);

    if (ok[0] && ok[1] && ok[2] && ok[3]) {
        QVector<QPoint> pts(4);
        for (int i = 0; i < 4; ++i)
            pts[i] = QPoint(clampi(sx[i], 0, screenW), clampi(sy[i], 0, screenH));

        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(22, 22, 26));
        painter.drawPolygon(pts);
        painter.restore();

        // Target frame
        float fw = W * 1.4f;
        float fh = H * 1.4f;
        QVector3D ftl(-fw/2, -fh/2, -depth + 0.1f);
        QVector3D ftr( fw/2, -fh/2, -depth + 0.1f);
        QVector3D fbr( fw/2,  fh/2, -depth + 0.1f);
        QVector3D fbl(-fw/2,  fh/2, -depth + 0.1f);

        int s_ftl[2], s_ftr[2], s_fbr[2], s_fbl[2];
        bool ok_ftl, ok_ftr, ok_fbr, ok_fbl;
        projectVertex(ftl, s_ftl[0], s_ftl[1], ok_ftl);
        projectVertex(ftr, s_ftr[0], s_ftr[1], ok_ftr);
        projectVertex(fbr, s_fbr[0], s_fbr[1], ok_fbr);
        projectVertex(fbl, s_fbl[0], s_fbl[1], ok_fbl);

        if (ok_ftl && ok_ftr && ok_fbr && ok_fbl) {
            painter.save();
            QPen borderPen(QColor(80, 85, 95), 3);
            painter.setPen(borderPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawPolygon(QVector<QPoint>{
                QPoint(clampi(s_ftl[0],0,screenW), clampi(s_ftl[1],0,screenH)),
                QPoint(clampi(s_ftr[0],0,screenW), clampi(s_ftr[1],0,screenH)),
                QPoint(clampi(s_fbr[0],0,screenW), clampi(s_fbr[1],0,screenH)),
                QPoint(clampi(s_fbl[0],0,screenW), clampi(s_fbl[1],0,screenH))});

            int s_mt[2], s_mb[2], s_ml[2], s_mr[2];
            bool ok_mt, ok_mb, ok_ml, ok_mr;
            projectVertex(QVector3D(   0, -fh/2, -depth+0.1f), s_mt[0], s_mt[1], ok_mt);
            projectVertex(QVector3D(   0,  fh/2, -depth+0.1f), s_mb[0], s_mb[1], ok_mb);
            projectVertex(QVector3D(-fw/2,   0, -depth+0.1f), s_ml[0], s_ml[1], ok_ml);
            projectVertex(QVector3D( fw/2,   0, -depth+0.1f), s_mr[0], s_mr[1], ok_mr);
            QPen crossPen(QColor(80, 85, 95), 2);
            painter.setPen(crossPen);
            if (ok_ml && ok_mr) painter.drawLine(
                clampi(s_ml[0],0,screenW), clampi(s_ml[1],0,screenH),
                clampi(s_mr[0],0,screenW), clampi(s_mr[1],0,screenH));
            if (ok_mt && ok_mb) painter.drawLine(
                clampi(s_mt[0],0,screenW), clampi(s_mt[1],0,screenH),
                clampi(s_mb[0],0,screenW), clampi(s_mb[1],0,screenH));
            painter.restore();
        }
    }
}
