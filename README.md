## Требования

- git [https://git-scm.com](https://git-scm.com)
- C++20 совместимый компилятор
- CMake 3.10+ [https://cmake.org/](https://cmake.org/)
- Qt 5 [https://www.qt.io/](https://www.qt.io/)
- OpenGL 3.3+
- (Опционально) Ваша любимая IDE
- (Опционально) Ninja build [https://ninja-build.org/](https://ninja-build.org/)

## Установка зависимостей (macOS)

```bash
brew install qt@5 cmake
```

## Сборка и запуск
```bash
./build_and_run.sh
```

### или вручную
```bash
mkdir -p build-release

cd build-release

cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$(brew --prefix qt@5)

make -j$(sysctl -n hw.ncpu)

./src/App/demo-app
```

# Управление
## Зум
Тачпад:
* движение двумя пальцами вниз = приблизить
* движение двумя пальцами вверх = отдалить

Мышка:
* движение колесика вниз = приблизить
* движение колесика вверх = отдалить

## Перемешение
Перемещение мыши с зажатой ЛКМ = перемещение полотна отрисовки за зажатую точку

## Изменение количества итераций
Слайдер с числовым отображением количества итераций отрисовки