#pragma once
#include <QColor>
#include <QObject>
#include <QTimer>
#include <QVector>
#include "Drawing.h"

class Project : public QObject {
    Q_OBJECT
    Q_PROPERTY(int currentFrame READ currentFrame WRITE setCurrentFrame NOTIFY currentFrameChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
public:
    explicit Project(QObject *parent = nullptr);

    int currentFrame() const { return m_current; }
    int frameCount() const { return m_frames.size(); }
    int fps() const { return m_fps; }
    bool playing() const { return m_timer.isActive(); }
    const Drawing &drawing(int i) const { return m_frames.at(qBound(0, i, m_frames.size() - 1)); }

    void setCurrentFrame(int i);
    void setFps(int f);

    Q_INVOKABLE void addFrame();
    Q_INVOKABLE void duplicateFrame();
    Q_INVOKABLE void removeFrame();
    Q_INVOKABLE void clearFrame();
    Q_INVOKABLE void undo();
    Q_INVOKABLE void togglePlay();

    Q_INVOKABLE void beginStroke(const QColor &color, qreal width, qreal x, qreal y, qreal pressure);
    Q_INVOKABLE void appendPoint(qreal x, qreal y, qreal pressure);
    Q_INVOKABLE void eraseAt(qreal x, qreal y, qreal radius);

    Q_INVOKABLE bool save(const QString &path) const;
    Q_INVOKABLE bool load(const QString &path);

signals:
    void currentFrameChanged();
    void frameCountChanged();
    void fpsChanged();
    void playingChanged();
    void contentChanged();

private:
    QVector<Drawing> m_frames;
    int m_current = 0;
    int m_fps = 12;
    QTimer m_timer;
};
