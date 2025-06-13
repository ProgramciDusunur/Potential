#!/bin/bash

echo "Potential Paralel Bench Testi"
echo "=============================="

# Potential binary'sinin varlığını kontrol et
if [ ! -f "./Potential" ]; then
    echo "Hata: Potential binary'si bulunamadı!"
    echo "Bu scripti Potential binary'sinin olduğu klasöre koyun."
    exit 1
fi

# Executable kontrolü
if [ ! -x "./Potential" ]; then
    echo "Potential binary'sine execute izni veriliyor..."
    chmod +x ./Potential
fi

echo "8 paralel Potential bench başlatılıyor..."
echo

# Paralel bench çalıştır
time bash -c "for i in {1..8}; do
    ./Potential bench &
done
wait"

echo
echo "Paralel bench tamamlandı!"
echo
echo "Eğer 0 nps sorunu yaşıyorsanız, seed'li versiyonu deneyin:"
echo "Seed'li versiyon için 's' tuşuna basın, çıkmak için Enter..."
read -n 1 choice

if [ "$choice" = "s" ] || [ "$choice" = "S" ]; then
    echo
    echo "Seed'li paralel bench başlatılıyor..."
    echo
    
    time bash -c "for i in {1..8}; do
        ./Potential bench --seed=\$i &
    done
    wait"
    
    echo
    echo "Seed'li bench tamamlandı!"
fi

echo
echo "Test bitti!"