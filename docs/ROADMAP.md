# Roadmap

Objetivo: o fluxo de trabalho de quem usa o Harmony deve ser quase o mesmo aqui — interface semelhante, não idêntica.

1. **v0.1** (feito): palco, desenho, borracha, timeline, onion skin, play, salvar/abrir, console JS.
2. **v0.2 Camadas** (feito): várias camadas, visibilidade, ordem, renomear, opacidade, formato `.tbm` v2.
   - Limitação: a opacidade da camada é aplicada por traço; traços sobrepostos escurecem a sobreposição. Correção prevista com composição por camada.
3. **v0.3 Desenho** (em andamento)
   - Feito: suavização de traços, zoom e arrasto com dois dedos, paleta de cores, seletor HSV, conta-gotas, seleção/mover/recolorir/apagar traços.
   - Falta: editar pontos da curva, balde de tinta (preencher áreas fechadas), selecionar vários traços, transformar (girar/escalar), pincéis (lápis, tinta, marcador), salvar/abrir pelo app.
4. **Timeline avançada**: exposição de desenhos tipo X-Sheet, keyframes, interpolação, tempo de exposição por desenho.
5. **Rig e cutout**: peças, pivôs, hierarquia, deformadores.
6. **Câmera**: movimento e zoom de câmera, múltiplas câmeras.
7. **Áudio e exportação**: som, lip-sync, exportar MP4/GIF/sequência PNG.
8. **Scripting**: API JS estável e Python embutido.
9. **Funcionalidades extras (ideias)**: desfazer/refazer por histórico completo, atalhos por gestos, modo foco com interface mínima, backup automático, interpolação assistida entre desenhos.
10. **Polimento**: ícones, tema claro/escuro, tutorial inicial, desempenho, tablets e celulares.

## Antes de vender

- **Nome e marca**: "Toon Boom" e "Harmony" são marcas de terceiros. Escolher um nome próprio para o produto antes de publicar em loja.
- **Licença do Qt**: Qt é LGPL/comercial. Para vender, ou cumprir a LGPL (Qt como bibliotecas compartilhadas, aviso de licença e possibilidade de relink) ou comprar a licença comercial.
- **Loja**: conta de desenvolvedor Google Play, política de privacidade, ícone e capturas de tela; build assinado (AAB).
