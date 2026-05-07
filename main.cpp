#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>   // для int16_t
#include <algorithm> // для алгоритмів STL (std::transform, std::reverse)
#include <cmath>     // для std::sin

// Зчитування RAW файлу у вектор
std::vector<int16_t> readAudio(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Помилка відкриття файлу: " << filename << std::endl;
        return {};
    }
    
    // Переходимо в кінець файлу, щоб дізнатися його розмір
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Створюємо вектор відповідного розміру (кількість семплів = байт / 2)
    std::vector<int16_t> buffer(size / sizeof(int16_t));
    
    // Зчитуємо дані
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

// Запис змінених даних назад у файл
void writeAudio(const std::string& filename, const std::vector<int16_t>& buffer) {
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(int16_t));
}

int main() {
    // 1. Вичитати дані у вектор
    std::string inputFile = "audio.raw";
    std::vector<int16_t> originalData = readAudio(inputFile);
    
    if (originalData.empty()) {
        return 1; // Завершуємо програму, якщо файл не знайдено
    }

    std::cout << "Файл успішно прочитано. Починаємо обробку..." << std::endl;

    // --- ЕФЕКТ 1: Збільшення гучності ---
    // Завдання: Помножити кожен з елементів на 2[cite: 73].
    std::vector<int16_t> volumeData = originalData;
    std::transform(volumeData.begin(), volumeData.end(), volumeData.begin(), 
                   [](int16_t sample) { 
                       // Використовуємо множення, але слідкуємо, щоб не було переповнення типу int16_t
                       int32_t val = static_cast<int32_t>(sample) * 2;
                       if (val > 32767) return static_cast<int16_t>(32767);
                       if (val < -32768) return static_cast<int16_t>(-32768);
                       return static_cast<int16_t>(val); 
                   });
    writeAudio("volume_up.raw", volumeData);

    // --- ЕФЕКТ 2: Реверс ---
    // Завдання: Реверс запису (Записати файл задом наперед)[cite: 74].
    std::vector<int16_t> reverseData = originalData;
    std::reverse(reverseData.begin(), reverseData.end()); // STL алгоритм реверсу
    writeAudio("reverse.raw", reverseData);

    // --- ЕФЕКТ 3: Bit-crushing ---
    // Завдання: Зменшення розрядності за формулою (sample >> 12) << 12[cite: 75].
    std::vector<int16_t> bitcrushData = originalData;
    std::transform(bitcrushData.begin(), bitcrushData.end(), bitcrushData.begin(), 
                   [](int16_t sample) { return (sample >> 12) << 12; });
    writeAudio("bitcrush.raw", bitcrushData);

    // --- ЕФЕКТ 4: Distortion (Кліппінг) ---
    // Завдання: Обрізаємо піки сигналу[cite: 77]. 
    // if (значення > поріг) return поріг; if (значення < -поріг) return -поріг[cite: 78, 79].
    std::vector<int16_t> distortionData = originalData;
    int16_t threshold = 10000; // Встановлюємо поріг (максимум для int16_t це 32767)
    std::transform(distortionData.begin(), distortionData.end(), distortionData.begin(), 
                   [threshold](int16_t sample) { 
                       if (sample > threshold) return threshold;
                       if (sample < -threshold) return static_cast<int16_t>(-threshold);
                       return sample;
                   });
    writeAudio("distortion.raw", distortionData);

    // --- ЕФЕКТ 5: Tremolo ---
    // Завдання: Множимо кожну амплітуду на значення синусоїди, що змінюється[cite: 80].
    // Формула: mod = 0.75 + 0.25 * std::sin(i * freq); data *= mod[cite: 81, 82].
    std::vector<int16_t> tremoloData = originalData;
    double freq = 0.0005; // Частота модуляції (можна підбирати на слух)
    int i = 0; // Лічильник для індексу
    std::transform(tremoloData.begin(), tremoloData.end(), tremoloData.begin(), 
                   [&i, freq](int16_t sample) { 
                       double mod = 0.75 + 0.25 * std::sin(i * freq);
                       i++;
                       return static_cast<int16_t>(sample * mod);
                   });
    writeAudio("tremolo.raw", tremoloData);

    // --- ЕФЕКТ 6: Ring Modulator ---
    // Завдання: Інвертуємо знак кожного другого семпла[cite: 83].
    // Примітка: В інструкції є описка "if (i % 20)"[cite: 83]. За логікою фрази "кожного другого семпла"[cite: 83], має бути i % 2 == 0.
    std::vector<int16_t> ringModData = originalData;
    int j = 0;
    std::transform(ringModData.begin(), ringModData.end(), ringModData.begin(), 
                   [&j](int16_t sample) { 
                       if (j % 2 == 0) { 
                           sample = -sample;
                       }
                       j++;
                       return sample;
                   });
    writeAudio("ring_modulator.raw", ringModData);

    std::cout << "Всі файли успішно згенеровано!" << std::endl;
    return 0;
}