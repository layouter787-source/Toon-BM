#pragma once
#include <QColor>
#include <QPointF>
#include <QVector>

// Um traço: pontos no espaço do palco (1920x1080) com pressão por ponto.
struct Stroke {
    QVector<QPointF> points;
    QVector<float> pressure;
    QColor color{Qt::black};
    qreal width{6.0};
};

// Um desenho = conteúdo de um frame.
struct Drawing {
    QVector<Stroke> strokes;
};
