#!/usr/bin/env python3
"""
Простой сканер FPV сигналов
Демонстрирует базовое сканирование частот с анализом RSSI
"""

import pigpio
import time
import sys

# Конфигурация
FREQ_MIN = 5725
FREQ_MAX = 6000
FREQ_STEP = 1
RSSI_THRESHOLD = 50

# GPIO пины
CS_PIN = 8
MOSI_PIN = 10
MISO_PIN = 9
SCK_PIN = 11
RSSI_PIN = 7

class SimpleFPVScanner:
    def __init__(self):
        self.pi = None
        self.spi_handle = None
        self.running = True
        
    def init_hardware(self):
        """Инициализация оборудования"""
        print("🔧 Инициализация оборудования...")
        
        try:
            # Инициализация pigpio
            self.pi = pigpio.pi()
            if not self.pi.connected:
                print("❌ Не удалось подключиться к pigpio")
                return False
            
            # Настройка GPIO
            self.pi.set_mode(CS_PIN, pigpio.OUTPUT)
            self.pi.set_mode(MOSI_PIN, pigpio.OUTPUT)
            self.pi.set_mode(SCK_PIN, pigpio.OUTPUT)
            self.pi.set_mode(RSSI_PIN, pigpio.INPUT)
            
            # Инициализация SPI
            self.spi_handle = self.pi.spi_open(0, 2000000)
            if self.spi_handle < 0:
                print("❌ Ошибка открытия SPI")
                self.pi.stop()
                return False
            
            print("✅ Оборудование инициализировано")
            return True
            
        except Exception as e:
            print(f"❌ Ошибка инициализации: {e}")
            return False
    
    def set_frequency(self, frequency):
        """Установка частоты на RX5808"""
        try:
            # Расчет параметров для RX5808
            freq_reg = int((frequency - 479) * 8192 / 1000)
            
            # Разбивка на байты
            reg_0A = (freq_reg >> 8) & 0xFF
            reg_0B = freq_reg & 0xFF
            
            # Запись в регистры
            self.pi.write(CS_PIN, 0)
            self.pi.spi_write(self.spi_handle, [0x8A, reg_0A])  # Регистр 0A
            self.pi.write(CS_PIN, 1)
            time.sleep(0.01)
            
            self.pi.write(CS_PIN, 0)
            self.pi.spi_write(self.spi_handle, [0x8B, reg_0B])  # Регистр 0B
            self.pi.write(CS_PIN, 1)
            time.sleep(0.01)
            
            # Включение синтезатора
            self.pi.write(CS_PIN, 0)
            self.pi.spi_write(self.spi_handle, [0x8C, 0x01])  # Регистр 0C
            self.pi.write(CS_PIN, 1)
            
            # Ожидание стабилизации
            time.sleep(0.05)
            
            return True
            
        except Exception as e:
            print(f"❌ Ошибка установки частоты {frequency}: {e}")
            return False
    
    def read_rssi(self):
        """Чтение уровня RSSI"""
        try:
            # Простое цифровое чтение RSSI
            rssi_digital = self.pi.read(RSSI_PIN)
            
            # Для более точного измерения можно использовать ADC
            # Здесь используется простое преобразование
            rssi_percent = rssi_digital * 100
            
            return rssi_percent
            
        except Exception as e:
            print(f"❌ Ошибка чтения RSSI: {e}")
            return 0
    
    def scan_frequency(self, frequency):
        """Сканирование одной частоты"""
        if not self.set_frequency(frequency):
            return 0
        
        # Чтение RSSI с усреднением
        rssi_sum = 0
        samples = 10
        
        for i in range(samples):
            rssi_sum += self.read_rssi()
            time.sleep(0.01)
        
        avg_rssi = rssi_sum / samples
        return avg_rssi
    
    def scan_range(self, start_freq, end_freq):
        """Сканирование диапазона частот"""
        print(f"🔍 Сканирование диапазона {start_freq}-{end_freq} МГц...")
        
        signals_found = []
        
        for freq in range(start_freq, end_freq + 1, FREQ_STEP):
            if not self.running:
                break
                
            rssi = self.scan_frequency(freq)
            
            # Вывод статуса
            status = "📡" if rssi < RSSI_THRESHOLD else "🎯"
            print(f"{status} {freq} МГц: RSSI = {rssi:.1f}%")
            
            # Проверка на обнаружение сигнала
            if rssi > RSSI_THRESHOLD:
                signals_found.append({
                    'frequency': freq,
                    'rssi': rssi,
                    'timestamp': time.time()
                })
                print(f"🎯 СИГНАЛ ОБНАРУЖЕН: {freq} МГц, RSSI: {rssi:.1f}%")
        
        return signals_found
    
    def continuous_scan(self):
        """Непрерывное сканирование"""
        print("🔄 Непрерывное сканирование (Ctrl+C для остановки)...")
        
        try:
            while self.running:
                signals = self.scan_range(FREQ_MIN, FREQ_MAX)
                
                if signals:
                    print(f"\n📊 Обнаружено сигналов: {len(signals)}")
                    for signal in signals:
                        print(f"   {signal['frequency']} МГц: {signal['rssi']:.1f}%")
                
                print("\n⏳ Пауза 5 секунд...")
                time.sleep(5)
                
        except KeyboardInterrupt:
            print("\n⏹️ Сканирование остановлено пользователем")
            self.running = False
    
    def monitor_frequency(self, frequency, duration=0):
        """Мониторинг конкретной частоты"""
        print(f"👁️ Мониторинг частоты {frequency} МГц...")
        
        if not self.set_frequency(frequency):
            return
        
        start_time = time.time()
        last_rssi = 0
        
        try:
            while self.running:
                rssi = self.read_rssi()
                
                # Вывод изменений RSSI
                if abs(rssi - last_rssi) > 5:
                    print(f"📊 {frequency} МГц: RSSI = {rssi:.1f}%")
                    last_rssi = rssi
                
                # Проверка времени
                if duration > 0 and (time.time() - start_time) > duration:
                    break
                
                time.sleep(0.1)
                
        except KeyboardInterrupt:
            print("\n⏹️ Мониторинг остановлен")
            self.running = False
    
    def cleanup(self):
        """Очистка ресурсов"""
        print("🧹 Очистка ресурсов...")
        
        if self.spi_handle is not None:
            self.pi.spi_close(self.spi_handle)
        
        if self.pi:
            self.pi.stop()
        
        print("✅ Очистка завершена")

