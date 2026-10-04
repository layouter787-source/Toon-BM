# Toon-BM — instruções para o Claude Code

Este arquivo é lido no início de cada sessão. Siga-o como especificação do projeto.

## Objetivo

Aplicativo de animação 2D profissional, **nativo para tablet Android**, com a experiência de uso próxima à do Toon Boom Harmony: quem usa o Harmony deve se sentir em casa, com interface **semelhante, não idêntica**. O dono do projeto não tem PC e hoje só tem um celular para testar. Por isso:

- Tudo que for possível verificar sem aparelho (compilação, testes automatizados, renderização headless) **deve ser verificado antes de entregar**.
- Um app que não compila ou quebra ao abrir é falha grave. Nunca deixe a `main` vermelha.

## Restrições

- Stack: C++17, Qt 6.8 (Qt Quick / QML), CMake. Scripting em JavaScript via `QJSEngine`; Python planejado.
- Não copiar código, assets, ícones ou marca do Toon Boom. "Toon Boom" e "Harmony" são marcas de terceiros e não podem aparecer no nome do produto.
- Qt: LGPL. Manter Qt como bibliotecas compartilhadas e preservar avisos de licença.
- Interface pensada para toque e caneta primeiro (alvos de toque ≥ 44 dp), mas funcional em celular, em modo paisagem.

## Estrutura

```
src/core/    modelo de dados e regras (sem dependência de UI): Project, Drawing, Stroke, LayerInfo
src/ui/      itens QML em C++: CanvasItem (palco, ferramentas, zoom/pan, seleção)
src/script/  ScriptRunner (QJSEngine, globais `project` e `console`)
qml/         interface (Main.qml)
docs/        ROADMAP.md e documentação
```

Modelo: `Project` guarda `LayerInfo` (nome, visibilidade, opacidade; valem para todos os frames) e uma lista de `Drawing` (frames). Cada `Drawing` tem um vetor de traços por camada (índice 0 = fundo). Coordenadas sempre no espaço do palco 1920x1080. Formato de arquivo `.tbm` é JSON, versão 2.

## Como compilar

Android (APK): o workflow `.github/workflows/android.yml` instala Qt 6.8.3 com `aqtinstall` (desktop `gcc_64` + `android_arm64_v8a`), NDK 26.1.10909125, e roda `cmake --build build --target apk`. Os erros de configuração e compilação são copiados para anotações da execução.

Desktop (para testes headless):

```
pip install aqtinstall
aqt install-qt linux desktop 6.8.3 linux_gcc_64 --outputdir ~/Qt
cmake -S . -B build-desktop -DCMAKE_PREFIX_PATH=~/Qt/6.8.3/gcc_64
cmake --build build-desktop
QT_QPA_PLATFORM=offscreen ctest --test-dir build-desktop --output-on-failure
```

## Regras de trabalho

1. Antes de mudar código, leia o que já existe e siga o estilo (nomes em inglês no código, comentários e textos de interface em português).
2. Mudanças pequenas e verificáveis. Cada commit deve compilar.
3. Toda regra nova em `src/core` ganha teste automatizado (QtTest) em `tests/`.
4. Compile para desktop e rode os testes antes de dar a tarefa por concluída. Só então deixe o CI do Android rodar.
5. Não declare "pronto" sem ter compilado. Se não conseguiu verificar algo, diga exatamente o quê.
6. Mantenha `docs/ROADMAP.md` e este arquivo atualizados quando a arquitetura ou o plano mudar.
7. Não edite `.github/workflows/` sem avisar o dono: ele precisa colar o arquivo manualmente se o conector não tiver a permissão Workflows.

## Primeiras tarefas

1. Criar `tests/` com QtTest para `Project`: camadas (adicionar/remover/mover), frames, desfazer, seleção, conta-gotas, e ida e volta de salvar/abrir `.tbm`. Registrar com `enable_testing()` / `add_test()` no CMake.
2. Teste de renderização headless: desenhar traços em um `QImage` pelo mesmo código de `CanvasItem` e comparar com um resultado esperado.
3. Salvar e abrir projeto pela interface (seletor de arquivo em Android).
4. Fase 3 restante: editar pontos da curva, balde de tinta, seleção múltipla, transformar, pincéis.
5. Fase 4: X-Sheet, keyframes e interpolação.

## Armadilhas já encontradas

- `QML_ELEMENT` com `qt_add_qml_module` falhou na geração do registro de tipos. O `CanvasItem` é registrado manualmente em `main.cpp` (`qmlRegisterType`, módulo `ToonBM.Core`), e o `Main.qml` faz `import ToonBM.Core 1.0`.
- Dentro do `CanvasItem`, `project: project` referencia a própria propriedade. Use `win.appProject`.
- `CanvasItem::scale()` esconde `QQuickItem::scale()`; considerar renomear para `stageScale()`.
- Em bindings QML que chamam funções do `Project` (`layerName(i)`), dependa de `project.layersRevision` para reavaliar.
- A opacidade de camada é aplicada por traço; falta composição por camada.
- Botões com `checkable` e `checked` ligado a binding perdem o binding ao clicar; use `highlighted`.

## Definição de pronto para uma fase

Compila em desktop e Android, testes passam, nada de regressão no CI, ROADMAP atualizado, e o dono recebe uma descrição curta do que testar no aparelho.
