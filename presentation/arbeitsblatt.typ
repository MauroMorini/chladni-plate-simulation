// Arbeitsblatt für die Gruppenarbeit nach dem Vortrag (Gruppen von 2–4, mit Laptop).
// Kompilieren (im Ordner presentation/):  typst compile arbeitsblatt.typ
// Seite 1: Animationen und Chladni-Spiel auf der Website, Seite 2: Lautsprecher und Teller.
// Lösung zu Aufgabe 4 (Spiel, μ = 0.5): + bei f ≈ 23, × bei f ≈ 34, Ring bei f ≈ 50.

// Farben der Universität Basel, wie in slides.typ
#let dark = rgb("#2D373C")
#let mint = rgb("#A5D7D2")
#let red = rgb("#D20537")

#set document(title: "Arbeitsblatt: Wellen und Chladni-Figuren")
#set page(paper: "a4", margin: (x: 2cm, y: 1.8cm),
  footer: text(size: 9pt, fill: dark.lighten(30%))[Departement Mathematik und Informatik, Universität Basel #h(1fr) tinyurl.com/tecday26])
#set text(font: "Inter", size: 11pt, lang: "de", region: "ch", fill: dark)
#set par(leading: 0.7em)
#show heading.where(level: 1): set text(size: 20pt)
#show heading.where(level: 2): it => block(above: 1.6em, below: 0.8em, text(fill: red, it))
#show strong: set text(fill: red)

// Nummerierte Aufgabe; die Nummern laufen über alle Teile weiter
#let nr = counter("aufgabe")
#let aufgabe(body) = block(above: 1.1em, below: 0.6em, grid(columns: (1.6em, 1fr),
  { nr.step(); text(weight: "bold", fill: red, context nr.display() + ".") }, body))
// Platz zum Schreiben bzw. Zeichnen
#let platz(hoehe) = block(width: 100%, height: hoehe, stroke: (bottom: 0.5pt + mint))
#let skizze(beschriftung) = stack(spacing: 0.4em,
  rect(width: 100%, height: 4.2cm, stroke: 0.6pt + dark.lighten(40%), radius: 3pt),
  text(size: 9pt, beschriftung))
// Gesuchtes Muster im Spiel: Knotenlinien auf der quadratischen Platte, darunter Platz für die Frequenz
#let s = 1.5cm
#let linie(..pkte) = place(line(stroke: 1.6pt + dark, ..pkte))
#let muster(form) = stack(spacing: 0.4em,
  box(width: s, height: s, fill: mint.lighten(60%), stroke: 0.6pt + dark.lighten(40%), {
    if form == "plus" { linie(start: (50%, 0%), end: (50%, 100%)); linie(start: (0%, 50%), end: (100%, 50%)) }
    if form == "kreuz" { linie(start: (0%, 0%), end: (100%, 100%)); linie(start: (100%, 0%), end: (0%, 100%)) }
    if form == "ring" { place(center + horizon, rect(width: 72%, height: 72%, radius: 30%, stroke: 1.6pt + dark)) }
  }),
  text(size: 9pt)[f = #box(width: 1cm, stroke: (bottom: 0.5pt))])

= Wellen und Chladni-Figuren
#text(fill: dark.lighten(25%))[Simulationen: *tinyurl.com/tecday26* #h(1fr) Gruppe: #box(width: 5cm, stroke: (bottom: 0.5pt))]

== Teil 1 · Simulationen

#aufgabe[
  *Fliegender Ball.* Haltet den Ball am höchsten Punkt an. Wie gross sind $v$ und $a$?
  Warum ist $a$ dort nicht null?
  #platz(1.6cm)
]

#aufgabe[
  *Stehende Welle.* Zählt die Knoten für $n = 1, 2, 3, 4$. Welche Regel findet ihr? \
  Schaltet «auch Zwischenwerte» ein: Warum kann eine eingespannte Saite nur bei ganz bestimmten
  Frequenzen schwingen?
  #platz(2.1cm)
]

#aufgabe[
  *Raum-Zeit-Bild.* Wählt «Eigenschwingung» und sucht einen Punkt $x_0$, der sich nie bewegt.
  Woran erkennt ihr ihn im Raum-Zeit-Bild? \
  Was ist bei der «gezupften Saite» anders?
  #platz(2.1cm)
]

== Teil 2 · Chladni-Spiel

#aufgabe[
  Schaltet «Dauernd streuen» ein und sucht die Frequenzen dieser drei Muster:
  #v(0.3em)
  #grid(columns: 3, column-gutter: 1.5cm, muster("plus"), muster("kreuz"), muster("ring"))
]

#aufgabe[
  Was macht der Sand zwischen zwei Eigenfrequenzen? Warum?
  #platz(1.4cm)
]

#aufgabe[
  Ändert das Material $mu$. Was bleibt gleich, was ändert sich?
  #platz(1.4cm)
]

#pagebreak()

== Teil 3 · Eure eigene Chladni-Platte

#text(size: 10pt, fill: dark.lighten(20%))[Material: Lautsprecher, Teller, Salz, Tongenerator (Handy oder Laptop)]

#aufgabe[
  Legt den Teller auf den Lautsprecher, streut wenig Salz darauf und erhöht die Frequenz langsam.
  Skizziert drei Muster und notiert die Frequenz.
  #v(0.4em)
  #grid(columns: (1fr, 1fr, 1fr), column-gutter: 0.8em,
    skizze[f = #h(1fr) Hz], skizze[f = #h(1fr) Hz], skizze[f = #h(1fr) Hz])
]

#aufgabe[
  Werden die Muster mit steigender Frequenz feiner oder gröber? Warum?
  #platz(2cm)
]

#aufgabe[
  Berührt den Teller mit einem Finger am Rand. Was passiert mit dem Muster?
  #platz(2cm)
]

#aufgabe[
  Vergleicht eure Muster mit dem Spiel. Was ist anders, und warum?
  #platz(2cm)
]