def show_menu():
    """Отображение меню"""
    print("\n" + "="*50)
    print("🚁 Простой FPV Сканер")
    print("="*50)
    print("1. Полное сканирование")
    print("2. Непрерывное сканирование")
    print("3. Сканирование диапазона")
    print("4. Мониторинг частоты")
    print("5. Тест оборудования")
    print("0. Выход")
    print("="*50)

def main():
    """Основная функция"""
    print("🚀 Запуск простого FPV сканера...")
    
    scanner = SimpleFPVScanner()
    
    if not scanner.init_hardware():
        print("❌ Ошибка инициализации оборудования")
        return 1
    
    try:
        while True:
            show_menu()
            choice = input("Выберите опцию: ").strip()
            
            if choice == "1":
                signals = scanner.scan_range(FREQ_MIN, FREQ_MAX)
                if signals:
                    print(f"\n🎯 Обнаружено {len(signals)} сигналов:")
                    for signal in signals:
                        print(f"   {signal['frequency']} МГц: {signal['rssi']:.1f}%")
                else:
                    print("\n📭 Сигналы не обнаружены")
            
            elif choice == "2":
                scanner.continuous_scan()
            
            elif choice == "3":
                try:
                    start = int(input(f"Начальная частота ({FREQ_MIN}-{FREQ_MAX}): "))
                    end = int(input(f"Конечная частота ({start}-{FREQ_MAX}): "))
                    
                    if FREQ_MIN <= start <= end <= FREQ_MAX:
                        signals = scanner.scan_range(start, end)
                        if signals:
                            print(f"\n🎯 Обнаружено {len(signals)} сигналов")
                        else:
                            print("\n📭 Сигналы не обнаружены")
                    else:
                        print("❌ Неверный диапазон частот")
                except ValueError:
                    print("❌ Неверный ввод")
            
            elif choice == "4":
                try:
                    freq = int(input(f"Частота для мониторинга ({FREQ_MIN}-{FREQ_MAX}): "))
                    duration = int(input("Время мониторинга в секундах (0 = бесконечно): "))
                    
                    if FREQ_MIN <= freq <= FREQ_MAX:
                        scanner.monitor_frequency(freq, duration)
                    else:
                        print("❌ Неверная частота")
                except ValueError:
                    print("❌ Неверный ввод")
            
            elif choice == "5":
                print("🔧 Тест оборудования...")
                # Простой тест
                test_freq = 5800
                if scanner.set_frequency(test_freq):
                    rssi = scanner.read_rssi()
                    print(f"✅ Тест пройден: {test_freq} МГц, RSSI: {rssi:.1f}%")
                else:
                    print("❌ Тест не пройден")
            
            elif choice == "0":
                print("👋 Завершение работы...")
                break
            
            else:
                print("❌ Неверный выбор")
    
    except KeyboardInterrupt:
        print("\n⏹️ Программа остановлена пользователем")
    
    finally:
        scanner.cleanup()
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
