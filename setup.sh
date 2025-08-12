#!/bin/bash
set -e # Прерывать выполнение при ошибке

# Название проекта
PROJECT_NAME="PLC_Sender"
# Путь к исполняемому файлу (после компиляции)
OUTPUT_BINARY="PLC_Sender"

# Список зависимостей для установки
DEPENDENCIES=(
    "build-essential"
    "qtbase5-dev"
    "qtbase5-dev-tools"
    "libqt5network5"
    "qtchooser"
    "libssh-dev"
)

# Проверка и установка зависимостей
echo "Проверка зависимостей..."
for pkg in "${DEPENDENCIES[@]}"; do
    if ! dpkg -s "$pkg" &> /dev/null; then
        echo "Устанавливаем $pkg..."
        sudo apt-get update
        sudo apt-get install -y "$pkg"
    else
        echo "$pkg уже установлен"
    fi
done

# Проверка наличия libsgp4s.so
if [ ! -f "$PWD/LIBS/Bin/libsgp4s.so" ]; then
    echo -e "\n\033[31mОшибка: libsgp4s.so не найден в $PWD/LIBS/Bin/\033[0m" >&2
    echo "Пожалуйста, убедитесь, что библиотека libsgp4s.so присутствует в $PWD/LIBS/Bin/" >&2
    exit 1
fi

# Настройка окружения для сборки
echo "Настройка Qt окружения..."
export LD_LIBRARY_PATH=$PWD/LIBS/Bin:$LD_LIBRARY_PATH

# Компиляция проекта
echo "Компиляция проекта..."
qmake "$PROJECT_NAME.pro"
make -j$(nproc)

# Проверка результата компиляции
if [ -f "$OUTPUT_BINARY" ]; then
    echo -e "\n\033[32mКомпиляция успешно завершена!\033[0m"
    echo "Исполняемый файл: $PWD/$OUTPUT_BINARY"
else
    echo -e "\n\033[31mОшибка компиляции!\033[0m" >&2
    exit 1
fi

# Копирование дополнительных файлов (для работы программы)
echo "Копирование дополнительных файлов..."
for file in TLE.txt settings.ini libsgp4s.so; do
    if [ -f "$file" ] || [ -f "$PWD/LIBS/Bin/$file" ]; then
        cp "$file" "$PWD/LIBS/Bin/$file" "$PWD/" 2>/dev/null
        echo "$file скопирован в $PWD/"
    else
        echo "Предупреждение: $file не найден"
    fi
done
