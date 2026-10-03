#include "Project.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineF>

// ---- Geometria de acerto (borracha, seleção, conta-gotas) ----

static qreal distToSegment(const QPointF &p, const QPointF &a, const QPointF &b) {
    const QPointF ab = b - a;
    const qreal len2 = ab.x() * ab.x() + ab.y() * ab.y();
    if (len2 <= 1e-9) return QLineF(p, a).length();
    qreal t = ((p.x() - a.x()) * ab.x() + (p.y() - a.y()) * ab.y()) / len2;
    t = qBound(qreal(0.0), t, qreal(1.0));
    return QLineF(p, a + ab * t).length();
}

static bool strokeHit(const Stroke &s, const QPointF &p, qreal r) {
    if (s.points.isEmpty()) return false;
    const qreal reach = r + s.width / 2.0;
    if (s.points.size() == 1) return QLineF(s.points[0], p).length() <= reach;
    for (int i = 1; i < s.points.size(); ++i)
        if (distToSegment(p, s.points[i - 1], s.points[i]) <= reach) return true;
    return false;
}

Project::Project(QObject *parent) : QObject(parent) {
    m_layers.append(LayerInfo{QStringLiteral("Camada 1"), true, 1.0});
    m_frames.append(blankDrawing());
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        setCurrentFrame((m_current + 1) % m_frames.size());
    });
}

Drawing Project::blankDrawing() const {
    Drawing d;
    d.layers.resize(m_layers.size());
    return d;
}

void Project::layersTouched() {
    ++m_layersRevision;
    emit layersChanged();
    emit contentChanged();
}

// ---- Frames ----

void Project::setCurrentFrame(int i) {
    i = qBound(0, i, m_frames.size() - 1);
    if (i == m_current) return;
    clearSelection();
    m_current = i;
    emit currentFrameChanged();
}

void Project::setFps(int f) {
    f = qBound(1, f, 120);
    if (f == m_fps) return;
    m_fps = f;
    if (m_timer.isActive()) m_timer.setInterval(1000 / m_fps);
    emit fpsChanged();
}

void Project::addFrame() {
    clearSelection();
    m_frames.insert(m_current + 1, blankDrawing());
    ++m_current;
    emit frameCountChanged();
    emit currentFrameChanged();
    emit contentChanged();
}

void Project::duplicateFrame() {
    clearSelection();
    m_frames.insert(m_current + 1, m_frames[m_current]);
    ++m_current;
    emit frameCountChanged();
    emit currentFrameChanged();
    emit contentChanged();
}

void Project::removeFrame() {
    clearSelection();
    if (m_frames.size() <= 1) {
        m_frames[0] = blankDrawing();
        emit contentChanged();
        return;
    }
    m_frames.removeAt(m_current);
    m_current = qMin(m_current, m_frames.size() - 1);
    emit frameCountChanged();
    emit currentFrameChanged();
    emit contentChanged();
}

void Project::clearFrame() {
    clearSelection();
    strokes().clear();
    emit contentChanged();
}

void Project::undo() {
    clearSelection();
    auto &s = strokes();
    if (s.isEmpty()) return;
    s.removeLast();
    emit contentChanged();
}

void Project::togglePlay() {
    if (m_timer.isActive()) m_timer.stop();
    else m_timer.start(1000 / m_fps);
    emit playingChanged();
    emit contentChanged();
}

// ---- Camadas ----

void Project::setCurrentLayer(int i) {
    i = qBound(0, i, m_layers.size() - 1);
    if (i == m_layer) return;
    clearSelection();
    m_layer = i;
    emit currentLayerChanged();
    layersTouched();
}

void Project::addLayer() {
    clearSelection();
    m_layers.append(LayerInfo{QStringLiteral("Camada %1").arg(m_layers.size() + 1), true, 1.0});
    for (Drawing &d : m_frames) d.layers.append(QVector<Stroke>());
    m_layer = m_layers.size() - 1;
    emit layerCountChanged();
    emit currentLayerChanged();
    layersTouched();
}

void Project::removeLayer() {
    if (m_layers.size() <= 1) return;
    clearSelection();
    m_layers.removeAt(m_layer);
    for (Drawing &d : m_frames) d.layers.removeAt(m_layer);
    m_layer = qMin(m_layer, m_layers.size() - 1);
    emit layerCountChanged();
    emit currentLayerChanged();
    layersTouched();
}

