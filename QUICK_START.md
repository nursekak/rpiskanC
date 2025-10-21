# 🚀 Быстрый старт FPV Interceptor

## ⚡ Быстрая установка

```bash
# 1. Клонирование проекта
git clone <repository-url>
cd fpv-interceptor

# 2. Установка зависимостей
make install-deps

# 3. Настройка системы
make setup-system
sudo reboot

# 4. Сборка программы
make

# 5. Проверка оборудования
make test-hardware

# 6. Запуск
./fpv_interceptor.sh
```

## 🔧 Настройка pigpio

```bash
# Запуск pigpio демона
sudo systemctl enable pigpiod
sudo systemctl start pigpiod

# Проверка статуса
sudo systemctl status pigpiod
```

## 🔌 Быстрое подключение

### RX5808 → Raspberry Pi 4:
```
GND    → Pin 6 (GND)
+5V    → Pin 2 (5V)
RSSI   → Pin 26 (GPIO 7)
VIDEO  → USB Video DVR
A6.5M  → Pin 19 (GPIO 10)
CH1    → Pin 23 (GPIO 11)
CH2    → Pin 24 (GPIO 8)
ANT    → Антенна 5.8 ГГц
```

## 🎯 Основные команды

### Сканирование:
- **Полное сканирование**: Опция 1
- **Непрерывное сканирование**: Опция 2
- **Мониторинг частоты**: Опция 4

### Тестирование:
```bash
# Тест оборудования
python3 examples/test_hardware.py

# Простой сканер
python3 examples/simple_scanner.py
```

## 📊 Что делает программа

1. **Сканирует** диапазон 5725-6000 МГц
2. **Анализирует** RSSI сигналы
3. **Обнаруживает** FPV видеосигналы
4. **Захватывает** видеопотоки
5. **Сохраняет** данные

## 🔧 Устранение проблем

### pigpio не работает:
```bash
# Запуск демона
sudo systemctl start pigpiod

# Проверка статуса
sudo systemctl status pigpiod

# Проверка подключения
python3 -c "import pigpio; pi = pigpio.pi(); print('OK' if pi.connected else 'ERROR'); pi.stop()"
```

### SPI не работает:
```bash
echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
sudo reboot
```

### Видео не захватывается:
```bash
ls /dev/video*
lsusb
```

### Слабый сигнал:
- Проверьте антенну
- Проверьте заземление
- Попробуйте другую позицию

## 📈 Результаты

Программа автоматически:
- Обнаруживает сигналы с RSSI > 50%
- Анализирует характеристики FPV
- Захватывает видеопотоки
- Сохраняет данные в файлы

---

**🎯 Готово! Начинайте перехват FPV сигналов!**
