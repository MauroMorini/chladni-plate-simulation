// Vortrag für Gymnasiast:innen: Numerische Analysis der Wellengleichung und Chladni-Figuren.
// Kompilieren (im Ordner presentation/):  typst compile slides.typ
// Live-Vorschau:                          typst watch slides.typ
// Die Abbildungen in figures/ erzeugt figures/make_figures.py, die Fotos in images/ stammen aus
// old_presentations/TecDay2025.pdf.

#import "@preview/touying:0.6.1": *
#import themes.metropolis: *

// Farben der Universität Basel
#let dark = rgb("#2D373C")
#let mint = rgb("#A5D7D2")
#let red = rgb("#D20537")
#let blue = rgb("#1F6F8B")

#show: metropolis-theme.with(
  aspect-ratio: "16-9",
  footer: self => self.info.institution,
  config-info(
    title: [Wellen machen Unsichtbares sichtbar],
    subtitle: [Numerische Analysis der Wellengleichung],
    author: [Martin Ramm, Mauro Morini],
    date: none,
    institution: [Departement Mathematik und Informatik, Universität Basel],
  ),
  config-colors(
    primary: red,
    primary-light: mint,
    secondary: dark,
    neutral-lightest: rgb("#ffffff"),
    neutral-dark: dark,
    neutral-darkest: dark,
  ),
)

#set text(font: "Inter", size: 16pt, lang: "de", region: "ch")
#show raw: set text(size: 12pt)
#show math.equation: set text(font: "New Computer Modern Math")
#set list(marker: text(red)[▸])
#show strong: set text(fill: red)

// Kleine Helfer
#let caption(body) = text(size: 11pt, fill: dark.lighten(25%), body)
#let photo(path, label, ..args) = figure(image(path, ..args), caption: caption(label), supplement: none, numbering: none, gap: 0.4em)
#let boxed(body, fill: mint.lighten(60%)) = box(fill: fill, inset: 0.7em, radius: 6pt, body)
// Knopf, der eine Animation aus animations/ im Browser öffnet
#let animation(path, label) = link(path, box(fill: red, inset: (x: 0.6em, y: 0.35em), radius: 4pt, text(fill: white, size: 13pt)[▶ #label]))
#let step(body) = box(fill: mint.lighten(50%), stroke: dark + 1pt, inset: 0.6em, radius: 6pt, width: 100%, align(center, body))

#title-slide()

== Übersicht

#slide(composer: (1.2fr, 1fr))[
  #set text(size: 19pt)
  + *Wellen* -- wo sie vorkommen und wie man sie mit Mathematik beschreibt
  + *Computersimulation* -- wie ein Computer die Wellengleichung löst
  + *Chladni-Figuren* -- warum Sand auf einer schwingenden Platte Muster bildet

  #v(1em)
  Danach: *Sie experimentieren selbst!*
][
  #photo("images/chladni_setup.png", [Chladni-Platte mit Lautsprecher und Frequenzgenerator])
]

= Wellen

== Was ist eine Welle?

#slide(composer: (1fr, 1fr))[
  #image("images/surfer.png", height: 5.2cm)
  #v(0.5em)
  #image("images/pulse_travelling.png", height: 3.6cm)
][
  #boxed[Eine *Welle* ist eine Störung, die sich ausbreitet. Sie transportiert Energie und Information -- aber kein Material.]

  #v(0.5em)
  Beispiel *La-Ola-Welle* im Stadion:
  - Jede Person steht nur kurz auf und setzt sich wieder.
  - Trotzdem läuft die Welle einmal ums ganze Stadion.

  #v(0.5em)
  Genauso bei Wasser, Schall oder einer Gitarrensaite: Die Teilchen schwingen nur hin und her, die *Form* wandert weiter.
]

== Wellen sind überall

#grid(
  columns: (1fr, 1fr, 1fr),
  gutter: 1em,
  photo("images/bat.png", [Fledermäuse orten Beute mit Ultraschall-Echos], height: 5.5cm),
  photo("images/ultrasound.png", [Ultraschallbild in der Medizin], height: 5.5cm),
  photo("images/seismic.png", [Seismik: Schallwellen zeigen, was im Boden liegt], height: 5.5cm),
)

