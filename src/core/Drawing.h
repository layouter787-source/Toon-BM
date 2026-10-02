#pragma once
#include <QColor>
#include <QPointF>
#include <QString>
#include <QVector>

// Um traço: pontos no espaço do palco (1920x1080) com pressão por ponto.
struct Stroke {
    QVector<QPointF> points;
    QVector<float> pressure;
    QColor color{Qt::black};
    qreal width{6.0};
};

// Propriedades de uma camada (valem para todos os frames).
struct LayerInfo {
    QString name;
    bool visible{true};
    qreal opacity{1.0};
};

// Um desenho = conteúdo de um frame: os traços de cada camada (índice 0 = fundo).
struct Drawing {
    QVector<QVector<Stroke>> layers;
};
