# Relatório do CI

- Commit: ce4714087699722131f14882911ffc2c5980e9f5
- Atualizado em: 2026-10-04T14:15:27Z
- Execução: https://github.com/layouter787-source/Toon-BM/actions/runs/37208461380
- Desktop (build e testes): failure
- Android (APK): success

## Etapas
- android: configurar=0 compilar_apk=0
- desktop: configurar=0 compilar_testes=0 testes=0 compilar_app=2

## Testes

## Erros

### desktop-build-app.log
```
[100%] Linking CXX executable ToonBM
/usr/bin/ld: cannot open output file ToonBM: Is a directory
collect2: error: ld returned 1 exit status
gmake[2]: *** [CMakeFiles/ToonBM.dir/build.make:325: ToonBM] Error 1
gmake[1]: *** [CMakeFiles/Makefile2:111: CMakeFiles/ToonBM.dir/all] Error 2
gmake: *** [Makefile:101: all] Error 2
```
