# Pacman – TODOs & Feature-Ideen

## Punkte & Scoring

> **Status:** Aktuell fest 200 Pkt. pro Geist und 50 Pkt. pro Power-Pellet (Code vs. TODO Abweichung).
> **Nächste Schritte:** 
> 1. Geist-Scoring auf Kette umstellen (200, 400, 800, 1600) via Zähler in `updateGame`.
> 2. Power-Pellet Punkte auf 30 korrigieren oder TODO anpassen.
> 3. "Level Finished" Logik implementieren (wichtig für Game-Flow).

- [ ] **Punkte für Geister fressen** – gestaffelt: 1. Geist = 200, 2. = 400, 3. = 800, 4. = 1600, wenn direkt hintereinander. Ansonsten halt 200 pro Geist
- [ ] **Punkte für normale Pellets** – bereits implementiert (10 Pkt.), ggf. anpassen
- [ ] **Punkte für Power-Pellets** – bereits 30 Pkt., ggf. anpassen
- [x] **Bonus-Objekte** – Früchte/Items mit Zeitfenster erscheinen und verschwinden (als farbige Pixel-Sprites auf dem Board platzieren, nach X Sekunden verschwinden):
- [ ] Wenn alle Punkte aufgesammelt sind, soll kurz eine Meldung erscheinen "Finished" - "Level X starts in [Countdown]" (X ist das nächste Level, Countdown ist von 3 runter zählen, bis alles wieder respawned)

---

## Game Over / Highscore

> **Status:** Grundfunktionen (Score-Anzeige, Highscore-Blinken, Feuerwerk, Persistence) sind aktiv.
> **Nächste Schritte:** Visuelles Polishing des "Highscore"-Screens (z.B. Seed-Anzeige prominenter platzieren).

- [x] Sobald GameOver ist, soll Game Over - Text, CurrentScore (präsent auf Fläche) und Seed (irgendwo in einer Ecke) sichtbar sein. D.h. auf einer Würfelseite Game Over und den CurrentScore ausgeben. Wenn neuer Highscore, dann natürlich ersichtlich machen. Auf einer anderen angrenzenden Fläche soll dann auch stehen: Again (erneut spielen mit selben Seed), New (neue Map mit neuem Seed), Play With Seed, Modus, Quit. Bei der Simulation soll automatisch nach x-Zeiteinheiten again initiiert werden.
- [x] **Score-Anzeige nach Spielende** – vor dem Neustart den aktuellen Score einige Sekunden anzeigen
- [x] **Highscore-Erkennung** – wenn neuer Highscore erreicht, visuell hervorheben (z.B. blinkende Farbe, besonderer Text oder Feuerwerk)
  - [x] Feuerwerk überarbeitet: Raketen + direkte Explosionen gemischt, alle 6 Faces, Fade-out, lebendige Farben
- [x] **Highscore persistieren** – in Datei speichern (wie in Breakout3D), beim Start laden
- [x] **Highscore-Anzeige** im Menü-Screen von Pacman dauerthaft, Ingame nicht, nur bei GameOver

---

## Player-Input

> **Status:** Pacman & Menü laufen. Geister-KI nutzt BFS/Manhattan Chase.
> **Nächste Schritte:** Frightened-KI verbessern. Aktuell bewegen sie sich zwar anders, fliehen aber nicht aktiv. Integration von `ghost-flee` Logik (Weg vom Pacman gewichten).

- [x] Pacman mit Controller steuerbar
- [x] Menü mit Controller steuerbar
- [ ] Ggf. Geister mit Controller steuerbar
- [ ] Wenn Geister blau sind, dann müssen sie auch mehr vor Pacman fliehen, da is ja verwundbar sind

---

## Seed-generierte Maps

> **Status:** `buildBoard()` ist nun seed-basiert und generiert deterministische Labyrinthe (DFS + Random Loops). Erreichbarkeit der Tiles wird via Flood-Fill sichergestellt.
> **Nächste Schritte:** 
> 1. Algorithmus verfeinern (weniger Sackgassen, bessere Pfad-Struktur).
> 2. Schwierigkeitsgrad (Sackgassen-Anzahl/Labyrinth-Dichte) an Level-Progress koppeln.

- [ ] **Prozedurale Map-Generierung** – Erster Meilenstein umgesetzt: Labyrinth wird algorithmisch generiert (DFS), garantiert keine eingesperrten Tiles. Zukünftig: Schwierigkeit an Level anpassen (Sackgassen-Steuerung).
- [x] **Seed-System** – mit demselben Seed wird dieselbe Map generiert (reproduzierbar)
- [x] **Seed über Kommandozeile übergebbar: `./Pacman --seed 12345`**
- [x] Seed wird beim Game-Over/Victory angezeigt, damit man sagen kann „mit Seed XY habe ich Highscore Z"
- [x] Play With Seed UX: Kreis-Button kehrt zurück zum Hauptmenü
- [x] Play With Seed UX: D-Pad Up/Down scrollt in 1er-Schritten, Halten beschleunigt
- [x] Play With Seed UX: D-Pad Links/Rechts scrollt in 10er-Schritten, Halten beschleunigt
- [x] Seed-Menü zeigt beim Öffnen immer den zuletzt verwendeten Seed vor
- [x] **Zufalls-Seed** bei jedem Neustart (wenn kein Seed angegeben)
- [ ] **Seed im Ready-Screen** anzeigen

