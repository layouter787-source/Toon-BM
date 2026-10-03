#pragma once
#include <QColor>
#include <QQuickPaintedItem>
#include "core/Project.h"

// Palco de desenho. Coordenadas internas fixas em 1920x1080, escaladas para caber no item.
// Registrado no QML manualmente em main.cpp (módulo "ToonBM.Core").
class CanvasItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(Project *project READ project WRITE setProject NOTIFY projectChanged)
    Q_PROPERTY(QColor brushColor READ brushColor WRITE setBrushColor NOTIFY brushChanged)
    Q_PROPERTY(qreal brushSize READ brushSize WRITE setBrushSize NOTIFY brushChanged)
    Q_PROPERTY(bool eraser READ eraser WRITE setEraser NOTIFY brushChanged)
    Q_PROPERTY(bool onionSkin READ onionSkin WRITE setOnionSkin NOTIFY brushChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY viewChanged)
    Q_PROPERTY(qreal panX READ panX WRITE setPanX NOTIFY viewChanged)
    Q_PROPERTY(qreal panY READ panY WRITE setPanY NOTIFY viewChanged)
public:
    static constexpr qreal kStageW = 1920.0;
    static constexpr qreal kStageH = 1080.0;

    explicit CanvasItem(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

    Project *project() const { return m_project; }
    void setProject(Project *p);
    QColor brushColor() const { return m_color; }
    void setBrushColor(const QColor &c) { m_color = c; emit brushChanged(); }
    qreal brushSize() const { return m_size; }
    void setBrushSize(qreal s) { m_size = s; emit brushChanged(); }
    bool eraser() const { return m_eraser; }
    void setEraser(bool e) { m_eraser = e; emit brushChanged(); }
    bool onionSkin() const { return m_onion; }
    void setOnionSkin(bool o) { m_onion = o; emit brushChanged(); update(); }

    qreal zoom() const { return m_zoom; }
    void setZoom(qreal z) { m_zoom = qBound(0.2, z, 10.0); emit viewChanged(); update(); }
    qreal panX() const { return m_panX; }
    void setPanX(qreal v) { m_panX = v; emit viewChanged(); update(); }
    qreal panY() const { return m_panY; }
    void setPanY(qreal v) { m_panY = v; emit viewChanged(); update(); }

    // Volta o palco para "caber na tela".
    Q_INVOKABLE void resetView();
    // Cancela o traço em andamento (usado quando começa um gesto de dois dedos).
    Q_INVOKABLE void cancelStroke();

signals:
    void projectChanged();
    void brushChanged();
    void viewChanged();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    qreal scale() const;
    QPointF offset() const;
    QPointF toStage(const QPointF &p) const;
    static qreal pressureOf(QMouseEvent *e);
    void drawDrawing(QPainter *p, const Drawing &d, qreal opacity) const;

    Project *m_project = nullptr;
    QColor m_color{Qt::black};
    qreal m_size = 6.0;
    bool m_eraser = false;
    bool m_onion = true;
    bool m_down = false;
    qreal m_zoom = 1.0;
    qreal m_panX = 0.0;
    qreal m_panY = 0.0;
};
