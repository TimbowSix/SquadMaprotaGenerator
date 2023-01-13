# Was zum fi.. mache ich hier, wie was bauen


## Vorbereitung

Wir entwickelen die C++ version unter Linux, weil sie auch unter Linux laufen soll.
Daher:

    1. Den Rota Ordner mit einer WSL Verbindung öffnen
    2. Wir brauchen boost daher (Debin/Ubuntu): sudo apt install libboost-all-dev
    3. Eig. sollte gcc etc schon auf der WSL laufen sonst:  sudo apt install build-essential cmake gcc-multilib libssl-dev
    4. Install boost 1.81 mit der install Skript
    5. git submodule update --init --recursive

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

## Create Config

### Server cert

sudo mkdir /etc/maprota
sudo groupadd maprota
sudo usermod -a -G maprota $USER
su - $USER
sudo chgrp maprota /etc/maprota
sudo chmod ug+rw /etc/maprota

openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout /etc/maprota/maprotaServer.key -out /etc/maprota/maprotaServer.crt