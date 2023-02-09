<p align="center">
<img src="https://i.imgur.com/voLzgMM.png" alt="drawing" width="180"/>
</p>

# GooseBay-Generator

Weitere Informationen ist unter [Dokumentation](doku/main.pdf) zu finden.

## Nutzung

Als Container ausführen:
```bash
docker-compose up
```

## Api Schnitelle

**Request Format** <br>
Alle param values im Json-Format

| Endpoint    | Method | Params                        | Description                                                  | Optimizer running |
| ----------- | ------ | ----------------------------- | ------------------------------------------------------------ | --------------- |
| getRota     | GET    | rotaCount [int]               | gibt `rotaCount` generierte Rotas zurück                     | true            |
| getRota     | GET    | pastRota  [array]             | generiert eine Fortsetzung von `pastRota`                    | false           |
| getProposal | GET    | pastRota [array], count [int] | generiert `count` Layers als Vorschlag anhand der `pastRota` | false           |

**Return Format**

``` json
{
    "status": state,
    "msg": value
}
```


## Selber Bauen und Installieren

```shell
# Alle dependencies installieren
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential cmake gcc-multilib libssl-dev wget
# Boost 1.81.0 bauen/installieren
sudo ./install_boost.sh
# submodules updaten
git submodule update --init --recursive
# build Ordner erstellen
mkdir build
cd build
# bauen
cmake ..
make
# installieren
sudo make install
```

Nach der Installation kann die Config unter  `/etc/maprota/config.json` angepasst werden.

Der API Server lässt sich dann mit
``` shell
SquadMaprotaServer host port
```
oder
``` shell
SquadMaprotaServer
```
start. Ohne `host` und `port` Angabe startet der Server unter `localhost:1330`.

----
<br>

# Admin Manual

## Quickstart

Für einen schnelle configuration kann die "config.json" anpasst werden:

-   gewünschte Anzahl an Rotas unter `number_of_rotas` einstellen
-   Layer pro Rota unter `number_of_layers` einstellen
-   Anzahl der Seed-Layer unter `seed_layer` einstellen
-   der Output-Ordner unter `output_path` eingestellen

Die Einstellung `update_layers` sollte generell auf `true` stehen, das Skript kann
eine Rota aber auch ohne die Layer zu Updaten generieren.

## Einstellparameter Übersicht

hier werden die Einstellparameter in der `config.json` und `bioms.json` erklärt

