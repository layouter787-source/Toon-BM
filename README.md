# Toon-BM

Animação 2D para **tablets**, inspirada no fluxo de trabalho do Toon Boom Harmony e feita para quem só tem tablet.

> Projeto original. Não contém código, assets ou marca do Toon Boom. Usa a mesma stack do Harmony: **C++ + Qt**, com scripting em **JavaScript** e **Python** (planejado).

## Stack

| Camada | Tecnologia | Pasta |
|---|---|---|
| Núcleo (projeto, frames, traços) | C++17 | `src/core` |
| UI, toque e caneta | Qt 6.5+ (Qt Quick) | `qml/`, `src/ui` |
| Scripting | JavaScript via `QJSEngine` (substitui o Qt Script, removido no Qt 6) | `src/script` |
| Scripting avançado | Python | planejado |
| Build | CMake + Qt para Android, APK pelo GitHub Actions | `.github/workflows` |

## Estado atual (v0.1)

- Palco 1920x1080 com ajuste automático à tela
- Desenho com caneta ou dedo, com pressão
- Borracha, cor, tamanho do pincel, desfazer
- Timeline por frames: adicionar, duplicar, remover
- Onion skin (frame anterior em transparência)
- Play/pause com FPS configurável
- Salvar e abrir projeto (`.tbm`, JSON)
- Console de script JS: `project.addFrame()`, `project.fps = 12`

## Gerar o APK sem PC

1. Aba **Actions** > *Android APK* > *Run workflow* (ou faça push na `main`).
2. Ao terminar, baixe o artefato `ToonBM-apk` pelo navegador do tablet.
3. Instale o `.apk`.

Local: Qt 6.8 para Android + NDK 26, depois `qt-cmake -S . -B build && cmake --build build --target apk`.

## Roadmap

Veja [docs/ROADMAP.md](docs/ROADMAP.md).
