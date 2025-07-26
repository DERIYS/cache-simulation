# Projektbericht: Cache-Simulation in SystemC

## Kurzbeschreibung der Aufgabenstellung

Das Projekt umfasst die Entwicklung eines Cache-Simulators in SystemC, der die Funktionalität eines mehrstufigen Cache-Systems abbildet. Ziel ist es, Lese- und Schreibanfragen zu verarbeiten, Cache-Misses durch Abrufen von Daten aus dem Hauptspeicher zu behandeln und verschiedene Mapping-Strategien wie *Direct-Mapped* und *Fully Associative* zu unterstützen. Der Simulator kann konfigurierbare Parameter wie Cache-Größe, Zeilengröße und Latenz berücksichtigen und die Leistung des Systems analysieren. Das Rahmenprogramm in C steuert die Simulation, indem es Eingaben aus einer CSV-Datei verarbeitet und Ergebnisse wie Zyklenanzahl, Cache-Hits und -Misses ausgibt.

## Teilnehmer-Anteile

- **Safie Emiramzaieva**: Implementierung des Haupt-Cache-Moduls (**`cache.hpp`**), Verbindung der Signale zwischen dem Hauptmodul und den Cache-Level-Modulen (**`cache_layer.hpp`**), Synchronisation zwischen ihnen, Anbindung an das Hauptspeichermodul, Testen, Unterstützung beim Parsen der CSV-Dateien.

- **Lev Franko**: Implementierung des Cache-Level-Moduls (**`cache_layer.hpp`**), Umsetzung der Suchalgorithmen im Cache sowie der LRU-Ersetzungsstrategie im vollassoziativen Cache, Synchronisation mit **`cache.hpp`**, Stress-Testen der gesamten Simulation, Optimierung des Codes.

- **Roman Kupar**: Implementierung des Parsens der CSV-Dateien, Berücksichtigung und Behandlung von Sonderfällen bei den Eingabedaten, Umsetzung des Rahmenprogramms **`main.c`**, Verarbeitung von Konsolenargumenten, automatisiertes Testen des Programms, 
Unterstützung bei der Verbindung der C- und C++-Dateien, Impelmentierung von Console-GUI-Steuerung.

## Ergebnisse der Literaturrecherche

Die Literaturrecherche beantwortet die folgenden Fragen und klärt wichtige Begriffe:

### Typische Größen für Caches, Cachezeilen und Latenzen in modernen CPUs

In modernen CPUs variieren die Cache-Parameter je nach Ebene:
- **L1 Cache**: 32 KB bis 64 KB pro Kern, Latenz von 1–5 Takten.
- **L2 Cache**: 256 KB bis 512 KB pro Kern, Latenz von 10–20 Takten.
- **L3 Cache**: 4 MB bis 64 MB, geteilt zwischen Kernen, Latenz von 20–50 Takten.
- **Cache-Zeilengröße**: Üblicherweise 64 Bytes.

Diese Werte hängen von der Architektur (z. B. Intel, AMD) ab, bieten aber eine gute Grundlage für die Simulation.

### Replacement-Strategien und ihre Vor- und Nachteile

Folgende Strategien wurden untersucht:
- **Least Recently Used (LRU)**: Entfernt die am längsten ungenutzte Zeile. Effektiv, betrachet zeitliche Lokalität, aber komplex in der Implementierung.
- **First In, First Out (FIFO)**: Entfernt die älteste Zeile. Einfacher als LRU, aber oft weniger effizient.
- **Random Replacement**: Wählt zufällig eine Zeile zum Entfernen. Sehr simpel, aber unvorhersehbar in der Leistung, verlangt einen Random-Generator.
- **Least Frequently Used (LFU)**: Entfernt die am seltensten genutzte Zeile. Effektiv, erfordert jedoch Zählmechanismen.

Jede Strategie balanciert zwischen Leistung und Implementierungsaufwand, wobei LRU in unserer Simulation genutzt wird. In modernen Caches wird meist ein Kompromiss zwischen LRU und LFU genutzt.

### Speicherzugriffsverhalten eines speicherintensiven Algorithmus

