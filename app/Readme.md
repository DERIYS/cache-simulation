# Cache-Simulator CLI GUI
Dieses Programm ist eine einfache grafische Benutzeroberfläche (GUI) für die Kommandozeile zur Bedienung des Cache-Simulators.
Die Benutzeroberfläche ermöglicht die komfortable Konfiguration von Simulationseinstellungen sowie das Starten und mehrfache Ausführen der Simulation.

## Wichtige Hinweise
* Diese Anwendung verwendet das gleiche ausführbare Programm für die Simulation, das sich im Root-Verzeichnis des Projekts befindet (../../project).

* Das Verzeichnis app dient ausschließlich als grafische Repräsentation und zur Steuerung der Simulation über eine Menüführung in der Konsole.

* Durch die GUI ist es möglich, Simulationen mehrfach mit unterschiedlichen Parametern bequem nacheinander auszuführen, ohne das Programm neu starten zu müssen.

## Funktionen
* Navigierbares Menü mit den Optionen: Simulation starten, Konfiguration anpassen, Programm beenden

* Konfigurierbare Parameter wie Anzahl der Zyklen, Cache-Ebenen, Cache-Liniengröße, Latenzen, Zuordnungsstrategie, Debug-Modus

* Start der Simulation als separater Prozess mit Übergabe der gewählten Parameter

## Build-Anleitung

Um das Projekt zu bauen und auszuführen, verwenden Sie folgende Befehle:

```bash
mkdir build && cd build && cmake .. && make && ./cache_application