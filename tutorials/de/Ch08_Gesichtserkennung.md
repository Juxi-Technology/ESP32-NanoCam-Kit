# Kapitel 8: Gesichtserkennung

## Funktionsprinzip

Gesichtserkennung = **Gesichtsdetektion** (MSR01+MNP01 zweistufige Pipeline) + **Merkmalsextraktion** (FaceRecognition112V1S8 MFN-Neuronales Netz) + **Kosinus-Ähnlichkeitsvergleich**.

```
RGB565-Frame der Kamera
  → MSR01 Grobdetektion (320×240, 0.3F Schwelle)
  → MNP01 Feindetektion (basierend auf den Kandidatenrahmen der Grobdetektion, 0.4F Schwelle)
  → Extraktion von 10 Gesichts-Landmarken (Augen/Nasenspitze/Mundwinkel)
  → Landmarken-Ausrichtung → Zuschnitt auf 112×112 Gesicht
  → MFN-Faltungsnetz → 512-dimensionaler Merkmalsvektor
  → L2-Normalisierung
  → Berechnung der Kosinus-Distanz zu allen registrierten ID-Vektoren im Flash
  → maximale Kosinus-Ähnlichkeit > Schwelle (0.55) → Übereinstimmung → ID ausgeben
  → alle Ähnlichkeiten < Schwelle → Fremder → "who?" ausgeben
```

### Leistungsoptimierung

MFN-Merkmalsextraktion und Vergleich mit der gesamten Datenbank sind rechenintensiv; bei Ausführung in jedem Frame stockt das Bild. Die aktuelle Implementierung verwendet eine **Frame-Skipping-Strategie**: Die Gesichtsdetektion läuft in jedem Frame (günstig), die MFN-Erkennung alle 10 Frames (teuer), und das Etikett wird mit dem letzten Erkennungsergebnis fortlaufend überlagert. So bleibt das Bild flüssig und das ID-Etikett flackert nicht.

### Speicherung der Gesichtsmerkmale

Die registrierten Gesichtsmerkmale (ID + 512-dimensionales Embedding) werden dauerhaft in der `fr`-Partition des Flash gespeichert (96 KB, maximal 47 Gesichts-IDs). Sie gehen bei Stromausfall nicht verloren.

## Hardware-Vorbereitung

- NanoCam-Kernboard + Basisboard
- USB-C-Datenkabel (Stromversorgung + serielle Verbindung zum PC)
- Seriell-Tool (Baudrate 115200)

## Schritte

### 8.1 Gesichtserkennungsmodus aktivieren

```
ai_mode:4
```

Das Gerät startet automatisch neu und wechselt in den FaceID-Modus; die WS2812-RGB-LED (GPIO18 DIN, VDD50-Versorgung) leuchtet violett. Nach dem Neustart sollte die serielle Ausgabe Folgendes zeigen:

```
I (5526) MFN: fr partition size: 98304 bytes, maxminum 47 IDs can be stored
I (5526) MFN: No face ID in flash
```

`No face ID in flash` bedeutet, dass noch kein Gesicht registriert wurde — das ist normal.

### 8.2 Gesicht registrieren

Richten Sie das Gesicht frontal auf die Kamera (Abstand 30-50cm, gleichmäßige Beleuchtung) und stellen Sie sicher, dass sich **nur ein Gesicht** im Bild befindet. Senden Sie seriell:

```
face_eril
```

Nachdem das Gerät ein Gesicht erkannt hat, extrahiert es automatisch die Merkmale und registriert sie im Flash:

```
I (xxxx) ENROLL: ID 1 is enrolled
```

Im Bild wird blauer Text `Enroll: ID 1` überlagert, der nach etwa 0.5 Sekunden verschwindet.
> **Hinweis**: Der Befehl lautet `face_eril` (Abkürzung von enroll), nicht `face_enroll`. Wenn `fail: unknown command` erscheint, prüfen Sie die Schreibweise.

### 8.3 Gesichter erkennen und unterscheiden

Nach der Registrierung senden Sie den Erkennungsbefehl:

```
face_rz
```

Das System wechselt in den fortlaufenden Erkennungsmodus. Das aktuelle Gesicht wird mit allen im Flash registrierten IDs verglichen:
- **Übereinstimmung**: Die serielle Ausgabe zeigt `Similarity: 0.85, Match ID: 1`, im Bild wird fortlaufend grünes `ID: 1` überlagert
- **Fremder**: Die serielle Ausgabe zeigt `Similarity: 0.32, Match ID: 0`, im Bild wird fortlaufend rotes `who?` überlagert
> Das Etikett wird **fortlaufend angezeigt** und verschwindet nicht. Um den Erkennungsmodus zu verlassen, senden Sie `face_detect`, um zum reinen Detektionsmodus zurückzukehren.

### 8.4 Gesicht löschen

```
face_del
```