#v(0.5em)
#align(center)[
  *Schall* (Musik, Sprache) #h(0.6em) · #h(0.6em) *Licht*, Funk, WLAN, Röntgen #h(0.6em) · #h(0.6em) *Erdbeben* #h(0.6em) · #h(0.6em) *Wasserwellen* #h(0.6em) · #h(0.6em) *Saiten und Platten*
]

#speaker-note[Gemeinsamer Trick: Eine Welle trifft auf ein Hindernis und wird reflektiert. Aus dem Echo kann man auf das Unsichtbare schliessen.]

== Wie beschreibt man eine Welle?

Wir betrachten eine schwingende *Saite* (z.B. Gitarre). Ihre Form ändert sich mit der Zeit:

#align(center)[
  #boxed[$u(x, t)$ #h(0.5em) = Auslenkung der Saite am *Ort* $x$ zur *Zeit* $t$]
]

#align(center, image("figures/partial_derivatives.svg", height: 6cm))

Eine Funktion mit *zwei* Variablen: links ein Foto (Zeit fest), rechts ein einzelner Punkt im Lauf der Zeit.

== Erinnerung: die Ableitung

#slide(composer: (1.1fr, 1fr))[
  #image("figures/derivative_slope.svg", width: 100%)
][
  Die Ableitung $u'(x)$ ist die *Steigung* der Tangente:

  $ u'(x) = lim_(h -> 0) (u(x + h) - u(x)) / h $

  #v(0.3em)
  In Worten: Wie stark ändert sich $u$, wenn ich $x$ ein kleines bisschen verändere?

  #v(0.3em)
  - $u' > 0$: steigt
  - $u' < 0$: fällt
  - $u' = 0$: oben oder unten flach
]

== Ableitungen und Bewegung

Ein Ball wird senkrecht nach oben geworfen:

#align(center, image("figures/derivative_motion.svg", height: 5.4cm))

#grid(
  columns: (1fr, auto),
  align: horizon,
  [
    - 1. Ableitung der Höhe = *Geschwindigkeit*
    - 2. Ableitung der Höhe = *Beschleunigung*
  ],
  animation("animations/ball.html")[Animation: fliegender Ball],
)

== Die zweite Ableitung: Krümmung

#slide(composer: (1.3fr, 1fr))[
  #image("figures/curvature.svg", width: 100%)
][
  Die 2. Ableitung $u''$ sagt, wie sich die Steigung ändert, also wie die Kurve *gebogen* ist:

  #v(0.3em)
  - #text(fill: red)[$u'' < 0$]: biegt nach unten (wie ein Hügel)
  - #text(fill: blue)[$u'' > 0$]: biegt nach oben (wie ein Tal)
  - grosses $|u''|$: stark gekrümmt
]

== Partielle Ableitungen

#slide(composer: (0.9fr, 1.2fr))[
  #set text(size: 15pt)
  #image("figures/space_time.svg", width: 100%)
  #align(center, animation("animations/raumzeit.html")[Animation: Raum-Zeit-Bild])
][
  #set text(size: 15pt)
  $u(x, t)$ ist eine *Landschaft* über der $(x, t)$-Ebene (Farbe = Auslenkung).

  #v(0.2em)
  - Eine *waagrechte Linie* ist ein Foto der Saite: Zeit $t_0$ fest, nur $x$ ändert sich.
  - Eine *senkrechte Linie* ist ein Punkt $x_0$ im Lauf der Zeit: nur $t$ ändert sich.

  #v(0.2em)
  Entlang einer solchen Linie ist $u$ wieder eine ganz normale Funktion *einer* Variablen, und die können wir wie gewohnt ableiten:

  #v(0.2em)
  #table(
    columns: (auto, auto, auto),
    stroke: none,
    inset: 0.4em,
    align: horizon,
    fill: (_, row) => if row == 0 { mint } else { mint.lighten(70%) },
    [], [*Nach der Zeit* ($x$ fest)], [*Nach dem Ort* ($t$ fest)],
    [1.], [$partial u \/ partial t$ = Geschwindigkeit], [$partial u \/ partial x$ = Steigung],
    [2.], [$partial^2 u \/ partial t^2$ = *Beschleunigung*], [$partial^2 u \/ partial x^2$ = *Krümmung*],
  )

  #v(0.2em)
  Das Symbol $partial$ ("d rund") heisst: Ableitung nach einer Variablen, die anderen bleiben fest.
]

