# AWK Interpreter Benutzerhandbuch

**Version 1.0**

---

## Inhaltsverzeichnis

1. [Einführung](#1-einführung)
2. [Erste Schritte](#2-erste-schritte)
3. [AWK-Programmstruktur](#3-awk-programmstruktur)
4. [Muster (Patterns)](#4-muster-patterns)
5. [Aktionen](#5-aktionen)
6. [Variablen](#6-variablen)
7. [Operatoren](#7-operatoren)
8. [Kontrollstrukturen](#8-kontrollstrukturen)
9. [Arrays](#9-arrays)
10. [Funktionen](#10-funktionen)
11. [Ein- und Ausgabe](#11-ein--und-ausgabe)
12. [Reguläre Ausdrücke](#12-reguläre-ausdrücke)
13. [Eingebaute Funktionen](#13-eingebaute-funktionen)
14. [Spezielle Variablen](#14-spezielle-variablen)
15. [gawk-Erweiterungen](#15-gawk-erweiterungen)
16. [Beispiele](#16-beispiele)
17. [Fehlerbehebung](#17-fehlerbehebung)

---

## 1. Einführung

### Was ist AWK?

AWK ist eine leistungsstarke Programmiersprache zur Textverarbeitung, die für Mustererkennung und Datenverarbeitung entwickelt wurde. AWK eignet sich hervorragend für:

- Extraktion von Feldern aus strukturierten Daten
- Transformation von Textdateien
- Erstellung von Berichten
- Verarbeitung von Protokolldateien
- Schnelle Datenanalyse

### Über diesen Interpreter

Dieser AWK-Interpreter ist eine vollständige Implementierung in C++17 mit folgenden Eigenschaften:

- Vollständige POSIX-AWK-Konformität
- Umfangreiche gawk (GNU AWK) Erweiterungen
- Plattformübergreifende Unterstützung (Windows, Linux, macOS)
- Über 50 eingebaute Funktionen

---

## 2. Erste Schritte

### AWK-Programme ausführen

Es gibt drei Möglichkeiten, AWK-Programme auszuführen:

**1. Programm auf der Kommandozeile:**
```bash
awk 'Programm' Eingabedatei
```

**2. Programm aus Datei:**
```bash
awk -f Programm.awk Eingabedatei
```

**3. Ohne Eingabedatei (nur BEGIN):**
```bash
awk 'BEGIN { print "Hallo, Welt!" }'
```

### Kommandozeilenoptionen

| Option | Beschreibung |
|--------|--------------|
| `-F fs` | Feldtrenner setzen |
| `-v var=wert` | Variable vor Ausführung setzen |
| `-f datei` | Programm aus Datei lesen |
| `-h`, `--help` | Hilfe anzeigen |
| `--version` | Version anzeigen |

### Ihr erstes AWK-Programm

Erstellen Sie eine Datei `daten.txt`:
```
Alice 25 Entwicklung
Bob 30 Marketing
Carol 28 Entwicklung
```

Ausführen:
```bash
awk '{ print $1, $3 }' daten.txt
```

Ausgabe:
```
Alice Entwicklung
Bob Marketing
Carol Entwicklung
```

---

## 3. AWK-Programmstruktur

### Grundstruktur

Ein AWK-Programm besteht aus Muster-Aktions-Paaren:

```awk
Muster { Aktion }
Muster { Aktion }
...
```

- Wenn das **Muster** fehlt, gilt die Aktion für alle Datensätze
- Wenn die **Aktion** fehlt, werden passende Datensätze ausgegeben
- Mehrere Muster-Aktions-Paare werden der Reihe nach verarbeitet

### Programmablauf

```
1. BEGIN-Regeln ausführen (einmal, vor der Eingabe)
2. Für jede Eingabedatei:
   a. BEGINFILE-Regeln ausführen
   b. Für jeden Datensatz (Zeile):
      - In Felder aufteilen
      - Jedes Muster prüfen
      - Passende Aktionen ausführen
   c. ENDFILE-Regeln ausführen
3. END-Regeln ausführen (einmal, nach aller Eingabe)
```

### Beispiel-Programmstruktur

```awk
BEGIN {
    # Initialisierung
    FS = ","
    print "Verarbeitung gestartet"
}

/Muster/ {
    # Passende Zeilen verarbeiten
    anzahl++
}

END {
    # Abschlussverarbeitung
    print "Gesamttreffer:", anzahl
}
```

---

## 4. Muster (Patterns)

### Mustertypen

#### BEGIN und END

Werden einmal vor/nach der Verarbeitung ausgeführt:

```awk
BEGIN { print "Start" }
END { print "Ende" }
```

#### BEGINFILE und ENDFILE (gawk)

Werden vor/nach jeder Datei ausgeführt:

```awk
BEGINFILE { print "Verarbeite:", FILENAME }
ENDFILE { print "Fertig mit:", FILENAME }
```

#### Regulärer Ausdruck

Trifft auf Zeilen zu, die ein Muster enthalten:

```awk
/fehler/ { print }       # Zeilen mit "fehler"
/^#/ { next }            # Kommentarzeilen überspringen
/[0-9]+/ { print }       # Zeilen mit Zahlen
```

#### Ausdruck

Trifft basierend auf beliebigem Ausdruck zu:

```awk
$3 > 100 { print }           # Feld 3 größer als 100
NR > 10 { print }            # Nach Zeile 10
length($0) > 80 { print }    # Lange Zeilen
```

#### Negiertes Muster

Trifft auf Zeilen zu, die NICHT passen:

```awk
!/debug/ { print }       # Zeilen ohne "debug"
```

#### Bereichsmuster

Trifft von Startmuster bis Endmuster zu:

```awk
/START/,/ENDE/ { print } # Zeilen zwischen START und ENDE
NR==5,NR==10 { print }   # Zeilen 5 bis 10
```

#### Kombinierte Muster

Muster mit logischen Operatoren kombinieren:

```awk
/fehler/ && /kritisch/ { print }  # Beide Muster
/warnung/ || /fehler/ { print }   # Eines der Muster
```

---

## 5. Aktionen

### Anweisungstypen

Aktionen sind Anweisungsblöcke in geschweiften Klammern:

```awk
{
    anweisung1
    anweisung2
    ...
}
```

### Mehrere Anweisungen

Anweisungen können getrennt werden durch:
- Zeilenumbrüche
- Semikolons

```awk
{ x = 1; y = 2; print x + y }

# Oder:
{
    x = 1
    y = 2
    print x + y
}
```

### Leere Aktion

Eine leere Aktion gibt den gesamten Datensatz aus:

```awk
/Muster/    # Entspricht: /Muster/ { print $0 }
```

---

## 6. Variablen

### Feldvariablen

| Variable | Beschreibung |
|----------|--------------|
| `$0` | Gesamter aktueller Datensatz |
| `$1, $2, ...` | Einzelne Felder |
| `$NF` | Letztes Feld |
| `$(NF-1)` | Vorletztes Feld |

```awk
{ print $1, $NF }        # Erstes und letztes Feld
{ print $2 + $3 }        # Summe von Feld 2 und 3
{ $2 = "NEU"; print }    # Feld 2 ändern
```

### Benutzervariablen

Variablen werden bei erster Verwendung erstellt:

```awk
{ anzahl++ }                     # Numerisch (beginnt bei 0)
{ name = "Hans" }                # Zeichenkette
{ summe += $2 }                  # Akkumulator
BEGIN { schwellwert = 100 }      # In BEGIN initialisieren
```

### Variablentypen

AWK-Variablen sind dynamisch typisiert:

```awk
x = 42           # Numerisch
x = "hallo"      # Zeichenkette
x = "42"         # Zeichenkette, die numerisch aussieht
```

Typumwandlung erfolgt automatisch:
- Zeichenkette zu Zahl: Führender numerischer Teil, oder 0
- Zahl zu Zeichenkette: Formatiert durch CONVFMT

### Spezielle Variablen

Siehe [Abschnitt 14](#14-spezielle-variablen) für vollständige Liste.

Häufige spezielle Variablen:

| Variable | Beschreibung | Standard |
|----------|--------------|----------|
| `FS` | Feldtrenner | Leerzeichen |
| `RS` | Datensatztrenner | Zeilenumbruch |
| `OFS` | Ausgabe-Feldtrenner | Leerzeichen |
| `ORS` | Ausgabe-Datensatztrenner | Zeilenumbruch |
| `NR` | Datensatznummer (gesamt) | - |
| `NF` | Anzahl der Felder | - |
| `FNR` | Datensatznummer in Datei | - |
| `FILENAME` | Aktueller Dateiname | - |

---

## 7. Operatoren

### Arithmetische Operatoren

| Operator | Beschreibung | Beispiel |
|----------|--------------|----------|
| `+` | Addition | `x + y` |
| `-` | Subtraktion | `x - y` |
| `*` | Multiplikation | `x * y` |
| `/` | Division | `x / y` |
| `%` | Modulo | `x % y` |
| `^` | Potenz | `x ^ y` |

### Zuweisungsoperatoren

| Operator | Beschreibung | Entspricht |
|----------|--------------|------------|
| `=` | Zuweisung | `x = y` |
| `+=` | Addieren und zuweisen | `x = x + y` |
| `-=` | Subtrahieren und zuweisen | `x = x - y` |
| `*=` | Multiplizieren und zuweisen | `x = x * y` |
| `/=` | Dividieren und zuweisen | `x = x / y` |
| `%=` | Modulo und zuweisen | `x = x % y` |
| `^=` | Potenz und zuweisen | `x = x ^ y` |

### Inkrement/Dekrement

| Operator | Beschreibung |
|----------|--------------|
| `++x` | Prä-Inkrement |
| `x++` | Post-Inkrement |
| `--x` | Prä-Dekrement |
| `x--` | Post-Dekrement |

### Vergleichsoperatoren

| Operator | Beschreibung |
|----------|--------------|
| `<` | Kleiner als |
| `<=` | Kleiner oder gleich |
| `>` | Größer als |
| `>=` | Größer oder gleich |
| `==` | Gleich |
| `!=` | Ungleich |

### Logische Operatoren

| Operator | Beschreibung |
|----------|--------------|
| `&&` | Logisches UND |
| `\|\|` | Logisches ODER |
| `!` | Logisches NICHT |

### Zeichenkettenoperatoren

| Operator | Beschreibung | Beispiel |
|----------|--------------|----------|
| (Leerzeichen) | Verkettung | `"Hallo" " " "Welt"` |
| `~` | Regex-Übereinstimmung | `$1 ~ /Muster/` |
| `!~` | Keine Regex-Übereinstimmung | `$1 !~ /Muster/` |

### Weitere Operatoren

| Operator | Beschreibung | Beispiel |
|----------|--------------|----------|
| `? :` | Ternär | `x > 0 ? "pos" : "neg"` |
| `in` | Array-Mitgliedschaft | `schlüssel in array` |
| `$` | Feldzugriff | `$1`, `$(i+1)` |

### Operatorpriorität

Von höchster zu niedrigster:

1. `$` (Feldzugriff)
2. `^` (Potenz)
3. `++ --` (Inkrement/Dekrement)
4. `! - +` (Unär)
5. `* / %` (Multiplikation)
6. `+ -` (Addition)
7. (Verkettung)
8. `< <= > >= != ==` (Vergleich)
9. `~ !~` (Regex-Übereinstimmung)
10. `in` (Array-Mitgliedschaft)
11. `&&` (Logisches UND)
12. `||` (Logisches ODER)
13. `?:` (Ternär)
14. `= += -= *=` usw. (Zuweisung)

---

## 8. Kontrollstrukturen

### if-else

```awk
if (Bedingung) {
    Anweisungen
}

if (Bedingung) {
    Anweisungen
} else {
    Anweisungen
}

if (Bedingung1) {
    Anweisungen
} else if (Bedingung2) {
    Anweisungen
} else {
    Anweisungen
}
```

Beispiel:
```awk
{
    if ($3 > 100) {
        print $1, "hoch"
    } else if ($3 > 50) {
        print $1, "mittel"
    } else {
        print $1, "niedrig"
    }
}
```

### while

```awk
while (Bedingung) {
    Anweisungen
}
```

Beispiel:
```awk
BEGIN {
    i = 1
    while (i <= 5) {
        print i
        i++
    }
}
```

### do-while

```awk
do {
    Anweisungen
} while (Bedingung)
```

Beispiel:
```awk
BEGIN {
    i = 1
    do {
        print i
        i++
    } while (i <= 5)
}
```

### for

```awk
for (Init; Bedingung; Inkrement) {
    Anweisungen
}
```

Beispiel:
```awk
BEGIN {
    for (i = 1; i <= 10; i++) {
        print i, i * i
    }
}
```

### for-in (Arrays)

```awk
for (Schlüssel in Array) {
    Anweisungen
}
```

Beispiel:
```awk
END {
    for (wort in anzahl) {
        print wort, anzahl[wort]
    }
}
```

### switch-case (gawk)

```awk
switch (Ausdruck) {
    case Wert1:
        Anweisungen
        break
    case Wert2:
        Anweisungen
        break
    default:
        Anweisungen
}
```

Beispiel:
```awk
{
    switch ($1) {
        case "rot":
        case "grün":
        case "blau":
            print "Primärfarbe"
            break
        default:
            print "Andere Farbe"
    }
}
```

### Steueranweisungen

| Anweisung | Beschreibung |
|-----------|--------------|
| `break` | Innerste Schleife verlassen |
| `continue` | Nächste Iteration der Schleife |
| `next` | Zum nächsten Datensatz springen |
| `nextfile` | Zur nächsten Datei springen |
| `exit [code]` | Programm beenden |

---

## 9. Arrays

### Assoziative Arrays

AWK-Arrays sind assoziativ (durch Zeichenketten indiziert):

```awk
# Elemente erstellen/zugreifen
array["schlüssel"] = wert
array[1] = "erster"
array["name"] = "Hans"

# Zugriff
print array["schlüssel"]
```

### Array-Operationen

**Existenz prüfen:**
```awk
if ("schlüssel" in array) {
    print "existiert"
}
```

**Element löschen:**
```awk
delete array["schlüssel"]
```

**Gesamtes Array löschen:**
```awk
delete array
```

**Iteration:**
```awk
for (schlüssel in array) {
    print schlüssel, array[schlüssel]
}
```

### Mehrdimensionale Arrays

Simuliert mit SUBSEP:

```awk
# Speichern
matrix[1, 2] = 10
matrix[zeile, spalte] = wert

# Zugriff
print matrix[1, 2]

# Existenz prüfen
if ((i, j) in matrix) {
    print "existiert"
}
```

### Array-Beispiele

**Worthäufigkeit:**
```awk
{
    for (i = 1; i <= NF; i++)
        wörter[$i]++
}
END {
    for (w in wörter)
        print w, wörter[w]
}
```

**Nach Feld gruppieren:**
```awk
{
    summe[$1] += $2
    anzahl[$1]++
}
END {
    for (schlüssel in summe)
        print schlüssel, summe[schlüssel]/anzahl[schlüssel]
}
```

---

## 10. Funktionen

### Benutzerdefinierte Funktionen

```awk
function Name(Parameter) {
    Anweisungen
    return Wert
}
```

Beispiel:
```awk
function max(a, b) {
    return a > b ? a : b
}

function fakultät(n) {
    if (n <= 1) return 1
    return n * fakultät(n - 1)
}

BEGIN {
    print max(5, 3)
    print fakultät(6)
}
```

### Lokale Variablen

Lokale Variablen als zusätzliche Parameter deklarieren:

```awk
function summe_array(arr,    i, total) {
    total = 0
    for (i in arr)
        total += arr[i]
    return total
}
```

### Array-Parameter

Arrays werden als Referenz übergeben:

```awk
function verdopple_werte(arr,    schlüssel) {
    for (schlüssel in arr)
        arr[schlüssel] *= 2
}
```

### Eingebaute Funktionen

Siehe [Abschnitt 13](#13-eingebaute-funktionen) für vollständige Referenz.

---

## 11. Ein- und Ausgabe

### print-Anweisung

```awk
print                    # $0 ausgeben
print Ausdruck           # Ausdruck ausgeben
print Ausdruck1, Ausdruck2  # Mit OFS-Trenner ausgeben
```

### printf-Anweisung

```awk
printf Format, Ausdruck1, Ausdruck2, ...
```

**Formatspezifizierer:**

| Spezifizierer | Beschreibung |
|---------------|--------------|
| `%d`, `%i` | Ganzzahl |
| `%f` | Gleitkommazahl |
| `%e`, `%E` | Wissenschaftliche Notation |
| `%g`, `%G` | Kürzere Form von %f oder %e |
| `%s` | Zeichenkette |
| `%c` | Zeichen |
| `%x`, `%X` | Hexadezimal |
| `%o` | Oktal |
| `%%` | Prozentzeichen |

**Modifikatoren:**

| Modifikator | Beschreibung |
|-------------|--------------|
| `-` | Linksbündig |
| `+` | Vorzeichen anzeigen |
| ` ` | Leerzeichen vor positiven Zahlen |
| `0` | Nullen als Füllzeichen |
| `Breite` | Mindestbreite |
| `.Präzision` | Dezimalstellen / max. Zeichenkettenlänge |

Beispiele:
```awk
printf "%10s %5d %8.2f\n", name, anzahl, durchschnitt
printf "%-20s %08d\n", $1, $2
printf "%+.2f%%\n", prozent
```

### Ausgabeumleitung

**In Datei schreiben:**
```awk
print "Daten" > "datei.txt"
```

**An Datei anhängen:**
```awk
print "Daten" >> "datei.txt"
```

**An Befehl weiterleiten:**
```awk
print "Daten" | "sort"
```

**Koprozess (gawk):**
```awk
print "Daten" |& "befehl"
"befehl" |& getline ergebnis
```

### getline

Eingabe aus verschiedenen Quellen lesen:

| Form | Beschreibung |
|------|--------------|
| `getline` | Nächsten Datensatz aus Eingabe lesen |
| `getline var` | In Variable lesen |
| `getline < "datei"` | Aus Datei lesen |
| `getline var < "datei"` | Aus Datei in Variable lesen |
| `"cmd" \| getline` | Von Befehl lesen |
| `"cmd" \| getline var` | Von Befehl in Variable lesen |

Rückgabewerte:
- `1` = Erfolg
- `0` = Dateiende
- `-1` = Fehler

Beispiel:
```awk
BEGIN {
    while ((getline zeile < "daten.txt") > 0) {
        print zeile
    }
    close("daten.txt")
}
```

### Dateien schließen

```awk
close("dateiname")    # Datei schließen
close("befehl")       # Pipe schließen
```

---

## 12. Reguläre Ausdrücke

### Grundsyntax

```awk
/Muster/            # Muster in Schrägstrichen
$1 ~ /Muster/       # Feld passt auf Muster
$1 !~ /Muster/      # Feld passt nicht auf Muster
```

### Metazeichen

| Zeichen | Beschreibung |
|---------|--------------|
| `.` | Beliebiges einzelnes Zeichen |
| `*` | Null oder mehr des Vorherigen |
| `+` | Ein oder mehr des Vorherigen |
| `?` | Null oder ein des Vorherigen |
| `^` | Anfang der Zeichenkette/Zeile |
| `$` | Ende der Zeichenkette/Zeile |
| `[...]` | Zeichenklasse |
| `[^...]` | Negierte Zeichenklasse |
| `\|` | Alternation (ODER) |
| `(...)` | Gruppierung |
| `{n}` | Genau n mal |
| `{n,}` | n oder mehr mal |
| `{n,m}` | n bis m mal |

### Zeichenklassen

| Klasse | Beschreibung |
|--------|--------------|
| `[abc]` | a, b oder c |
| `[a-z]` | Kleinbuchstabe |
| `[A-Z]` | Großbuchstabe |
| `[0-9]` | Ziffer |
| `[^0-9]` | Keine Ziffer |
| `[a-zA-Z]` | Beliebiger Buchstabe |
| `[a-zA-Z0-9]` | Alphanumerisch |

### Escape-Sequenzen

| Sequenz | Beschreibung |
|---------|--------------|
| `\\` | Backslash |
| `\.` | Punkt |
| `\*` | Sternchen |
| `\n` | Zeilenumbruch |
| `\t` | Tabulator |

### Beispiele

```awk
/^$/                    # Leere Zeile
/^#/                    # Kommentarzeile
/[0-9]+/                # Enthält Zahl
/^[A-Z][a-z]+$/         # Wort mit Großbuchstabe
/fehler\|warnung/i      # Fehler oder Warnung (Groß-/Kleinschreibung ignorieren)
/^.{80,}$/              # Zeilen mit 80+ Zeichen
```

---

## 13. Eingebaute Funktionen

### Zeichenkettenfunktionen

#### length(s)
Gibt die Zeichenkettenlänge zurück, oder Array-Länge wenn s ein Array ist.
```awk
print length("hallo")     # 5
print length(array)       # Anzahl der Elemente
```

#### substr(s, start [, len])
Gibt Teilzeichenkette ab Position start (1-indiziert) zurück.
```awk
print substr("hallo", 2, 3)    # "all"
print substr("hallo", 3)       # "llo"
```

#### index(s, t)
Gibt Position von t in s zurück, oder 0 wenn nicht gefunden.
```awk
print index("hallo", "ll")     # 3
```

#### split(s, a [, fs [, seps]])
Teilt Zeichenkette s in Array a mit Trenner fs.
```awk
n = split("a,b,c", arr, ",")   # n=3, arr[1]="a", usw.
```

#### sub(regex, ersetzung [, ziel])
Ersetzt erste Übereinstimmung von regex in ziel (Standard $0).
```awk
sub(/alt/, "neu")              # In $0
sub(/alt/, "neu", str)         # In str
```

#### gsub(regex, ersetzung [, ziel])
Ersetzt alle Übereinstimmungen von regex in ziel.
```awk
gsub(/alt/, "neu")             # Alle in $0 ersetzen
n = gsub(/a/, "A", str)        # Gibt Anzahl zurück
```

#### gensub(regex, ersetzung, wie [, ziel]) (gawk)
Ersetzen mit Rückverweisunterstützung.
```awk
# wie: "g" für global, oder Zahl für n-tes Vorkommen
gensub(/(.)(.)/, "\\2\\1", "g", "abcd")   # "badc"
```

#### match(s, regex [, arr])
Gibt Position der Übereinstimmung zurück, setzt RSTART und RLENGTH.
```awk
if (match(s, /[0-9]+/)) {
    print substr(s, RSTART, RLENGTH)
}

# Mit Erfassungsgruppen (gawk):
match(s, /([0-9]+)-([0-9]+)/, gruppen)
print gruppen[1], gruppen[2]
```

#### tolower(s)
Gibt Kleinbuchstaben-Version von s zurück.
```awk
print tolower("Hallo")         # "hallo"
```

#### toupper(s)
Gibt Großbuchstaben-Version von s zurück.
```awk
print toupper("Hallo")         # "HALLO"
```

#### sprintf(format, ausdruck, ...)
Gibt formatierte Zeichenkette zurück.
```awk
s = sprintf("%05d", 42)        # "00042"
```

#### strtonum(s) (gawk)
Wandelt Zeichenkette in Zahl um, erkennt Hex (0x) und Oktal (0).
```awk
print strtonum("0x1F")         # 31
```

### Mathematische Funktionen

| Funktion | Beschreibung |
|----------|--------------|
| `sin(x)` | Sinus (Bogenmaß) |
| `cos(x)` | Kosinus |
| `tan(x)` | Tangens |
| `atan2(y, x)` | Arkustangens von y/x |
| `asin(x)` | Arkussinus |
| `acos(x)` | Arkuskosinus |
| `sinh(x)` | Hyperbolischer Sinus |
| `cosh(x)` | Hyperbolischer Kosinus |
| `tanh(x)` | Hyperbolischer Tangens |
| `exp(x)` | e^x |
| `log(x)` | Natürlicher Logarithmus |
| `log10(x)` | Zehnerlogarithmus |
| `log2(x)` | Zweierlogarithmus |
| `sqrt(x)` | Quadratwurzel |
| `int(x)` | Zu Ganzzahl abschneiden |
| `ceil(x)` | Aufrunden |
| `floor(x)` | Abrunden |
| `round(x)` | Auf nächste Ganzzahl runden |
| `abs(x)` | Absolutwert |
| `rand()` | Zufallszahl [0,1) |
| `srand([seed])` | Zufallsgenerator initialisieren |

### Array-Funktionen (gawk)

#### asort(quelle [, ziel [, wie]])
Sortiert Array-Werte.
```awk
n = asort(arr)                 # An Ort sortieren
n = asort(arr, sortiert)       # In neues Array sortieren
```

#### asorti(quelle [, ziel [, wie]])
Sortiert Array-Indizes.
```awk
n = asorti(arr, schlüssel)     # Sortierte Schlüssel erhalten
```

#### isarray(x)
Gibt 1 zurück wenn x ein Array ist.
```awk
if (isarray(arr)) print "ist Array"
```

### E/A-Funktionen

#### close(datei)
Schließt Datei oder Pipe.
```awk
close("ausgabe.txt")
close("sort")
```

#### fflush([datei])
Leert Ausgabepuffer.
```awk
fflush()                       # Alle leeren
fflush("ausgabe.txt")          # Bestimmte Datei leeren
```

#### system(befehl)
Führt Shell-Befehl aus, gibt Exit-Status zurück.
```awk
status = system("ls -l")
```

### Zeitfunktionen

#### systime()
Gibt aktuellen Unix-Zeitstempel zurück.
```awk
jetzt = systime()
```

#### mktime(datumsspec)
Wandelt Datumsspezifikation in Zeitstempel um.
```awk
ts = mktime("2024 06 15 12 30 00")
```

#### strftime(format [, zeitstempel])
Formatiert Zeitstempel als Zeichenkette.
```awk
print strftime("%Y-%m-%d %H:%M:%S", systime())
```

Häufige Formatcodes:
| Code | Beschreibung |
|------|--------------|
| `%Y` | 4-stelliges Jahr |
| `%m` | Monat (01-12) |
| `%d` | Tag (01-31) |
| `%H` | Stunde (00-23) |
| `%M` | Minute (00-59) |
| `%S` | Sekunde (00-59) |
| `%A` | Wochentagname |
| `%B` | Monatsname |

### Bitweise Funktionen (gawk)

| Funktion | Beschreibung |
|----------|--------------|
| `and(x, y)` | Bitweises UND |
| `or(x, y)` | Bitweises ODER |
| `xor(x, y)` | Bitweises XOR |
| `lshift(x, n)` | Linksverschiebung |
| `rshift(x, n)` | Rechtsverschiebung |
| `compl(x)` | Bitweises Komplement |

### Typfunktionen (gawk)

| Funktion | Beschreibung |
|----------|--------------|
| `typeof(x)` | Gibt Typnamen zurück |
| `isarray(x)` | Prüft ob Array |
| `mkbool(x)` | Zu Boolean umwandeln |

---

## 14. Spezielle Variablen

### Ein-/Ausgabevariablen

| Variable | Beschreibung | Standard |
|----------|--------------|----------|
| `FS` | Eingabe-Feldtrenner | Leerzeichen |
| `RS` | Eingabe-Datensatztrenner | Zeilenumbruch |
| `OFS` | Ausgabe-Feldtrenner | Leerzeichen |
| `ORS` | Ausgabe-Datensatztrenner | Zeilenumbruch |
| `CONVFMT` | Zahl-zu-Zeichenkette-Format | `"%.6g"` |
| `OFMT` | Ausgabe-Zahlenformat | `"%.6g"` |
| `SUBSEP` | Array-Index-Trenner | `"\034"` |

### Datensatzinformationen

| Variable | Beschreibung |
|----------|--------------|
| `NR` | Gesamtzahl gelesener Datensätze |
| `NF` | Felder im aktuellen Datensatz |
| `FNR` | Datensätze in aktueller Datei |
| `$0` | Gesamter aktueller Datensatz |
| `$1..$NF` | Einzelne Felder |
| `FILENAME` | Aktueller Eingabedateiname |

### Regex-Übereinstimmungsergebnisse

| Variable | Beschreibung |
|----------|--------------|
| `RSTART` | Startposition der Übereinstimmung |
| `RLENGTH` | Länge der Übereinstimmung |

### Programmargumente

| Variable | Beschreibung |
|----------|--------------|
| `ARGC` | Anzahl der Argumente |
| `ARGV` | Array der Argumente |
| `ENVIRON` | Umgebungsvariablen |

### gawk-Erweiterungen

| Variable | Beschreibung |
|----------|--------------|
| `IGNORECASE` | Groß-/Kleinschreibung ignorieren (0/1) |
| `RT` | Tatsächlicher Datensatztrenner-Text |
| `FPAT` | Feldmuster (Alternative zu FS) |
| `SYMTAB` | Symboltabellenzugriff |
| `FUNCTAB` | Funktionstabelle |
| `PROCINFO` | Prozessinformationen |

---

## 15. gawk-Erweiterungen

Dieser Interpreter unterstützt viele GNU AWK-Erweiterungen:

### BEGINFILE und ENDFILE

```awk
BEGINFILE {
    print "Starte:", FILENAME
}
ENDFILE {
    print "Fertig mit:", FILENAME, "mit", FNR, "Datensätzen"
}
```

### switch-Anweisung

```awk
switch (wert) {
    case "a":
    case "b":
        print "Buchstabe"
        break
    case 1:
        print "Zahl"
        break
    default:
        print "Anderes"
}
```

### Namensräume

```awk
@namespace "meinebib"

function helfer() {
    return 42
}

@namespace "awk"

BEGIN {
    print meinebib::helfer()
}
```

### Koprozesse

Bidirektionale Kommunikation mit externen Prozessen:

```awk
BEGIN {
    cmd = "sort"
    print "banane" |& cmd
    print "apfel" |& cmd
    close(cmd, "to")

    while ((cmd |& getline zeile) > 0)
        print zeile
    close(cmd)
}
```

### Indirekte Funktionsaufrufe

```awk
function addiere(a, b) { return a + b }
function multipliziere(a, b) { return a * b }

BEGIN {
    op = "addiere"
    print @op(3, 4)    # Ruft addiere(3, 4) auf
}
```

### FPAT (Feldmuster)

Felder durch Inhalt statt Trenner definieren:

```awk
BEGIN {
    FPAT = "([^,]+)|(\"[^\"]+\")"
}
{
    # Behandelt korrekt: feld1,"feld,mit,kommas",feld3
    for (i = 1; i <= NF; i++)
        print i ": " $i
}
```

### IGNORECASE

```awk
BEGIN { IGNORECASE = 1 }
/fehler/ { print }    # Findet FEHLER, Fehler, fehler, usw.
```

---

## 16. Beispiele

### Zeilennummerierung

```awk
{ print NR ": " $0 }
```

### Leerzeilen entfernen

```awk
NF > 0 { print }
# Oder: /./
# Oder: !/^$/
```

### Spalte summieren

```awk
{ summe += $2 }
END { print "Summe:", summe }
```

### Durchschnitt

```awk
{ summe += $2; anzahl++ }
END { print "Durchschnitt:", summe/anzahl }
```

### Maximum finden

```awk
NR == 1 || $2 > max { max = $2; maxzeile = $0 }
END { print "Max:", max, "Zeile:", maxzeile }
```

### Worthäufigkeit

```awk
{
    for (i = 1; i <= NF; i++) {
        wort = tolower($i)
        gsub(/[^a-zäöüß]/, "", wort)
        if (wort) wörter[wort]++
    }
}
END {
    for (w in wörter)
        printf "%4d %s\n", wörter[w], w
}
```

### Duplikate entfernen (Reihenfolge beibehalten)

```awk
!gesehen[$0]++ { print }
```

### CSV in feste Breite

```awk
BEGIN { FS = ","; OFS = "" }
{
    printf "%-20s %-10s %8.2f\n", $1, $2, $3
}
```

### Protokollanalyse

```awk
/FEHLER/ { fehler++ }
/WARNUNG/ { warnungen++ }
END {
    print "Fehler:", fehler
    print "Warnungen:", warnungen
}
```

### Gruppenstatistik

```awk
{
    gruppe = $1
    wert = $2
    summe[gruppe] += wert
    anzahl[gruppe]++
    if (!(gruppe in min) || wert < min[gruppe]) min[gruppe] = wert
    if (!(gruppe in max) || wert > max[gruppe]) max[gruppe] = wert
}
END {
    printf "%-10s %8s %8s %8s %8s\n", "Gruppe", "Anzahl", "Summe", "Min", "Max"
    for (g in summe)
        printf "%-10s %8d %8.2f %8.2f %8.2f\n", g, anzahl[g], summe[g], min[g], max[g]
}
```

---

## 17. Fehlerbehebung

### Häufige Fehler

**"field separator is empty" (Feldtrenner ist leer)**
```bash
# Falsch: Leerer Trenner
awk -F'' '...'

# Richtig: Einzelnes Zeichen verwenden
awk -F',' '...'
```

**Division durch Null**
```awk
# Vor Division prüfen
{ if (anzahl > 0) print summe/anzahl }
```

**Nicht initialisierte Variable**
```awk
# Variablen sind standardmäßig "" oder 0
# Bei Bedarf in BEGIN initialisieren
BEGIN { anzahl = 0 }
```

**Zeichenketten- vs. Zahlenvergleich**
```awk
# "10" < "9" ist WAHR (Zeichenkettenvergleich)
# Numerisch erzwingen: 0 addieren
{ if ($1 + 0 > 9) print }
```

### Debugging-Tipps

**Variablenwerte ausgeben:**
```awk
{ print "DEBUG: x=" x " y=" y > "/dev/stderr" }
```

**Feldinformationen ausgeben:**
```awk
{
    print "NF=" NF
    for (i = 1; i <= NF; i++)
        print "  $" i "=[" $i "]"
}
```

**Feldtrenner prüfen:**
```awk
BEGIN { print "FS=[" FS "]" }
```

### Plattformunterschiede

**Zeilenenden:**
- Windows: CRLF (`\r\n`)
- Unix/Mac: LF (`\n`)

Behandlung:
```awk
{ gsub(/\r$/, "") }  # Abschließendes CR entfernen
```

**Pfadtrenner:**
- Windows: `\`
- Unix: `/`

---

## Kurzreferenz

### Einzeiler

```bash
# Zeilen ausgeben, die auf Muster passen
awk '/Muster/' datei

# Bestimmte Felder ausgeben
awk '{ print $1, $3 }' datei

# Zeilen ausgeben, wo Feld > Wert
awk '$2 > 100' datei

# Spalte summieren
awk '{ s += $1 } END { print s }' datei

# Zeilen zählen
awk 'END { print NR }' datei

# Zeilennummern ausgeben
awk '{ print NR, $0 }' datei

# Duplikate entfernen
awk '!gesehen[$0]++' datei

# Letztes Feld ausgeben
awk '{ print $NF }' datei

# Zeilen 10-20 ausgeben
awk 'NR >= 10 && NR <= 20' datei

# Text ersetzen
awk '{ gsub(/alt/, "neu"); print }' datei
```

---

*Ende des Benutzerhandbuchs*
