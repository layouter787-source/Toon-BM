#include "ScriptRunner.h"
#include "core/Project.h"

ScriptRunner::ScriptRunner(Project *project, QObject *parent) : QObject(parent) {
    QJSEngine::setObjectOwnership(project, QJSEngine::CppOwnership);
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);
    m_engine.globalObject().setProperty("project", m_engine.newQObject(project));
    m_engine.globalObject().setProperty("console", m_engine.newQObject(this));
}

void ScriptRunner::log(const QString &text) {
    m_output << text;
}

QString ScriptRunner::run(const QString &code) {
    m_output.clear();
    const QJSValue result = m_engine.evaluate(code);
    if (result.isError()) {
        return QStringLiteral("Erro (linha %1): %2")
            .arg(result.property("lineNumber").toInt())
            .arg(result.toString());
    }
    if (!result.isUndefined()) m_output << result.toString();
    return m_output.join(QLatin1Char('\n'));
}