| Parameter           | Type   | Beschreibung                                                                                                                                                                          | Default                              |
| ------------------- | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------ |
| number_of_layers    | int    | Anzahl an Rotas, die generiert werden sollen                                                                                                                                          | 30                                   |
| seed_layer          | int    | Anzal an Seedlayern am begin der Rota                                                                                                                                                 | 1                                    |
| layer_vote_api_url  | string | Abruf URL der Layer/Votes Daten                                                                                                                                                       | "https://api.welovesquad.com/votes"  |
| team_api_url        | string | Abruf URL der Layer/Team Daten                                                                                                                                                        | "https://api.welovesquad.com/layers" |
| mode_distribution   | object | siehe [mode_distribution](#mode_distribution[dict])                                                                                                                                                          | -                                    |
| maps                | array  | Maps aus denen die Rota gebildet werden soll. Jede der aufgelisteten Maps sollten genau so heißen wie in der `data\bioms.json` sonst wird einen nicht übereinstimmende Map ignoriert. | -                                    |
| biom_spacing        | int    | wie lange ein Map Cluster gelocket wird                                                                                                                                               | 4                                    |
| layer_locktime      | int    | wie lange ein Layer gelocket wird                                                                                                                                                     | 30                                   |
| max_same_team       | int    | wie lange eine Faktion hintereinander gespielt werden darf                                                                                                                            | 2                                    |
| min_biom_distance   | float  | Map Cluster gebene Abstand                                                                                                                                                            | 0.35                                  |
| mapvote_slope       | float  | Slope der Mapvote Sigmoidfunktion                                                                                                                                                     | 0.15                                 |
| mapvote_shift       | float  | Shift der Mapvote Sigmoidfunktion                                                                                                                                                     | 0.0                                  |
| layervote_slope     | float  | Slope der Layervote Sigmoidfunktion                                                                                                                                                   | 0.2                                  |
| layervote_shift     | float  | Shift der Mapvote Sigmoidfunktion                                                                                                                                                     | 0.0                                  |



---

### mode_distribution [object]

**Struktur:**

**pools** [object]
Beinhaltet mindestens den **main** pool und es können beliebige pools hinzugefügt werden
Formart neuer Pools:

```json
{
 "pool_name":
      {
          "Mode1": probability (float),
          "Mode2": probability (float)
      }
}
```

**pool_distribution** [object]
Wahrscheinlichkeit für die mode pools, s.o. Struktur:

```json
{
    "pool_name1": probability (float),
    "pool_name2": probability (float)
}
```

**pool_spacing** [int]
Mindestabstand zwischen nicht "main" pool modes

**space_main** [bool]
Entscheidet, ob 2x der selbe Mode aus dem Main pool hintereinander
kommen dürfen oder sie sich abwechseln müssen.

---
<br>

## Neue Map einfügen


Um eine neue Map in die Generierung aufzunehmen, müssen der Map in `bioms.json` biom Parameter zugewiesen werden.
Außerdem muss die Map in der `config.json`  unter `maps` hinzugefügt werden.

Beispiel Struktur der `bioms.json` :

```json
{
    "Sumari": [0.0, 0.0, 0.0, 0.0, 0.8, 0.0, 1.0, 0.0, 0.3],
    "Logar": [0.03, 0.0, 0.0, 0.0, 1.0, 0.0, 0.9, 0.4, 0.2]
}
```

Jede Map hat 9 Biom Parameter, jeder Parameter bildet einen float wert in dem Biom Array in folgender Reihenfolge:

-   Mapgröße
-   Wald
-   Schnee
-   Wasser
-   Wüste
-   Grasland
-   Stadt
-   Berge
-   Felder

Jeder Map wird wird für Jedes Biom dessen Anteil mit einem Wert zwischen 0 und 1 zugewiesen.
Es handelt sich hierbei _nicht_ um eine Prozentuale Aufteilung der Map, die Summe der Biomwerte kann also 1 problemlos überschreiten (s.o. Beispielwerte).
Dabei sollte jedoch nicht nur der flächenmäßige Anteil sondern auch der Anteil am Spielgeschehen berücksichtigt werden.
Beispielsweise hat die Map 'AlBasrah' von der Fläche nur einen verhältnismäßig kleinen Stadt-Anteil.
Dieser hat jedoch (z.B. durch die Anordnung der Flaggenpunkte) einen starken Einfluss auf das Spielgeschehen,
dementsprechend sollte hier ein höherer Stadt-Anteil gewählt werden.

### Mapgröße

Die Mapgröße ist bei den Biomparametern ein Sonderfall.
Bei diesem handelt es sich nicht um den Anteil an der Map, sondern um größe der Map in km².
Hierbei ist jedoch die tatsächliche Größe des Spielbereichs zu beachten,
während die offiziellen Angaben von OWI die gesamte Mapgröße einbeziehen. Diese Unterscheiden sich zum Teil stark.
Ein prominentes Beispiel hierbei wäre die Map Chora:

<center>
<img src="./doku/manual/img/chora_example.png" alt="drawing" width="400"/>
</center>
Der tatsächliche Spielbereich, hier rot umrandet, nimmt nur einen Bruchteil der gesamten Map ein.
Die offiziell angegebene Größe bezieht jedoch die gesamte Map mit ein, was Chora zu einer der größten Karten im Spiel machen würde.
Das ergibt im Sinne dieser Einteilung natürlich wenig Sinn.
