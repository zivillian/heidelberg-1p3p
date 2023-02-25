# Phasenumschaltung für Heidelberg Energy Control

## Warum?

Die Wallbox unterstützt von Hause aus keine automatisch Umschaltung zwischen 1 und 3-Phasen Betrieb. Dabei kann die Wallbox sowohl 1 als auch 3-phasig angeschlossen werden. Um den Ertrag bei PV Überschussladen zu steigern, kann es insbesondere im Winter oder der Übergansgzeit hilfreich sein die Anzahl der Phasen je nach verfügbarer Leistung umzuschalten.

Viele behelfen sich mit einem Lasttrennschalter und schalten manuell um, aber da [evcc](https://evcc.io/) die automatische Umschaltung unterstützt wollte ich das auch für meine Wallbox nachrüsten.

## Wie?

Zwischen die Sicherung und die Wallbox kommt ein Kasten mit 2 Schützen, einem ESP32, einem RS485-TTL Adapter und zwei Solid State Relais. EVCC spricht mit dem Mikrocontroller ModbusTCP und erreicht darüber auch die Wallbox. Über ein zusätzliches Register [könnte EVCC die Anzahl der genutzten Phasen auslesen und setzen](https://github.com/evcc-io/evcc/discussions/6168#discussioncomment-4925261).

Beim Umschalten wird durch den ESP zuerst die Ladeleistung auf 0 reduziert und darauf gewartet, dass das Fahrzeug nicht mehr lädt. Anschließend wird die Wallbox komplett vom Strom getrennt und nach einer Pause von 2 Sekunden mit der gewünschten Phasenkonfiguration neu gestartet. Aus Sicht des Fahrzeugs gab es einen kurzen Stromausfall. Dadurch, dass die Ladeleistung vorher auf 0 reduziert wurde, wird nicht unter Last geschaltet.

## Funktioniert das wirklich?

**Keine Ahnung!**

Ich habe meinen Prototyp gebaut und mit einer simulierten Wallbox über ModbusTCP getestet - das sah nicht verkehrt aus. Der nächste Schritt ist den Code auf ModbusRTU umzustellen, den Kasten zwischen die Wallbox und Sicherung zu bauen und:

1. mit EVCC ohne Umschaltung zu testen
1. die manuelle Umschaltung ohne Fahrzeug testen
1. die manuelle Umschaltung mit Fahrzeug ohne Ladevorgang testen
1. die manuelle Umschaltung mit Fahrzeug bei aktivem Ladevorgang testen
1. die automatische Umschaltung in EVCC implementieren
1. auf Sonne warten und die automatische Umschaltung beobachten
1. *Party*

## Darf ich das?

**Nope!**

Nach [§13 NAV](https://www.gesetze-im-internet.de/nav/__13.html "Niederspannungsanschlussverordnung ") darf das ...
> [...] nur durch ein in ein Installateurverzeichnis eines Netzbetreibers eingetragenes Installationsunternehmen durchgeführt werden [...].

## Ist das sicher?

Da jegliche Kommunikation mit der Wallbox über den ESP läuft ist sichergestellt, dass während des Umschaltvorgangs keine anderen Befehle an die Wallbox gesendet werden.

In der Steuerung sind der Leistungsteil und die Steuerung strikt getrennt - der Mikrocontroller steuert über seine GPIOs nur die Solid State Relais, die wiederrum die Schütze ansteuern. Durch die SSRs ist die galvanische Trennung sichergestellt. Über Hilfsschalter an den Schützen kann der Mikrocontroller den aktuellen Zustand auslesen - durch die Hilfsschalter sind auch die Strompfade zwischen AC und DC physisch getrennt. Die beiden Schütze sind elektrisch gegeneinander verriegelt - es kann also immer nur ein Schütz angezogen sein. Eine versehentliche Umschaltung während eines Ladevorgangs ist also ausgeschlossen. Selbst im Worst-Case wird die Wallbox durch den Aufbau in jedem Fall (wenn auch nur sehr kurz) vom Strom getrennt bevor sich die Phasenkonfiguration ändert - damit muss jedes Fahrzeug umgehen können.

## Kann ich mal sehen?

Klar!

![Bild vom inneren des Kasten](docs/kasten.jpg)

## Aber, warum ...?

### ... Schütze wenn die SSRs auch 25A können?

Solid State Relais haben zum einen immer einen Kriechstrom - damit wäre die Trennung der Wallbox nicht mehr sichergestellt. Zum anderen erzeugen sie bei Last relativ viel Abwärme - normalerweise kommen da Kühlkörper drunter die größer sind als die Relais.

### ... SSR und nicht einfache Relais?

Die Hoffnung ist, dass die länger halten, weil es keine beweglichen Teile gibt.

### ... Hilfsschalter an den Schützen?

Ja, der ESP weiß was er auf den GPIOs ausgegeben hat und könnte einfach dran glauben, dass der Rest sich wie geplant verhält. Bei den ersten Tests hat sich aber schon gezeigt, dass die Schütze noch einige Sekunden angezogen bleiben, nachdem der GPIO auf `LOW` gesetzt wurde. Darüber hinaus lässt sich ein klebendes Schütz eindeutig erkennen.

### ... nicht einfach NodeRED und ein Shelly

Die Gesamtleistung der meisten Shelly liegt unter den benötigen 16A bzw. 48A (3 * 16A) bei 230V. Das Hauptproblem ist aber, dass die Kommunikation mit der Wallbox am Shelly vorbei läuft - dadurch ist es möglich, dass kurz vor dem Umschalten der Ladevorgang wieder gestartet wird und das Fahrzeug es nicht verträgt, dass während eines laufenden Ladevorgangs Phasen verschwinden oder zugeschaltet werden. Im schlimmsten Fall fängt es an zu brennen.

### ... ist dann mein Auto abgefackelt?

Das kann dir [dein Elektriker](#darf-ich-das) sicher beantworten.

### ... hast du nicht *XYZ*, dass wär doch viel einfacher / besser / schöner?

Wenn du Ideen hast, [immer her damit](https://github.com/zivillian/heidelberg-1p3p/issues/new).

## Einkaufszettel

Folgende Artikel hab ich verbaut (**keine** Affiliate Links):
| Beschreibung                      | Artikel
| --------------------------------- | -------
| Schütz 1S/1Ö 25A                  | [Hager ESC227](https://hager.com/de/katalog/produkt/esc227-installationsschuetze-25a-1s1oe-230v)
| Schütz 3S/1Ö 25A                  | [Hager ESC428S](https://hager.com/de/katalog/produkt/esc428s-installationsschuetz-brumm-25a-3s1oe-230v)
| Hilfsschalter                     | [Hager ESC080](https://hager.com/de/katalog/produkt/esc080-hilfsschalter-6a-1s1oe)
| 5V Netzteil                       | [MeanWell HDR-15-5](https://www.amazon.de/dp/B06XWQSJGW)
| Solid State Relais (3VDC / 230AC) | [XRE-1D4825](https://www.amazon.de/gp/product/B0911X3KGL/)<br>(die gibt's in zig Varianten und 25A sind viel zu viel, aber diese kommt mit Berührungsschutz, ist ein 2er Pack, es gibt passende [Hutschienenadapter](https://www.amazon.de/gp/product/B09YKX8DCP) und es war das billigste Modell)
| Mikrocontroller                   | [ESP32 DEVKITV1](https://www.amazon.de/dp/B09QXB1PG5) (jeder ESP32 sollte tun - das Devkit hat keine störenden Stiftleisten und Befestigungslöcher)
| RS-485 TTL Adapter                | [Jopto TTL485-V2.0](https://www.amazon.de/dp/B096ZXXKCR) (mit Befestigungslöchern - die Stiftleiste hab ich gegen Schraubklemmen getauscht)
| Befestigung RS-485 und ESP        | Das ist der Punkt, wo du kreativ werden musst. Ich hatte noch einen alten NH-Sicherungsblock mit genieteter Hutschienenklemme, Alublech, Nieten und M3-Abstandhalter rumliegen und habe mir was gebastelt.
| AC Verteilung / Klemmen           | [Phoenix PT-6](https://www.phoenixcontact.com/de-de/produkte/durchgangsklemme-pt-6-3211813) ([Deckel](https://www.phoenixcontact.com/de-de/produkte/abschlussdeckel-d-pt-6-3212044), [Brücken](https://www.phoenixcontact.com/de-de/produkte/steckbruecke-fbs-2-8-3030284) und [Endhalter](https://www.phoenixcontact.com/de-de/produkte/endhalter-clipfix-35-5-3022276) nicht vergessen)<br>Es geht auch PT-4, aber du willst die Wallbox in jedem Fall mit ausreichendem Querschnitt anschließen und kein 6mm² in die PT-4 fummeln.
| DC Verteilung / Klemmen           | [Phoenix PT 2,5](https://www.phoenixcontact.com/de-de/produkte/durchgangsklemme-pt-25-3209510) ([Deckel](https://www.phoenixcontact.com/de-de/produkte/abschlussdeckel-d-st-25-3030417) und [Brücke](https://www.phoenixcontact.com/de-de/produkte/steckbruecke-fbs-2-5-3030161) nicht vergessen)
| Kabel                             | [H07 V-K 6mm²](https://www.hornbach.de/shop/Aderleitung-H07-V-K-1x6-mm-schwarz-Meterware/5188575/artikel.html) (schwarz) für den Leistungsteil
| Kabel                             | H07 V-K 1,5mm² [blau](https://www.stex24.de/124831-h07v-k-einzelader-pvc-dunkelblau-4520141) und [braun](https://www.stex24.de/124829-nyy-o-einzelader-pvc-braun-4520031) für die Ansteuerung der Schütze
| Kabel                             | [H05 V-K 0,5mm²](https://www.stex24.de/124802-h05v-einzelader-pvc-orange-4510091) für den DC Teil
