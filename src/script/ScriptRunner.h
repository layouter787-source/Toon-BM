#pragma once
#include <QJSEngine>
#include <QObject>
#include <QStringList>

class Project;

// Scripting em JavaScript (QJSEngine). Globais: `project` e `console.log()`.
class ScriptRunner : public QObject {
    Q_OBJECT
public:
    explicit ScriptRunner(Project *project, QObject *parent = nullptr);

    Q_INVOKABLE QString run(const QString &code);
    Q_INVOKABLE void log(const QString &text);

private:
    QJSEngine m_engine;
    QStringList m_output;
};