#speaker-note[In der Animation kann man x_0 und t_0 verschieben: Die Tangente im Foto hat die Steigung du/dx, die Tangente in der Zeitkurve die Steigung du/dt (Geschwindigkeit des Punktes).]

== Die Wellengleichung

#align(center)[
  #box(fill: mint.lighten(50%), inset: 0.8em, radius: 8pt)[
    #text(size: 24pt)[$ (partial^2 u) / (partial t^2) = c^2 (partial^2 u) / (partial x^2) $]
  ]
  #v(-0.3em)
  *Beschleunigung* #h(0.3em) = #h(0.3em) $c^2$ $times$ *Krümmung*
]

#pause
#grid(
  columns: (1.4fr, 1fr),
  gutter: 1em,
  image("figures/wave_equation_forces.svg", height: 5cm),
  align(horizon)[
    Je stärker die Saite gebogen ist, desto stärker wird sie zurückgezogen.

    #v(0.3em)
    Das ist Newtons $F = m a$ für jedes kleine Stück der Saite.
  ],
)

== Was bedeutet $c$?

#slide(composer: (1fr, 1fr))[
  $c$ ist die *Ausbreitungsgeschwindigkeit* der Welle. Sie hängt vom Material ab.

  #v(0.5em)
  Dieselbe Gleichung beschreibt (in 2D oder 3D) Schall, Licht, Erdbeben, Wasserwellen ... nur $c$ ist anders.
][
  #table(
    columns: (auto, auto),
    stroke: none,
    inset: 0.5em,
    fill: (_, row) => if row == 0 { mint } else if calc.odd(row) { mint.lighten(70%) },
    [*Welle*], [*Geschwindigkeit $c$*],
    [Schall in Luft], [343 m/s],
    [Schall in Wasser], [1480 m/s],
    [Schall in Aluminium], [ca. 6400 m/s],
    [Erdbebenwellen], [3--8 km/s],
    [Licht], [300'000 km/s],
  )
]

== Eine Gleichung aus dem 18. Jahrhundert

