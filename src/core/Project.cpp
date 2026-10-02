#include "Project.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineF>

Project::Project(QObject *parent) : QObject(parent) {
    m_frames.append(Drawing{});
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, [this] {
        setCurrentFrame((m_current + 1) % m_frames.size());
    });
}

void Project::setCurrentFrame(int i) {
    i = qBound(0, i, m_frames.size() - 1);
    if (i == m_current) return;
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
    m_frames.insert(m_current + 1, Drawing{});
    ++m_current;
    emit frameCountChanged();
    emit currentFrameChanged();
    emit contentChanged();
}

void Project::duplicateFrame() {
    m_frames.insert(m_current + 1, m_frames[m_current]);
    ++m_current;
    emit frameCountChanged();
    emit currentFrameChanged();
    emit contentChanged();
}

void Project::removeFrame() {
    if (m_frames.size() <= 1) {
        m_frames[0] = Drawing{};
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
    m_frames[m_current] = Drawing{};
    emit contentChanged();
}

void Project::undo() {
    auto &s = m_frames[m_current].strokes;
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

void Project::beginStroke(const QColor &color, qreal width, qreal x, qreal y, qreal pressure) {
    Stroke s;
    s.color = color;
    s.width = width;
    s.points.append(QPointF(x, y));
    s.pressure.append(float(pressure));
    m_frames[m_current].strokes.append(s);
    emit contentChanged();
}

void Project::appendPoint(qreal x, qreal y, qreal pressure) {
    auto &st = m_frames[m_current].strokes;
    if (st.isEmpty()) return;
    st.last().points.append(QPointF(x, y));
    st.last().pressure.append(float(pressure));
    emit contentChanged();
}

static bool strokeHit(const Stroke &s, const QPointF &p, qreal r) {
    const qreal reach = r + s.width / 2.0;
    for (const QPointF &pt : s.points)
        if (QLineF(pt, p).length() <= reach) return true;
    return false;
}

void Project::eraseAt(qreal x, qreal y, qreal radius) {
    auto &st = m_frames[m_current].strokes;
    bool changed = false;
    for (int i = st.size() - 1; i >= 0; --i) {
        if (strokeHit(st[i], QPointF(x, y), radius)) {
            st.removeAt(i);
            changed = true;
        }
    }
    if (changed) emit contentChanged();
}

bool Project::save(const QString &path) const {
    QJsonArray frames;
    for (const Drawing &d : m_frames) {
        QJsonArray strokes;
        for (const Stroke &s : d.strokes) {
            QJsonArray pts, pr;
            for (const QPointF &p : s.points) { pts.append(p.x()); pts.append(p.y()); }
            for (float v : s.pressure) pr.append(double(v));
            strokes.append(QJsonObject{
                {"color", s.color.name(QColor::HexArgb)},
                {"width", s.width},
                {"points", pts},
                {"pressure", pr}});
        }
        frames.append(QJsonObject{{"strokes", strokes}});
    }
    QJsonObject root{{"format", "toon-bm"}, {"version", 1}, {"fps", m_fps}, {"frames", frames}};
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

    QVector<Drawing> frames;
    for (const QJsonValue &fv : root.value("frames").toArray()) {
        Drawing d;
        for (const QJsonValue &sv : fv.toObject().value("strokes").toArray()) {
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
            d.strokes.append(s);
        }
        frames.append(d);
    }
    if (frames.isEmpty()) frames.append(Drawing{});

    m_frames = frames;
    m_current = 0;
    setFps(root.value("fps").toInt(12));
    emit frameCountChanged();
    emit currentFrameChanged();
    emit contentChanged();
    return true;
}
