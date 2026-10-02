#include "CanvasItem.h"
#include <QMouseEvent>
#include <QPainter>
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

void CanvasItem::drawDrawing(QPainter *p, const Drawing &d, qreal opacity) const {
    for (const Stroke &s : d.strokes) {
        QColor c = s.color;
        c.setAlphaF(c.alphaF() * opacity);
        QPen pen(c, s.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        if (s.points.size() == 1) {
            p->setPen(Qt::NoPen);
            p->setBrush(c);
            const qreal r = s.width * (0.3 + 0.7 * s.pressure.value(0, 1.0f)) / 2.0;
            p->drawEllipse(s.points[0], r, r);
            continue;
        }
        for (int i = 1; i < s.points.size(); ++i) {
            const qreal pr = (s.pressure.value(i - 1, 1.0f) + s.pressure.value(i, 1.0f)) / 2.0;
            pen.setWidthF(s.width * (0.3 + 0.7 * pr));
            p->setPen(pen);
            p->drawLine(s.points[i - 1], s.points[i]);
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
