#include <QColor>
#include <QPointF>
#include <QTemporaryDir>
#include <QtTest>
#include "core/Project.h"

namespace {
// Desenha uma linha reta de dois pontos na camada atual do frame atual.
void drawLine(Project &p, const QColor &c, const QPointF &a, const QPointF &b, qreal width = 4.0) {
    p.beginStroke(c, width, a.x(), a.y(), 1.0);
    p.appendPoint(b.x(), b.y(), 1.0);
}
}

class TstProject : public QObject {
    Q_OBJECT
private slots:
    void initialState();
    void layersAddRemove();
    void layerMove();
    void strokesGoToCurrentLayer();
    void undoRemovesLastStroke();
    void eraseHitsMiddleOfSegment();
    void selectMoveRecolorDelete();
    void pickColorTopmostVisible();
    void framesAddDuplicateRemove();
    void saveLoadRoundTrip();
};

void TstProject::initialState() {
    Project p;
    QCOMPARE(p.frameCount(), 1);
    QCOMPARE(p.layerCount(), 1);
    QCOMPARE(p.currentFrame(), 0);
    QCOMPARE(p.currentLayer(), 0);
    QCOMPARE(p.fps(), 12);
    QVERIFY(!p.playing());
    QVERIFY(!p.hasSelection());
}

void TstProject::layersAddRemove() {
    Project p;
    p.addLayer();
    QCOMPARE(p.layerCount(), 2);
    QCOMPARE(p.currentLayer(), 1);
    QCOMPARE(p.layerName(1), QStringLiteral("Camada 2"));

    p.removeLayer();
    QCOMPARE(p.layerCount(), 1);
    QCOMPARE(p.currentLayer(), 0);

    // A última camada nunca é removida.
    p.removeLayer();
    QCOMPARE(p.layerCount(), 1);
}

void TstProject::layerMove() {
    Project p;
    p.addLayer();
    p.addLayer();
    p.renameLayer(0, QStringLiteral("A"));
    p.renameLayer(1, QStringLiteral("B"));
    p.renameLayer(2, QStringLiteral("C"));
    p.setCurrentLayer(0);

    p.moveLayerUp();   // A sobe: B, A, C
    QCOMPARE(p.layerName(0), QStringLiteral("B"));
    QCOMPARE(p.layerName(1), QStringLiteral("A"));
    QCOMPARE(p.layerName(2), QStringLiteral("C"));
    QCOMPARE(p.currentLayer(), 1);

    p.moveLayerDown(); // A desce: A, B, C
    QCOMPARE(p.layerName(0), QStringLiteral("A"));
    QCOMPARE(p.layerName(1), QStringLiteral("B"));
    QCOMPARE(p.currentLayer(), 0);
}

void TstProject::strokesGoToCurrentLayer() {
    Project p;
    p.addLayer(); // camada 1 é a atual
    drawLine(p, Qt::red, QPointF(10, 10), QPointF(20, 20));
    QCOMPARE(int(p.drawing(0).layers[1].size()), 1);
    QCOMPARE(int(p.drawing(0).layers[0].size()), 0);

    p.setCurrentLayer(0);
    drawLine(p, Qt::blue, QPointF(10, 10), QPointF(20, 20));
    QCOMPARE(int(p.drawing(0).layers[0].size()), 1);
    QCOMPARE(int(p.drawing(0).layers[1].size()), 1);
}

void TstProject::undoRemovesLastStroke() {
    Project p;
    drawLine(p, Qt::red, QPointF(0, 0), QPointF(10, 0));
    drawLine(p, Qt::red, QPointF(0, 5), QPointF(10, 5));
    QCOMPARE(int(p.drawing(0).layers[0].size()), 2);
    p.undo();
    QCOMPARE(int(p.drawing(0).layers[0].size()), 1);
    p.undo();
    p.undo(); // não deve falhar com a camada vazia
    QCOMPARE(int(p.drawing(0).layers[0].size()), 0);
}

void TstProject::eraseHitsMiddleOfSegment() {
    Project p;
    drawLine(p, Qt::black, QPointF(0, 0), QPointF(100, 0), 4.0);

    p.eraseAt(50, 50, 2.0); // longe do traço
    QCOMPARE(int(p.drawing(0).layers[0].size()), 1);

    p.eraseAt(50, 3, 2.0);  // perto do meio do segmento, sem tocar em nenhum ponto
    QCOMPARE(int(p.drawing(0).layers[0].size()), 0);
}