#slide(composer: (1fr, 2fr, 1fr))[
  #photo("images/bernoulli.png", [Daniel Bernoulli (1700--1782), Professor in Basel])
][
  #set text(size: 17pt)
  - Die Wellengleichung ist fast 300 Jahre alt (d'Alembert 1747, Bernoulli, Euler).
  - Für eine Saite kann man sie *von Hand* lösen.
  - Aber für eine Platte mit Schraube, eine Geige, ein Erdbeben oder einen Konzertsaal? *Keine Formel!*

  #v(0.5em)
  #boxed[Lösung: Wir lassen den *Computer* rechnen. Das ist *numerische Mathematik*.]
][
  #photo("images/dalembert.png", [Jean-Baptiste le Rond d'Alembert (1717--1783)])
]

= Computersimulation

== Was ist eine Computersimulation?

#v(0.5em)
#grid(
  columns: (1fr, auto, 1fr, auto, 1fr, auto, 1fr),
  gutter: 0.4em,
  align: horizon,
  step[*Physik* \ #text(size: 16pt)[$F = m a$, Materialgesetze]],
  text(size: 28pt, fill: red)[→],
  step[*Modell* \ #text(size: 16pt)[Wellengleichung]],
  text(size: 28pt, fill: red)[→],
  step[*Algorithmus* \ #text(size: 16pt)[Rechenrezept (Numerik)]],
  text(size: 28pt, fill: red)[→],
  step[*Programm* \ #text(size: 16pt)[C++, Python, ...]],
)
#v(0.3em)
#align(right)[#text(size: 28pt, fill: red)[↓] #h(3.2cm)]
#grid(
  columns: (1fr, auto, 1fr),
  gutter: 0.4em,
  align: horizon,
  step[*Vergleich mit dem Experiment* \ #text(size: 16pt)[Stimmt das Modell? Verbessern!]],
  text(size: 28pt, fill: red)[←],
  step[*Simulation* \ #text(size: 16pt)[Zahlen, Bilder, Vorhersagen]],
)

#v(1fr)
#align(center, text(size: 16pt, style: "italic")[«Make everything as simple as possible, but not simpler.» -- Albert Einstein])

== Idee 1: Diskretisieren

Ein Computer kann nur *endlich viele* Zahlen speichern. Wir merken uns die Saite nur an Gitterpunkten mit Abstand $h$:

#align(center, image("figures/discretization.svg", height: 5.4cm))

#align(center)[Statt der Funktion $u(x)$ berechnen wir die Zahlen $u_i approx u(x_i)$ #h(0.5em) mit $x_i = i dot h$.]

== Idee 2: Ableitungen durch Differenzen ersetzen

#slide(composer: (1.1fr, 1fr))[
  #image("figures/secants.svg", width: 100%)
][
  Die Ableitung ist ein Grenzwert für $h -> 0$. \
  Der Computer nimmt einfach ein *kleines* $h$:

  $ u'(x) approx (u(x + h) - u(x)) / h $

  #v(0.3em)
  Je kleiner $h$, desto näher liegt die Sekante (blau) an der Tangente (rot). Die Steigungen (oben links) nähern sich dem exakten Wert $u'(x) = 0.40$.
]

== Wie fein muss das Gitter sein?

Eine Saite der Länge 10, die links im Takt hin und her bewegt wird. Gitterabstand $H$:

#align(center, image("figures/refinement.svg", height: 8.6cm))

== Genauigkeit kostet Rechenzeit

#boxed[Je kleiner der Gitterabstand $H$, desto *genauer*, aber auch desto *aufwendiger* ist die Simulation.]

  #v(0.5em)
  Anzahl Gitterpunkte in einem Würfel der Kantenlänge 1:

  #table(
    columns: (auto, auto, auto, auto),
    stroke: none,
    inset: 0.45em,
    align: right,
    fill: (_, row) => if row == 0 { mint } else if calc.odd(row) { mint.lighten(70%) },
    [$H$], [*1D*], [*2D*], [*3D*],
    [0.1], [10], [100], [1'000],
    [0.01], [100], [10'000], [1'000'000],
    [0.001], [1'000], [1'000'000], [1'000'000'000],
  )

== Komplizierte Formen: Finite Elemente

#slide(composer: (1fr, 1.6fr, 1.3fr))[
  #image("figures/fem_guitar.svg", height: 8.5cm)
][
  Echte Objekte sind keine Linien oder Quadrate.

  #v(0.3em)
  *Finite-Elemente-Methode (FEM):*
  - Objekt in viele kleine Dreiecke oder Würfel zerlegen
  - auf jedem Element eine einfache Funktion (z.B. eine Ebene)
  - der Computer setzt alles zu einem grossen Gleichungssystem zusammen

  #v(0.3em)
  Standard in der Technik: Autos, Brücken, Flugzeuge, Medizin, Computergrafik.
][
  #photo("images/car_mesh.png", [FEM-Gitter eines Autos])
]

= Chladni-Figuren

== Ernst Florens Friedrich Chladni (1756--1827)

#slide(composer: (1fr, 1fr, 1.2fr))[
  #photo("images/chladni_portrait.png", [Ernst Chladni], height: 6.5cm)
][
  #photo("images/chladni_bow.png", [Platte mit Geigenbogen angestrichen], height: 6.5cm)
][
  - 1787: «Entdeckungen über die Theorie des Klanges»
  - Sand auf einer Metallplatte, Platte mit dem Geigenbogen anstreichen → Muster!
  - Napoleon: *«Dieser Mann lässt die Töne sehen.»*
  - Er setzte einen Preis aus für die mathematische Erklärung.
]

== Stehende Wellen auf einer Saite

#slide(composer: (1.3fr, 1fr))[
  #image("figures/standing_waves.svg", width: 100%)
][
  Eine eingespannte Saite kann nur in ganz bestimmten *Formen* schön gleichmässig schwingen.

  #v(0.3em)
  - Jede Form hat ihre *eigene Frequenz*: Grundton $f_1$ und Obertöne $2 f_1, 3 f_1, ...$
  - Die #text(fill: red)[roten Punkte] bewegen sich *nie*: die *Knoten*.

  #v(0.3em)
  Darum klingt eine Gitarre nach einem bestimmten Ton und nicht nach Rauschen.

  #v(0.3em)
  #animation("animations/stehende_welle.html")[Animation: stehende Welle]
]

== Eigenschwingungen (Eigenfunktionen)

#align(center, image("figures/eigen_product.svg", height: 4.6cm))

#grid(
  columns: (1fr, 1fr),
  gutter: 1.5em,
  [
    Eine *Eigenschwingung* behält ihre Form $phi(x)$. Sie wird nur grösser, kleiner und klappt um, im Takt der Frequenz $f$.
  ],
  [
    *«Eigen»*: Jede Saite und jede Platte hat ihre *eigenen* Formen und Frequenzen, wie ein Fingerabdruck. Sie hängen von Form, Material und Befestigung ab.
  ],
)

#align(center, animation("animations/raumzeit.html?modus=eigen")[Animation: Eigenschwingung im Raum-Zeit-Bild])

== Warum nur bestimmte Frequenzen?

Setzen wir $u(x, t) = phi(x) cos(2 pi f t)$ in die Wellengleichung ein:

#align(center)[
  #boxed[$ -(2 pi f)^2 phi(x) = c^2 phi''(x) $]
]

