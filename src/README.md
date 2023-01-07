# Was zum fi.. mache ich hier, wie was bauen


## Vorbereitung

Wir entwickelen die C++ version unter Linux, weil sie auch unter Linux laufen soll.
Daher:

    1. Den Rota Ordner mit einer WSL Verbindung öffnen
    2. Wir brauchen boost daher (Debin/Ubuntu): sudo apt install libboost-all-dev
    3. Eig. sollte gcc etc schon auf der WSL laufen sonst:  sudo apt install build-essential cmake gcc-multilib

## Empfohlende Plugins

    - C/C++ (Microsoft)
    - CMake
    - CMake Integration
    - CMake Tools

## Wie bauen

    Normalerweise sollte das Plugin CMake Plugin nach dem Directory für die Cmake fragen.
    Entsprechend "src/CMakeLists.txt" auswählen.
    Mit den Plugins sollte unten eine Zeile sein mit "Build" und einen "Play" Button

### Manuell

Console im Hauptordner:

    mkdir build
    cd build
    cmake ../src/
    make

Neu bauen:

    cmake ../src/
    make

Ausführen:

    ./generator
    
## Installieren von `cpp-httplib`:

   git submodule update --init --recursive 
