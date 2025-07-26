# Projektbericht: Cache-Simulation in SystemC

## Kurzbeschreibung der Aufgabenstellung

Das Projekt umfasst die Entwicklung eines Cache-Simulators in SystemC, der die Funktionalität eines mehrstufigen Cache-Systems (L1, L2, L3) abbildet. Ziel ist es, Lese- und Schreibanfragen zu verarbeiten, Cache-Misses durch Abrufen von Daten aus dem Hauptspeicher zu behandeln und verschiedene Mapping-Strategien wie *Direct-Mapped* und *Fully Associative* zu unterstützen. Der Simulator soll konfigurierbare Parameter wie Cache-Größe, Zeilengröße und Latenz berücksichtigen und die Leistung des Systems analysieren. Ein Rahmenprogramm in C steuert die Simulation, indem es Eingaben aus einer CSV-Datei verarbeitet und Ergebnisse wie Zyklenanzahl, Cache-Hits und -Misses ausgibt.

## Teilnehmer-Anteile

- **Safie Emiramzaieva**: Implementierung des Haupt-Cache-Moduls (**`cache.hpp`**), Verbindung der Signale zwischen dem Hauptmodul und den Cache-Level-Modulen, Synchronisation zwischen ihnen, Anbindung an das Hauptspeichermodul, Testen, Unterstützung beim Parsen der CSV-Dateien.

- **Lev Franko**: Implementierung des Cache-Level-Moduls (**`cache_layer.hpp`**), Umsetzung der Suchalgorithmen im Cache sowie der LRU-Ersetzungsstrategie im vollassoziativen Cache, Synchronisation mit **`cache.hpp`**, Stress-Testen der gesamten Simulation, Optimierung des Codes.

- **Roman Kupar**: Implementierung des Parsens der CSV-Dateien, Berücksichtigung von Sonderfällen bei den Eingabedaten, Umsetzung des Rahmenprogramms **`main.c`**, Verarbeitung von Konsolenargumenten, automatisiertes Testen des Programms, 
Unterstützung bei der Verbindung der C- und C++-Dateien, Impelmentierung von Console-GUI-Steuerung.

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

Jede Strategie balanciert zwischen Leistung und Implementierungsaufwand, wobei LRU in unserer Simulation genutzt wird.

### Speicherzugriffsverhalten eines speicherintensiven Algorithmus

Matrixmultiplikation wurde als speicherintensiver Algorithmus analysiert. Durch **`generate_memory_requests.py`** werden CSV-Dateien mit Schreib- und Lesezugriffen erstellt, mithilfe deren die Simulation getestet werden kann. Die Ergebnisse der 30x30 Matrixmuliplikation (insgesamt 58500 Requests) sind:
- 916200 gebrauchte Taktzyklen 
- 99,7% Hit-Rate. Der hohe Prozentsatz wird durch die Mehrheit der Lesezugriffe verursacht (54900 Reads vs. 3600 Writes)
- 539% schneller als ohne Cache

### Begriffsdefinitionen

- **Cache-Levels**: Hierarchische Cache-Ebenen (L1, L2, L3) mit zunehmender Größe und Latenz, die den Speicherzugriff beschleunigen.
- **Mapping-Strategie**: Methode zur Zuordnung von Speicheradressen zu Cache-Zeilen.
- **Direct-Mapped**: Jeder Speicherblock wird genau einer Cache-Zeile zugeordnet, einfach aber konflktanfällig.
- **Fully Associative**: Jeder Block kann jeder Zeile zugeordnet werden, flexibel aber komplexer durch Replacement-Strategien.

## Kurzer Überblick über den Code

Der Code ist modular aufgebaut:
- **`cache.hpp`**: Modelliert eine Multi-Level-Cache-Hierarchie, enthält dabei Referenzen auf jede Cache-Ebene sowie Signale für das MainMemory-Modul, verbindet die entsprechenden Signale zwischen den Modulen (z.B. addr, r- und w-Flags usw.), verwaltet die Latenz und greift bei Bedarf auf den Hauptspeicher zu. Alle Cache-Ebenen, das Main-Memory und das Hauptmodul selbst laufen synchron mit demselben Clock.
- **`cache_layer.hpp`**: Modelliert eine einzelne Cache-Ebene. Es unterstützt sowohl Direct-Mapped als auch Fully-Associative Strategien und verwendet dafür C++ STL-Container zur effizienten Datenverwaltung. Jede Cachezeile wird durch ein CacheLine-Struct dargestellt, die ein Tag, ein Valid-Bit sowie einen `vector<uint8_t>` für die Daten enthält. Der Cache selbst wird als `vector<CacheLine>` gespeichert. Für Fully-Associative Caches nutzt das Modul zusätzlich eine `list<uint32_t>`, um die LRU-Reihenfolge zu verwalten, sowie ein `unordered_map<uint32_t>`, `list<uint32_t>::iterator>`, um schnelle Zuordnungen vom Tag zur LRU-Liste zu ermöglichen. All dies ermöglicht die konstante Laufzeit der Suche im Cachespeicher beim sowohl Direct-Mapped als auch Fully-Associative Cache.
- **`main_memory.hpp`**: Simuliert den Hauptspeicher, liefert eine ganze Cachezeile bei Cache-Misses, was der räumlichen Lokalität dient. Wir nutzen eine modifizierte Version des Lösungsvorschlags mit einem weiteren Output-Signal, das bei Miss in allen Cacheebenen die ganze Cachezeile beinhaltet.
- **`multiplexer.hpp`**: Steuert die Signalverteilung zwischen Cache-Ebenen. Wir benutzen den Lösungsvorschlag aus Übungsaufgaben.

## Dokumentation der Messumgebung und Ergebnisse

Die Messumgebung besteht aus einem C-Rahmenprogramm, das CLI-Optionen (z. B. `--num-cache-levels`, `--mapping-strategy`) verarbeitet und eine CSV-Datei mit Anfragen (z. B. `R,0x0010,` für Lesen) einliest. Die Simulation wird gestartet, und Ergebnisse wie Zyklen, Cache-Hits und -Misses werden in einem `Result`-Struct gespeichert und ausgegeben. Verschiedene Konfigurationen (z. B. Cache-Größe, Latenz) können getestet werden, um die Leistung zu analysieren.

## Andere nützliche Informationen (Analysen usw.)

Die Implementierung erlaubt:
- **Leistungsanalyse**: Vergleich von *Direct-Mapped* und *Fully Associative* unter verschiedenen Arbeitslasten.
- **Strategie-Optimierung**: Untersuchung der Auswirkungen von Replacement-Strategien wie LRU auf die Cache-Effizienz.
- **Realistische Simulation**: CSV-Dateien aus der Speicherzugriffsanalyse (z. B. Matrixmultiplikation) können die Simulation realistischer gestalten.

Der Simulator bietet eine flexible Plattform zur Untersuchung von Cache-Verhalten und kann für weitere Optimierungen wie beispielsweise eine smartere Ersetzungs- oder Mapping-Stratgie erweitert werden.