Löscht die zuletzt registrierte Gesichts-ID; die serielle Ausgabe zeigt `N IDs left`, im Bild erscheint kurz die Anzahl der verbleibenden IDs. Die Merkmale im Flash werden ebenfalls gelöscht.

### 8.5 Erkennungsmodus verlassen

```
face_detect
```

Kehrt zum reinen Gesichtsdetektionsmodus zurück (nur Rahmen + Landmarken, keine Erkennung); die ID-Etiketten werden entfernt.
> **Zum DETECT-Modus**: Auf dem ESP32-S3 ist die serielle Koordinatenausgabe im reinen Gesichtsdetektionsmodus deaktiviert (`#if !CONFIG_IDF_TARGET_ESP32S3`), um zu verhindern, dass das serielle Log mit Detektionsmeldungen überschwemmt wird. Erst nach dem Wechsel in den Erkennungsmodus (`face_rz`) werden `detection_result`-Koordinatenlogs ausgegeben.

## Vollständige Befehlsübersicht

| Befehl | Funktion | Etiketten-Verhalten | Fortlaufend |
|---|---|---|---|
| `face_eril` | Registriert das aktuell erkannte Gesicht | Blau "Enroll: ID N" | Kurzanzeige 0.5s |
| `face_rz` | Wechselt in den fortlaufenden Erkennungsmodus | Grün "ID: N" / Rot "who?" | ✅ Fortlaufend |
| `face_del` | Löscht die zuletzt registrierte ID | Rot "N IDs left" | Kurzanzeige 0.5s |
| `face_detect` | Verlässt die Erkennung, zurück zur reinen Detektion | Entfernt alle Etiketten | — |

## Beispielhafter Ablauf

```
ai_mode:4                          # Gesichtserkennungsmodus aktivieren
[Gerät startet neu, LED violett]

face_eril                          # Erstes Gesicht registrieren (Zhang San)
→ ID 1 is enrolled

face_eril                          # Zweites Gesicht registrieren (Li Si)
→ ID 2 is enrolled

face_rz                            # Fortlaufende Erkennung starten
→ Zhang San vor der Kamera: "ID: 1" wird fortlaufend angezeigt
→ Li Si vor der Kamera: "ID: 2" wird fortlaufend angezeigt
→ Fremder vor der Kamera: "who?" wird fortlaufend angezeigt

face_detect                        # Erkennungsmodus verlassen
→ Etiketten verschwinden, nur der Detektionsrahmen wird gezeichnet

face_del                           # Li Si löschen (ID 2)
→ 1 IDs left

face_rz                            # Erneut erkennen
→ Zhang San vor der Kamera: "ID: 1"
→ Li Si vor der Kamera: "who?" (wurde gelöscht)
```

> Der Gesichtserkennungsmodus benötigt viel Speicher (MFN-Modell + Gesichtsdetektionsmodell); die Type-C-Serielle (UART0) funktioniert normal. Falls die serielle Schnittstelle nicht reagiert, prüfen Sie zunächst, ob die Baudrate auf 115200 eingestellt ist.

## Code

### Zentrale Erkennungslogik

`components/modules/ai/who_human_face_recognition.cpp` — Frame-Skipping-Erkennungsstrategie:

```cpp
case RECOGNIZE:
{
    // Frame-Skipping: Alle 10 Detektionen 1 MFN-Erkennung ausführen
    static int recog_skip = 0;
    if (recog_skip <= 0) {
        recognize_result = recognizer->recognize(
            (uint16_t *)frame->buf,
            {(int)frame->height, (int)frame->width, 3},
            detect_results.front().keypoint);
        recog_skip = 10;
    }
    recog_skip--;
    frame_show_state = SHOW_STATE_RECOGNIZE;
    break;
}
```

## Fehlerbehebung

| Symptom | Mögliche Ursache | Lösung |
|---|---|---|
| `No face ID in flash` | Normal, noch nichts registriert | `face_eril` senden, um zu registrieren |
| Erkennung liefert immer `who?` | Zu wenig Licht / falscher Winkel / Ähnlichkeit unter der Schwelle | Erneut registrieren, frontal zur Kamera, gleichmäßige Beleuchtung |
| Keine Reaktion bei der Registrierung | Gesichter im Bild ≠ 1 | Sicherstellen, dass nur ein Gesicht zu sehen ist, Abstand 30-50cm |
| Bildstocken bei der Erkennung | Normal, MFN-Inferenz benötigt Zeit | Bereits durch Frame-Skipping optimiert, läuft alle 10 Frames |
| Etikett flackert | — | Behoben, das Etikett wird fortlaufend angezeigt und verschwindet nicht |
| `fail: unknown command` | Befehl falsch geschrieben | Befehl prüfen: `face_eril`, nicht `face_enroll` |

## Ergebnis

Gesicht registrieren → fortlaufende Erkennung mit ID-Anzeige → I2C-/serielle Ausgabe der Ergebnisse → Relais/Servo steuern — eine vollständige Zutrittskontrolllösung.
