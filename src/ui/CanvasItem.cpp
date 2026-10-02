#include "CanvasItem.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

CanvasItem::CanvasItem(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::AllButtons);
    setAntialiasing(true);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
}

void CanvasItem::setProject(Project *p) {
    if (m_project == p) return;
    if (m_project) disconnect(m_project, nullptr, this, nullptr);
    m_project = p;
    if (m_project) {
        connect(m_project, &Project::contentChanged, this, [this] { update(); });
        connect(m_project, &Project::currentFrameChanged, this, [this] { update(); });
        connect(m_project, &Project::layersChanged, this, [this] { update(); });
    }
    emit projectChanged();
    update();
}

qreal CanvasItem::scale() const {
    return qMax(0.01, qMin(width() / kStageW, height() / kStageH));
}

QPointF CanvasItem::offset() const {
    const qreal s = scale();
    return QPointF((width() - kStageW * s) / 2.0, (height() - kStageH * s) / 2.0);
}

QPointF CanvasItem::toStage(const QPointF &p) const {
    return (p - offset()) / scale();
}

qreal CanvasItem::pressureOf(QMouseEvent *e) {
    if (e->points().isEmpty()) return 1.0;
    const qreal p = e->points().first().pressure();
    return p > 0.0 ? p : 1.0;
}

// Desenha um traço suavizado: curvas quadráticas entre os pontos médios dos segmentos,
// usando cada ponto capturado como ponto de controle. A espessura segue a pressão.
static void drawSmoothStroke(QPainter *p, const Stroke &s, const QColor &c) {
    const int n = s.points.size();
    if (n == 0) return;

    auto widthAt = [&](int i) { return s.width * (0.3 + 0.7 * s.pressure.value(i, 1.0f)); };

    if (n == 1) {
        const qreal r = widthAt(0) / 2.0;
        p->setPen(Qt::NoPen);
        p->setBrush(c);
        p->drawEllipse(s.points[0], r, r);
        return;
    }

    QPen pen(c, s.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setBrush(Qt::NoBrush);

    if (n == 2) {
        pen.setWidthF((widthAt(0) + widthAt(1)) / 2.0);
        p->setPen(pen);
        p->drawLine(s.points[0], s.points[1]);
        return;
    }

    auto mid = [&](int i) { return (s.points[i] + s.points[i + 1]) / 2.0; };

    pen.setWidthF(widthAt(0));
    p->setPen(pen);
    p->drawLine(s.points[0], mid(0));

    for (int i = 1; i < n - 1; ++i) {
        QPainterPath path;
        path.moveTo(mid(i - 1));
        path.quadTo(s.points[i], mid(i));
        pen.setWidthF(widthAt(i));
        p->setPen(pen);
        p->drawPath(path);
    }

    pen.setWidthF(widthAt(n - 1));
    p->setPen(pen);
    p->drawLine(mid(n - 2), s.points[n - 1]);
}

// Desenha as camadas de baixo para cima, respeitando visibilidade e opacidade.
void CanvasItem::drawDrawing(QPainter *p, const Drawing &d, qreal opacity) const {
    for (int li = 0; li < d.layers.size(); ++li) {
        if (!m_project->layerVisible(li)) continue;
        const qreal layerOpacity = opacity * m_project->layerOpacity(li);
        for (const Stroke &s : d.layers[li]) {
            QColor c = s.color;
            c.setAlphaF(c.alphaF() * layerOpacity);
            drawSmoothStroke(p, s, c);
        }
    }
}

void CanvasItem::paint(QPainter *p) {
    if (!m_project) return;
    p->setRenderHint(QPainter::Antialiasing);
    p->translate(offset());
    p->scale(scale(), scale());
    const QRectF stage(0, 0, kStageW, kStageH);
    p->fillRect(stage, Qt::white);
    p->setClipRect(stage);

    const int cur = m_project->currentFrame();
    if (m_onion && !m_project->playing() && cur > 0)
        drawDrawing(p, m_project->drawing(cur - 1), 0.25);
    drawDrawing(p, m_project->drawing(cur), 1.0);
}

void CanvasItem::mousePressEvent(QMouseEvent *e) {
    if (!m_project || m_project->playing()) return;
    // Não desenha em camada oculta.
    if (!m_project->layerVisible(m_project->currentLayer())) return;
    const QPointF s = toStage(e->position());
    m_down = true;
    if (m_eraser) m_project->eraseAt(s.x(), s.y(), m_size * 2.0);
    else m_project->beginStroke(m_color, m_size, s.x(), s.y(), pressureOf(e));
    e->accept();
}

void CanvasItem::mouseMoveEvent(QMouseEvent *e) {
    if (!m_project || !m_down) return;
    const QPointF s = toStage(e->position());
    if (m_eraser) m_project->eraseAt(s.x(), s.y(), m_size * 2.0);
    else m_project->appendPoint(s.x(), s.y(), pressureOf(e));
    e->accept();
}

void CanvasItem::mouseReleaseEvent(QMouseEvent *e) {
    m_down = false;
    e->accept();
}
