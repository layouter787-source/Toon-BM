# Toon-BM

Animação 2D para **tablets**, feita para quem só tem tablet. Nome provisório.

> Projeto original. Não contém código, assets ou marca do Toon Boom. Usa uma stack parecida com a do Harmony: **C++ + Qt**, com scripting em **JavaScript** e **Python** (planejado).

## Stack

| Camada | Tecnologia | Pasta |
|---|---|---|
| Núcleo (projeto, camadas, frames, traços) | C++17 | `src/core` |
| UI, toque e caneta | Qt 6.5+ (Qt Quick) | `qml/`, `src/ui` |
| Scripting | JavaScript via `QJSEngine` (substitui o Qt Script, removido no Qt 6) | `src/script` |
| Scripting avançado | Python | planejado |
| Build | CMake + Qt para Android | `CMakeLists.txt` |

## Estado atual (v0.2)

- Palco 1920x1080 com ajuste automático à tela
- Desenho com caneta ou dedo, com pressão
- Borracha, cor, tamanho do pincel, desfazer
- **Camadas**: adicionar, remover, reordenar, renomear (toque duplo), ocultar e opacidade
- Timeline por frames: adicionar, duplicar, remover
- Onion skin (frame anterior em transparência)
- Play/pause com FPS configurável
- Salvar e abrir projeto (`.tbm`, JSON v2)
- Console de script JS: `project.addLayer()`, `project.addFrame()`, `project.fps = 12`

## Build

Qt 6.8 para Android + NDK 26, depois `qt-cmake -S . -B build && cmake --build build --target apk`.
O workflow de CI para gerar o APK pelo GitHub Actions ainda será adicionado.

## Roadmap

Veja [docs/ROADMAP.md](docs/ROADMAP.md), incluindo o checklist para venda.