Matrixmultiplikation wurde als ein speicherintensiver Algorithmus gewählt und analysiert. Durch **`generate_memory_requests.py`** werden CSV-Dateien mit Schreib- und Lesezugriffen erstellt, um die gesamte Simulation zu testen. Die Ergebnisse der 30x30 Matrixmuliplikation (58500 Requests, davon 3600 Schreib- und 54900 Lesezugriffe​) sind:
- 916200 gebrauchte Taktzyklen 
- 99,7% Hit-Rate. Der hohe Prozentsatz wird durch die Mehrheit der Lesezugriffe verursacht.
- 539% schneller als ohne Cache

### Begriffsdefinitionen

- **Cache-Levels**: Hierarchische Cache-Ebenen (L1, L2, L3) mit zunehmender Größe und Latenz, die den Speicherzugriff beschleunigen.
- **Mapping-Strategie**: Methode zur Zuordnung von Speicheradressen zu Cache-Zeilen.
- **Direct-Mapped**: Jede Adresse wird genau einer Cache-Zeile zugeordnet, einfach und schnell aber konflktanfällig.
- **Fully Associative**: Jede Adresse kann jeder Zeile zugeordnet werden, flexibel aber komplexer durch Replacement-Strategien.

## Kurzer Überblick über den Code

Der Code ist modular aufgebaut:
- **`cache.hpp`**: Modelliert die Multi-Level-Cache-Hierarchie mit Referenzen auf alle Ebenen. Verbindet zentrale Signale (Adresse, Lese-/Schreib-Flags etc.) zwischen Cache und Hauptspeicher, verwaltet Zugriffsverzögerungen und synchronisiert alle Module über einen gemeinsamen Clock.
- **`cache_layer.hpp`**: Modelliert eine einzelne Cache-Ebene mit Unterstützung für *Direct-Mapped* und *Fully-Associative* Zuordnungsstrategien. Cachezeilen bestehen aus Tag, Valid-Bit und Daten-Vektor. Es werden STL-Container (`vector`, `list`, `unordered_map`) zur effizienten Datenverwaltung und LRU-Umsetzung mit konstanter Suche genutzt.
- **`main_memory.hpp`**: Simuliert den Hauptspeicher und liefert bei einem Miss die gesamte Cachezeile (nicht nur ein Wort), was die räumliche Lokalität verbessert. Nutzt eine erweiterte Version des Lösungsvorschlags.
- **`multiplexer.hpp`**: Steuert die Signalverteilung zwischen Cache-Ebenen. Wir benutzen den Lösungsvorschlag aus Übungsaufgaben.

## Dokumentation der Messumgebung und Ergebnisse

Die Messumgebung besteht aus einem C-Rahmenprogramm, das CLI-Optionen, die von der Aufgabenstellung beschreiben wurden, verarbeitet und eine CSV-Datei mit Anfragen einliest. Darüber hinaus werden weitere CLI-Optionen für das Debuggen und Testen unterstützt (siehe `./project --help`). Die Simulation wird gestartet, und Ergebnisse wie Zyklen, Cache-Hits und -Misses werden in einem `Result`-Struct gespeichert und ausgegeben.

## Analysen und Statistiken
Die Implementierung erlaubt:
- **Leistungsanalyse verschiedener Zuordnungsstrategien**: Bei 30x30-Matrixmultiplikation mit wenig Cachezeilen erreicht *Fully-Associative* 93% Hit-Rate, *Direct-Mapped* 64%. Mit mehr Cachezeilen gleichen sich die Unterschiede aus.
- **Strategie-Optimierung**: Untersuchung der Auswirkungen von LRU auf die Cache-Effizienz haben einen positiven Einfluss der gewählten Erstetzungsstrategie auf die Performance und Hit-Rate der Simulation gezeigt.
- **Realistische Simulation**: CSV-Dateien aus der Speicherzugriffsanalyse (z. B. Matrixmultiplikation) können die Simulation realistischer gestalten.

Der Simulator bietet eine flexible Plattform zur Untersuchung von Cache-Verhalten und kann für weitere Optimierungen wie beispielsweise eine smartere Ersetzungs- oder Mapping-Stratgie erweitert werden.