#v(0.3em)
- Links: $phi$ mal eine Zahl. Rechts: die Krümmung von $phi$.
- Gesucht ist eine Form, deren *Krümmung überall proportional zur Form selbst* ist, und die an den Enden null ist.
- Das klappt nur für ganz bestimmte $f$: die *Eigenfrequenzen*. Die passenden Formen $phi$ heissen *Eigenfunktionen*.

#v(0.3em)
#align(center)[Saite: $phi_n (x) = sin(n pi x \/ L)$ #h(1em) und #h(1em) $f_n = n dot c \/ (2L)$]

#speaker-note[Optional, je nach Klasse. Die Kernaussage ist der letzte Punkt: nur bestimmte Frequenzen passen.]

== Resonanz

#slide(composer: (1fr, 1fr))[
  *Schaukel:* Wer im richtigen Takt anstösst, schaukelt immer höher. Im falschen Takt passiert fast nichts.

  #v(0.5em)
  *Chladni-Platte:* Der Lautsprecher schüttelt die Schraube in der Mitte mit der Frequenz $f$.
  - $f$ weit weg von einer Eigenfrequenz: Die Platte bewegt sich kaum.
  - $f$ nahe einer Eigenfrequenz: *Resonanz!* Die Platte schwingt stark, und zwar in der Form dieser Eigenschwingung.
][
  #boxed[
    Wenn Sie am Frequenzknopf drehen, «suchen» Sie die Eigenfrequenzen der Platte.

    #v(0.3em)
    Bei jeder Resonanz erscheint ein anderes Muster.
  ]
]

== Von der Saite zur Platte

#slide(composer: (1.4fr, 1fr))[
  #image("figures/plate_3d_and_sand.png", width: 100%)
][
  - Saite: Knoten sind *Punkte*.
  - Platte: Knoten sind *Linien*, die *Knotenlinien*.

  #v(0.5em)
  Der Sand wird von den bewegten Stellen weggeschleudert und bleibt dort liegen, wo sich die Platte *nicht* bewegt.

  #v(0.5em)
  #boxed[Chladni-Figur = Knotenlinien einer Eigenschwingung]
]

== Simulierte Chladni-Figuren

#align(center)[#image("figures/chladni_gallery.png", height: 8.4cm)]

#align(center)[Höhere Frequenz → mehr Knotenlinien → feineres Muster]

== Experiment und Simulation

#slide(composer: (1fr, 1fr))[
  #photo("images/sand_pattern_photo.png", [Experiment], height: 7cm)
][
  #photo("figures/chladni_single.png", [Simulation: Eigenschwingung bei 925 Hz], height: 7cm)
]

== Jetzt sind Sie dran!

#slide(composer: (1.2fr, 1fr))[
  + Drehen Sie die Frequenz *langsam* hoch. Wann entsteht ein Muster?
  + Notieren Sie die *Frequenz* und skizzieren Sie das *Muster*.
  + Vergleichen Sie mit der *Simulation*: Stimmen die Frequenzen? Wenn nicht, woran könnte es liegen?
  + *Knobelfrage:* Manche simulierten Muster tauchen im Experiment nie auf. Warum?

  #v(0.5em)
  #text(size: 16pt)[Tipp: Lautstärke nicht zu hoch, sonst springt der Sand nur noch herum.]
][
  #photo("images/setup_diagram.png", [Versuchsaufbau])
  #speaker-note[Auflösung der Knobelfrage: Der Lautsprecher drückt nur in der Mitte. Eigenschwingungen, die in der Mitte nicht angeregt werden (z.B. antisymmetrische), bekommen keine Energie und erscheinen deshalb nicht.]
]

#focus-slide[
  Vielen Dank! \
  #text(size: 0.7em)[Fragen?]
]
