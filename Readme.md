Teilnehmer Anteile:
- Safie Emiramzaieva: Implementierung des Haupt-Cache-Moduls (cache.hpp), Verbindung der Signale zwischen dem Hauptmodul und den Cache-Level-Modulen, Synchronisation zwischen ihnen, Anbindung an das Hauptspeichermodul, Testen, Unterstützung beim Parsen der CSV-Dateien.

- Lev Franko: Implementierung des Cache-Level-Moduls (cache_layer.hpp), Umsetzung der Adress-Suchalgorithmen im Cache sowie der Ersetzungsstrategie (Replacement Policy) LRU im vollassoziativen Cache, Synchronisation mit cache.hpp, Testen.

- Roman Kupar: Implementierung des Parsens der CSV-Dateien, Berücksichtigung von Sonderfällen bei den Eingabedaten, Umsetzung des Rahmenprogramms main.c, Verarbeitung von Konsolenargumenten, automatisiertes Testen des Programms.

Ergebnisse der Literaturrecherche:
Erklaerung der Begriffe:
1. Cache‑Levels: Mehrstufige Organisation (L1, L2, L3 …), wobei L1 clevere Nutzung, minimale Latenz; L2/L3 zunehmend größer und langsamer, oft geteilte Reservoire
2. Mapping‑Strategie: Legt fest, wie Speicheradresse einer Cache-Zeile zugeordnet wird:
    - Direct‑mapped: Jede Adresse hat genau einen möglichen Platz, einfach, aber mehr Konflikt-Misses.
    - Fully associative: Adresse kann überall landen. Minimaler Konflikt, aber teuer und langsam bei großer Cache. Nur bei kleinen Caches effizient.
Antworten auf Fragen:
1. Typische Größen & Latenzen in modernen CPUs:
    - L1: 32-128KB pro Kern.
    - L2: 256KB-2MB pro Kern.
    - L3: 4-64MB gemeinsam genutzt.
    Cache-Linien-Größe: heute meistens 64 Bytes.
    Latenzen: 
    - L1: ca. 1–4 Zyklen.
    - L2: 7–14 Zyklen
    - L3: 20–40 Zyklen.
2. Replacement-Strategien: Vor- & Nachteile
    - LRU: 
        + Bevorzugt kürzlich genutzte Daten, gut bei temporal locality, 
        - Overhead für Tracking, ineffizient bei Streaming
    - LFU: 
        + Hält häufig genutzte Daten
        - Träge bei Musterwechsel; hoher Verwaltungsaufwand
    - FIFO: 
        + Einfache Umsetzung
        - Ignoriert Nutzungsmuster, meist schlechtere Trefferquote
    - Random Replacement: 
        + Sehr geringer Overhead, simpel
        - Zufällig, geringe Vorhersagbarkei
3. Untersuchung eines speicherintensiven Algorithmus
    Für die Analyse des Speicherzugriffsverhaltens haben wir die Matrixmultiplikation gewählt, da sie ein typischer speicherintensiver Algorithmus ist. Sie verursacht zahlreiche Lese- und Schreibzugriffe auf große Datenmengen und eignet sich daher gut zur Untersuchung von Speicherverhalten, insbesondere im Bezug auf die Nutzung des Caches, denn dies beschleunigt die Ausführungszeit eines solchen Algorithmes deutlich.

- Kurzer Überblick über Code:
    Unsere Programmlogik besteht aus drei SystemC-Modulen. Das Hauptmodul CACHE modelliert eine Multi-Level-Cache-Hierarchie, enthält dabei Referenzen auf jede Cache-Ebene sowie auf das Main-Memory-Modul, verbindet die entsprechenden Signale zwischen den Modulen (z.B. addr, r- und w-Flags usw.), verwaltet die Latenz und greift bei Bedarf auf den Hauptspeicher zu. Alle Cache-Ebenen, das Main-Memory und das Hauptmodul selbst laufen synchron mit demselben Clock.

    Das CACHE_LAYER-SystemC-Modul modelliert eine einzelne Cache-Ebene. Es unterstützt sowohl direct-mapped als auch fully-associative Strategien und verwendet dafür C++ STL-Container zur effizienten Datenverwaltung. Jede Cache-Line wird durch eine CacheLine-Struktur dargestellt, die ein Tag, ein Valid-Bit sowie einen std::vector<uint8_t> für die Daten enthält. Der Cache selbst wird als std::vector<CacheLine> gespeichert. Für fully-associative Caches nutzt das Modul zusätzlich eine std::list<uint32_t>, um die LRU-Reihenfolge (Least Recently Used) zu verwalten, sowie ein std::unordered_map<uint32_t, std::list<uint32_t>::iterator>, um schnelle Zuordnungen vom Tag zur LRU-Liste zu ermöglichen.

# Projektbericht: Cache-Simulation in SystemC

## Kurzbeschreibung der Aufgabenstellung

Das Projekt umfasst die Entwicklung eines Cache-Simulators in SystemC, der die Funktionalität eines mehrstufigen Cache-Systems (L1, L2, L3) abbildet. Ziel ist es, Lese- und Schreibanfragen zu verarbeiten, Cache-Misses durch Abrufen von Daten aus dem Hauptspeicher zu behandeln und verschiedene Mapping-Strategien wie *Direct-Mapped* und *Fully Associative* zu unterstützen. Der Simulator soll konfigurierbare Parameter wie Cache-Größe, Zeilengröße und Latenz berücksichtigen und die Leistung des Systems analysieren. Ein Rahmenprogramm in C steuert die Simulation, indem es Eingaben aus einer CSV-Datei verarbeitet und Ergebnisse wie Zyklenanzahl, Cache-Hits und -Misses ausgibt.

## Teilnehmer-Anteile