void TstProject::selectMoveRecolorDelete() {
    Project p;
    drawLine(p, Qt::red, QPointF(10, 10), QPointF(20, 10));

    QVERIFY(!p.selectAt(500, 500, 5.0));
    QVERIFY(!p.hasSelection());

    QVERIFY(p.selectAt(15, 12, 5.0));
    QVERIFY(p.hasSelection());
    QVERIFY(!p.selectedBounds().isNull());

    p.moveSelected(100, 50);
    QCOMPARE(p.drawing(0).layers[0][0].points[0], QPointF(110, 60));
    QCOMPARE(p.drawing(0).layers[0][0].points[1], QPointF(120, 60));

    p.recolorSelected(QColor("#00ff00"));
    QCOMPARE(p.drawing(0).layers[0][0].color.name(), QStringLiteral("#00ff00"));

    p.deleteSelected();
    QVERIFY(!p.hasSelection());
    QCOMPARE(int(p.drawing(0).layers[0].size()), 0);
}

void TstProject::pickColorTopmostVisible() {
    Project p;
    drawLine(p, Qt::red, QPointF(10, 10), QPointF(20, 10));
    p.addLayer();
    drawLine(p, Qt::blue, QPointF(10, 10), QPointF(20, 10));

    QCOMPARE(p.pickColor(15, 10, 3.0), QStringLiteral("#0000ff")); // camada de cima

    p.setLayerVisible(1, false);
    QCOMPARE(p.pickColor(15, 10, 3.0), QStringLiteral("#ff0000")); // camada oculta é ignorada

    QCOMPARE(p.pickColor(500, 500, 3.0), QString()); // nada no ponto
}

void TstProject::framesAddDuplicateRemove() {
    Project p;
    drawLine(p, Qt::red, QPointF(0, 0), QPointF(10, 0));

    p.addFrame();
    QCOMPARE(p.frameCount(), 2);
    QCOMPARE(p.currentFrame(), 1);
    QCOMPARE(int(p.drawing(1).layers[0].size()), 0);

    p.setCurrentFrame(0);
    p.duplicateFrame();
    QCOMPARE(p.frameCount(), 3);
    QCOMPARE(p.currentFrame(), 1);
    QCOMPARE(int(p.drawing(1).layers[0].size()), 1);

    p.removeFrame();
    QCOMPARE(p.frameCount(), 2);
    QVERIFY(p.currentFrame() >= 0 && p.currentFrame() < p.frameCount());
}

void TstProject::saveLoadRoundTrip() {
    Project a;
    a.setFps(24);
    a.addLayer();
    a.renameLayer(0, QStringLiteral("Fundo"));
    a.setLayerOpacity(1, 0.5);
    a.setLayerVisible(1, false);
    a.setCurrentLayer(0);
    a.beginStroke(QColor("#336699"), 8.0, 1, 2, 0.5);
    a.appendPoint(3, 4, 0.7);
    a.appendPoint(5, 6, 0.9);
    a.addFrame();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("teste.tbm"));
    QVERIFY(a.save(path));

    Project b;
    QVERIFY(b.load(path));
    QCOMPARE(b.fps(), 24);
    QCOMPARE(b.layerCount(), 2);
    QCOMPARE(b.frameCount(), 2);
    QCOMPARE(b.layerName(0), QStringLiteral("Fundo"));
    QVERIFY(qFuzzyCompare(b.layerOpacity(1), 0.5));
    QVERIFY(!b.layerVisible(1));

    const Stroke &s = b.drawing(0).layers[0][0];
    QCOMPARE(int(s.points.size()), 3);
    QCOMPARE(s.color.name(), QStringLiteral("#336699"));
    QCOMPARE(s.points[2], QPointF(5, 6));
    QVERIFY(qFuzzyCompare(s.pressure[1], 0.7f));

    // Arquivo inexistente não pode abrir.
    QVERIFY(!b.load(dir.filePath(QStringLiteral("nao-existe.tbm"))));
}

QTEST_GUILESS_MAIN(TstProject)
#include "tst_project.moc"