---

## Pacman-Menü

> **Status:** Funktional vollständig. 

- [x] Highscore-Anzeige - kein Menüpunkt nur Anzeige (irgendwo auf dem Screen)
- [x] Play (erstellt Map mit zufälligem Seed)
- [x] Play With Seed (erstellt Map mit gegebenen Seed)
- [x] Modus (darunter kann man dann auswählen, ob mehrere Pacman vs Geister, ob ein Spieler vs andere Geister-Spieler, Default ist ein steuerbarer Pacman vs NPC-Geister) <-- vorerst eben nur ein Modus auswählbar (Default)
- [x] Demo (DEMO_MODE = true)
- [x] Quit (Beendet Applikation)

---

## Multiplayer - NOCH NICHT UMSETZEN

- [ ] **Modus 1: Mehrere Pacman vs. NPC-Geister**
  - Bis zu 4 Pacman-Spieler (verschiedene Farben zur Unterscheidung)
  - Jeder Pacman sammelt eigene Punkte, gemeinsam gewinnen sie wenn alle Pellets weg
  - Verloren wenn alle Pacman gefressen wurden
- [ ] **Modus 2: Ein Pacman vs. Geister (von Spielern gesteuert)**
  - 1 Spieler steuert Pacman, bis zu 4 Spieler steuern je einen Geist
  - Geister-Spieler müssen Pacman fangen
  - Geister-Spieler können im frightened-Modus nicht fressen → Strategie nötig
- [ ] **Farben für verschiedene Pacman** (Gelb, Grün, Lila, Türkis, …)
- [ ] **Controller-Support**
  - Joystick `/dev/input/js0..js3` – bereits teilweise vorhanden
  - Später: Handy-App via WLAN/Bluetooth als Controller (z.B. WebSocket-Server)

---

## Power-Ups (Ideen) - NOCH NICHT UMSETZEN

- [ ] **Unverwundbarkeit** – Pacman für X Sekunden unzerstörbar (blinkt weiß), Geister können ihn nicht fressen
- [ ] **Laseraugen** – Pacman schießt einen Strahl in Bewegungsrichtung, reißt die nächste Wand ein (permanente Map-Veränderung im Durchlauf)
- [ ] **Speed Boost** – Pacman doppelt so schnell für X Sekunden
- [ ] **Ghost Freeze** – alle Geister kurz eingefroren
- [ ] **Power-Up Spawn** – erscheinen zufällig auf dem Board, verschwinden nach Zeitlimit

---

## Wurmlöcher - NOCH NICHT UMSETZEN

- [ ] **Wurmloch-Paare** – je 2 Tiles auf verschiedenen Flächen sind verbunden
  - Pacman (und ggf. Geister) betritt Wurmloch → erscheint auf verbundener Fläche
  - Visuell: pulsierendes Lila/Weiß-Pixel
- [ ] **Zufällige Platzierung** pro Map-Generierung (seed-abhängig)
- [ ] **Balancing**: Geister können Wurmlöcher ggf. auch nutzen oder nicht (einstellbar)

---

## Sonstiges / Technisches

> **Status:** Wand-Kollision ist okay, aber instabil bei komplexen Kanten-Transitionen.
> **Nächste Schritte:** `canMove` Validierung schärfen, sobald prozedurale Maps laufen.

- [ ] **Wand-Kollision robuster machen** – aktuell bei Kanten-Übergängen noch gelegentliche Bugs (durch Wände gehen)
- [ ] **Sound-Support** – falls Framework Audio unterstützt, Pacman-Sounds
- [ ] **Difficulty-Level** – Geister-Geschwindigkeit und KI-Intelligenz einstellbar
- [ ] **Konfigurationsdatei** – YAML/JSON mit allen Spielparametern (FPS, Geschwindigkeit, Geisterzahl, etc.)

---

## Priorisierter Umsetzungsplan (Nächste Session)

1. **Level-Transition:** "Finished" Screen und automatischer Reset auf das nächste Level (wichtigster Game-Loop Teil).
2. **Scoring-Fixes:** Geist-Multiplikator und Power-Pellet Punkte korrigieren.
3. **Frightened-Flee:** `chooseGhostDirection` erweitern, damit Geister bei `frightenedTimer > 0` aktiv vom Pacman weglaufen (BFS Distanz maximieren).
4. **Prozedurale Maps:** Den Sprung von `buildBoard()` (statisch) zu algorithmisch wagen. Start mit einfachem Labyrinth-Algorithmus pro Fläche.