void Project::moveLayer(int from, int to) {
    const int n = m_layers.size();
    if (from < 0 || from >= n || to < 0 || to >= n || from == to) return;
    clearSelection();
    m_layers.move(from, to);
    for (Drawing &d : m_frames) d.layers.move(from, to);
    // A seleção acompanha a camada movida; as demais deslocam uma posição.
    if (m_layer == from) m_layer = to;
    else if (from < m_layer && m_layer <= to) --m_layer;
    else if (to <= m_layer && m_layer < from) ++m_layer;
    emit currentLayerChanged();
    layersTouched();
}

void Project::moveLayerUp() { moveLayer(m_layer, m_layer + 1); }
void Project::moveLayerDown() { moveLayer(m_layer, m_layer - 1); }

QString Project::layerName(int i) const {
    return (i >= 0 && i < m_layers.size()) ? m_layers[i].name : QString();
}

bool Project::layerVisible(int i) const {
    return (i >= 0 && i < m_layers.size()) ? m_layers[i].visible : false;
}

qreal Project::layerOpacity(int i) const {
    return (i >= 0 && i < m_layers.size()) ? m_layers[i].opacity : 1.0;
}

void Project::renameLayer(int i, const QString &name) {
    if (i < 0 || i >= m_layers.size()) return;
    m_layers[i].name = name;
    layersTouched();
}

void Project::setLayerVisible(int i, bool v) {
    if (i < 0 || i >= m_layers.size() || m_layers[i].visible == v) return;
    m_layers[i].visible = v;
    layersTouched();
}

void Project::setLayerOpacity(int i, qreal o) {
    if (i < 0 || i >= m_layers.size()) return;
    m_layers[i].opacity = qBound(0.0, o, 1.0);
    layersTouched();
}

// ---- Desenho ----

void Project::beginStroke(const QColor &color, qreal width, qreal x, qreal y, qreal pressure) {
    clearSelection();
    Stroke s;
    s.color = color;
    s.width = width;
    s.points.append(QPointF(x, y));
    s.pressure.append(float(pressure));
    strokes().append(s);
    emit contentChanged();
}

void Project::appendPoint(qreal x, qreal y, qreal pressure) {
    auto &st = strokes();
    if (st.isEmpty()) return;
    st.last().points.append(QPointF(x, y));
    st.last().pressure.append(float(pressure));
    emit contentChanged();
}

void Project::eraseAt(qreal x, qreal y, qreal radius) {
    auto &st = strokes();
    bool changed = false;
    for (int i = st.size() - 1; i >= 0; --i) {
        if (strokeHit(st[i], QPointF(x, y), radius)) {
            st.removeAt(i);
            changed = true;
        }
    }
    if (changed) {
        clearSelection();
        emit contentChanged();
    }
}

// ---- Seleção ----

void Project::clearSelection() {
    if (m_selected < 0) return;
    m_selected = -1;
    emit selectionChanged();
    emit contentChanged();
}

bool Project::selectAt(qreal x, qreal y, qreal radius) {
    const auto &st = strokes();
    for (int i = st.size() - 1; i >= 0; --i) {
        if (strokeHit(st[i], QPointF(x, y), radius)) {
            if (m_selected != i) {
                m_selected = i;
                emit selectionChanged();
                emit contentChanged();
            }
            return true;
        }
    }
    clearSelection();
    return false;
}

void Project::moveSelected(qreal dx, qreal dy) {
    auto &st = strokes();
    if (m_selected < 0 || m_selected >= st.size()) return;
    for (QPointF &pt : st[m_selected].points) pt += QPointF(dx, dy);
    emit contentChanged();
}

void Project::deleteSelected() {
    auto &st = strokes();
    if (m_selected < 0 || m_selected >= st.size()) return;
    st.removeAt(m_selected);
    m_selected = -1;
    emit selectionChanged();
    emit contentChanged();
}

void Project::recolorSelected(const QColor &color) {
    auto &st = strokes();
    if (m_selected < 0 || m_selected >= st.size()) return;
    st[m_selected].color = color;
    emit contentChanged();
}