- **Safie Emiramzaieva**: Implementierung des Haupt-Cache-Moduls (**'cache.hpp'**), Verbindung der Signale zwischen dem Hauptmodul und den Cache-Level-Modulen, Synchronisation zwischen ihnen, Anbindung an das Hauptspeichermodul, Testen, Unterstützung beim Parsen der CSV-Dateien.

- **Lev Franko**: Implementierung des Cache-Level-Moduls (**'cache_layer.hpp'**), Umsetzung der Adress-Suchalgorithmen im Cache sowie der Ersetzungsstrategie (Replacement Policy) LRU im vollassoziativen Cache, Synchronisation mit cache.hpp, Testen.

- **Roman Kupar**: Implementierung des Parsens der CSV-Dateien, Berücksichtigung von Sonderfällen bei den Eingabedaten, Umsetzung des Rahmenprogramms main.c, Verarbeitung von Konsolenargumenten, automatisiertes Testen des Programms.

## Ergebnisse der Literaturrecherche

Die Literaturrecherche beantwortet die folgenden Fragen und klärt wichtige Begriffe:

### Typische Größen für Caches, Cachezeilen und Latenzen in modernen CPUs

In modernen CPUs variieren die Cache-Parameter je nach Ebene:
- **L1 Cache**: 32 KB bis 64 KB pro Kern, Latenz von 1–5 Takten.
- **L2 Cache**: 256 KB bis 512 KB pro Kern, Latenz von 10–20 Takten.
- **L3 Cache**: 4 MB bis 64 MB, geteilt zwischen Kernen, Latenz von 20–50 Takten.
- **Cache-Zeilengröße**: Üblicherweise 64 Bytes.

Diese Werte hängen von der Architektur (z. B. Intel, AMD) ab, bieten jedoch eine gute Grundlage für die Simulation.

### Replacement-Strategien und ihre Vor- und Nachteile

Folgende Strategien wurden untersucht:
- **Least Recently Used (LRU)**: Entfernt die am längsten ungenutzte Zeile. Effektiv, aber komplex in der Implementierung.
- **First In, First Out (FIFO)**: Entfernt die älteste Zeile. Einfacher als LRU, aber oft weniger effizient.
- **Random Replacement**: Wählt zufällig eine Zeile. Sehr simpel, aber unvorhersehbar in der Leistung.
- **Least Frequently Used (LFU)**: Entfernt die am seltensten genutzte Zeile. Effektiv, erfordert jedoch Zählmechanismen.

Jede Strategie balanciert zwischen Leistung und Implementierungsaufwand, wobei LRU in unserer Simulation bevorzugt wird.

### Speicherzugriffsverhalten eines speicherintensiven Algorithmus

Matrixmultiplikation wurde als speicherintensiver Algorithmus analysiert. Durch Simulation könnten CSV-Dateien mit Zugriffen (z. B. Lesezugriff auf Adresse 0x0010, Schreibzugriff mit Wert 20) erstellt werden, die das Zusammenspiel mit dem Cache veranschaulichen.

### Begriffsdefinitionen

- **Cache-Levels**: Hierarchische Cache-Ebenen (L1, L2, L3) mit zunehmender Größe und Latenz, die den Speicherzugriff beschleunigen.
- **Mapping-Strategie**: Methode zur Zuordnung von Speicheradressen zu Cache-Zeilen.
- **Direct-Mapped**: Jeder Speicherblock wird genau einer Cache-Zeile zugeordnet, einfach aber konflktanfällig.
- **Fully Associative**: Jeder Block kann jeder Zeile zugeordnet werden, flexibel aber komplexer durch Replacement-Strategien.

## Kurzer Überblick über den Code

Der Code ist modular aufgebaut:
- **`cache.hpp`**: Hauptmodul, das Cache-Ebenen, Hauptspeicher und Multiplexer integriert. Es verarbeitet Lese- und Schreibanfragen und managed Cache-Misses.
- **`cache_layer.hpp`**: Verwaltet einzelne Cache-Ebenen mit Unterstützung für *Direct-Mapped* und *Fully Associative*. Es implementiert Zugriffslogik und LRU für *Fully Associative*.
- **`main_memory.hpp`**: Simuliert den Hauptspeicher, liefert Daten bei Cache-Misses.
- **`multiplexer.hpp`**: Steuert die Signalverteilung zwischen Cache-Ebenen.

SystemC ermöglicht eine hardwareähnliche Modellierung, wobei die Simulation durch Taktsignale gesteuert wird.

## Dokumentation der Messumgebung und Ergebnisse

Die Messumgebung besteht aus einem C-Rahmenprogramm, das CLI-Optionen (z. B. `--num-cache-levels`, `--mapping-strategy`) verarbeitet und eine CSV-Datei mit Anfragen (z. B. `R,0x0010,` für Lesen) einliest. Die Simulation wird gestartet, und Ergebnisse wie Zyklen, Cache-Hits und -Misses werden in einem `Result`-Struct gespeichert und ausgegeben. Verschiedene Konfigurationen (z. B. Cache-Größe, Latenz) können getestet werden, um die Leistung zu analysieren.

## Andere nützliche Informationen (Analysen usw.)

Die Implementierung erlaubt:
- **Leistungsanalyse**: Vergleich von *Direct-Mapped* und *Fully Associative* unter verschiedenen Arbeitslasten.
- **Strategie-Optimierung**: Untersuchung der Auswirkungen von Replacement-Strategien wie LRU auf die Cache-Effizienz.
- **Realistische Simulation**: CSV-Dateien aus der Speicherzugriffsanalyse (z. B. Matrixmultiplikation) können die Simulation realistischer gestalten.

Der Simulator bietet eine flexible Plattform zur Untersuchung von Cache-Verhalten und kann für weitere Optimierungen erweitert werden.