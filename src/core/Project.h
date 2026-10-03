#pragma once
#include <QColor>
#include <QObject>
#include <QRectF>
#include <QString>
#include <QTimer>
#include <QVector>
#include "Drawing.h"

class Project : public QObject {
    Q_OBJECT
    Q_PROPERTY(int currentFrame READ currentFrame WRITE setCurrentFrame NOTIFY currentFrameChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)
    Q_PROPERTY(int fps READ fps WRITE setFps NOTIFY fpsChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(int currentLayer READ currentLayer WRITE setCurrentLayer NOTIFY currentLayerChanged)
    Q_PROPERTY(int layerCount READ layerCount NOTIFY layerCountChanged)
    // Incrementa a cada mudança nas camadas; use em bindings QML que chamam layerName() etc.
    Q_PROPERTY(int layersRevision READ layersRevision NOTIFY layersChanged)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
public:
    explicit Project(QObject *parent = nullptr);

    int currentFrame() const { return m_current; }
    int frameCount() const { return m_frames.size(); }
    int fps() const { return m_fps; }
    bool playing() const { return m_timer.isActive(); }
    int currentLayer() const { return m_layer; }
    int layerCount() const { return m_layers.size(); }
    int layersRevision() const { return m_layersRevision; }
    bool hasSelection() const { return m_selected >= 0; }
    const Drawing &drawing(int i) const { return m_frames.at(qBound(0, i, m_frames.size() - 1)); }

    void setCurrentFrame(int i);
    void setFps(int f);
    void setCurrentLayer(int i);

    // Frames
    Q_INVOKABLE void addFrame();
    Q_INVOKABLE void duplicateFrame();
    Q_INVOKABLE void removeFrame();
    Q_INVOKABLE void clearFrame();   // limpa a camada atual do frame atual
    Q_INVOKABLE void undo();         // desfaz o último traço da camada atual
    Q_INVOKABLE void togglePlay();

    // Camadas
    Q_INVOKABLE void addLayer();
    Q_INVOKABLE void removeLayer();
    Q_INVOKABLE void moveLayer(int from, int to);
    Q_INVOKABLE void moveLayerUp();
    Q_INVOKABLE void moveLayerDown();
    Q_INVOKABLE QString layerName(int i) const;
    Q_INVOKABLE bool layerVisible(int i) const;
    Q_INVOKABLE qreal layerOpacity(int i) const;
    Q_INVOKABLE void renameLayer(int i, const QString &name);
    Q_INVOKABLE void setLayerVisible(int i, bool v);
    Q_INVOKABLE void setLayerOpacity(int i, qreal o);

    // Desenho (na camada atual)
    Q_INVOKABLE void beginStroke(const QColor &color, qreal width, qreal x, qreal y, qreal pressure);
    Q_INVOKABLE void appendPoint(qreal x, qreal y, qreal pressure);
    Q_INVOKABLE void eraseAt(qreal x, qreal y, qreal radius);

    // Seleção (um traço da camada atual do frame atual)
    Q_INVOKABLE bool selectAt(qreal x, qreal y, qreal radius);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void moveSelected(qreal dx, qreal dy);
    Q_INVOKABLE void deleteSelected();
    Q_INVOKABLE void recolorSelected(const QColor &color);
    Q_INVOKABLE QRectF selectedBounds() const;

    // Conta-gotas: cor do traço visível mais acima no ponto, ou texto vazio se não houver.
    Q_INVOKABLE QString pickColor(qreal x, qreal y, qreal radius) const;

    Q_INVOKABLE bool save(const QString &path) const;
    Q_INVOKABLE bool load(const QString &path);

signals:
    void currentFrameChanged();
    void frameCountChanged();
    void fpsChanged();
    void playingChanged();
    void contentChanged();
    void currentLayerChanged();
    void layerCountChanged();
    void layersChanged();
    void selectionChanged();

private:
    Drawing blankDrawing() const;
    QVector<Stroke> &strokes() { return m_frames[m_current].layers[m_layer]; }
    void layersTouched();

    QVector<Drawing> m_frames;
    QVector<LayerInfo> m_layers;
    int m_current = 0;
    int m_layer = 0;
    int m_fps = 12;
    int m_layersRevision = 0;
    int m_selected = -1;
    QTimer m_timer;
};