QRectF Project::selectedBounds() const {
    if (m_selected < 0) return QRectF();
    const QVector<Stroke> &st = m_frames[m_current].layers[m_layer];
    if (m_selected >= st.size() || st[m_selected].points.isEmpty()) return QRectF();
    const Stroke &s = st[m_selected];
    qreal minX = s.points[0].x(), maxX = minX;
    qreal minY = s.points[0].y(), maxY = minY;
    for (const QPointF &p : s.points) {
        minX = qMin(minX, p.x());
        maxX = qMax(maxX, p.x());
        minY = qMin(minY, p.y());
        maxY = qMax(maxY, p.y());
    }
    const qreal m = s.width / 2.0 + 6.0;
    return QRectF(minX - m, minY - m, (maxX - minX) + 2.0 * m, (maxY - minY) + 2.0 * m);
}

QString Project::pickColor(qreal x, qreal y, qreal radius) const {
    const Drawing &d = m_frames[m_current];
    for (int li = d.layers.size() - 1; li >= 0; --li) {
        if (!m_layers[li].visible) continue;
        const QVector<Stroke> &st = d.layers[li];
        for (int i = st.size() - 1; i >= 0; --i)
            if (strokeHit(st[i], QPointF(x, y), radius)) return st[i].color.name();
    }
    return QString();
}

// ---- Arquivo (.tbm, formato versão 2) ----

static QJsonArray strokesToJson(const QVector<Stroke> &strokes) {
    QJsonArray out;
    for (const Stroke &s : strokes) {
        QJsonArray pts, pr;
        for (const QPointF &p : s.points) { pts.append(p.x()); pts.append(p.y()); }
        for (float v : s.pressure) pr.append(double(v));
        out.append(QJsonObject{
            {"color", s.color.name(QColor::HexArgb)},
            {"width", s.width},
            {"points", pts},
            {"pressure", pr}});
    }
    return out;
}

static QVector<Stroke> strokesFromJson(const QJsonArray &arr) {
    QVector<Stroke> out;
    for (const QJsonValue &sv : arr) {
        const QJsonObject so = sv.toObject();
        Stroke s;
        s.color = QColor(so.value("color").toString());
        s.width = so.value("width").toDouble(6.0);
        const QJsonArray pts = so.value("points").toArray();
        for (int i = 0; i + 1 < pts.size(); i += 2)
            s.points.append(QPointF(pts[i].toDouble(), pts[i + 1].toDouble()));
        for (const QJsonValue &v : so.value("pressure").toArray())
            s.pressure.append(float(v.toDouble(1.0)));
        while (s.pressure.size() < s.points.size()) s.pressure.append(1.0f);
        out.append(s);
    }
    return out;
}

bool Project::save(const QString &path) const {
    QJsonArray layers;
    for (const LayerInfo &l : m_layers)
        layers.append(QJsonObject{{"name", l.name}, {"visible", l.visible}, {"opacity", l.opacity}});

    QJsonArray frames;
    for (const Drawing &d : m_frames) {
        QJsonArray perLayer;
        for (const QVector<Stroke> &ls : d.layers)
            perLayer.append(QJsonObject{{"strokes", strokesToJson(ls)}});
        frames.append(QJsonObject{{"layers", perLayer}});
    }

    QJsonObject root{{"format", "toon-bm"}, {"version", 2}, {"fps", m_fps},
                     {"layers", layers}, {"frames", frames}};
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    return true;
}

bool Project::load(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
    if (root.value("format").toString() != "toon-bm") return false;
    if (root.value("version").toInt() != 2) return false;

    QVector<LayerInfo> layers;
    for (const QJsonValue &lv : root.value("layers").toArray()) {
        const QJsonObject lo = lv.toObject();
        layers.append(LayerInfo{lo.value("name").toString(), lo.value("visible").toBool(true),
                                lo.value("opacity").toDouble(1.0)});
    }
    if (layers.isEmpty()) layers.append(LayerInfo{QStringLiteral("Camada 1"), true, 1.0});

    QVector<Drawing> frames;
    for (const QJsonValue &fv : root.value("frames").toArray()) {
        Drawing d;
        for (const QJsonValue &lv : fv.toObject().value("layers").toArray())
            d.layers.append(strokesFromJson(lv.toObject().value("strokes").toArray()));
        d.layers.resize(layers.size());
        frames.append(d);
    }
    if (frames.isEmpty()) {
        Drawing d;
        d.layers.resize(layers.size());
        frames.append(d);
    }

    clearSelection();
    m_layers = layers;
    m_frames = frames;
    m_current = 0;
    m_layer = 0;
    setFps(root.value("fps").toInt(12));
    emit layerCountChanged();
    emit currentLayerChanged();
    emit frameCountChanged();
    emit currentFrameChanged();
    layersTouched();
    return true;
}
