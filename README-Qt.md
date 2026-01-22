# Qt VU Meter Simulator

Questo simulatore Qt riproduce graficamente il comportamento del VU meter firmware Pulsar, sia in modalità demo che tramite dati reali su porta seriale.

## Modalità di utilizzo

### 1. Modalità Demo

Lancia il simulatore in modalità demo:

```sh
./qtmeter/qtmeter --demo
```

Per simulare la modalità reverse (led accesi dall'alto verso il basso):

```sh
./qtmeter/qtmeter --demo --reverse
```

### 2. Modalità Serial (con socat)

Per simulare la comunicazione seriale su Linux:

1. Crea due seriali virtuali collegate tra loro:

```sh
socat -d -d PTY,link=./vcom,raw,echo=0,mode=666 PTY,link=./vCOM,raw,echo=0,mode=666
```

2. Avvia il simulatore Qt collegato a una delle due seriali:

```sh
./qtmeter/qtmeter ./vcom
```

3. Invia dati all'altra seriale (ad esempio con uno script Python o con `echo`):

```sh
echo -ne '\xAB\xBA\x32\x00' > ./vCOM
```

- Il simulatore aggiornerà il VU meter in tempo reale in base ai dati ricevuti.
- Il comando seriale `0x01` imposta la modalità reverse (led accesi dall'alto).

## Caratteristiche
- Visualizzazione verticale, 12 led, colori: verde, giallo, arancio, rosso.
- Comportamento identico al firmware Pulsar (mapping valori/led, reverse, spegnimento a 0).
- Debug attivabile da console.
- Finestra Qt non ridimensionabile, VU meter ancorato in basso.

## Note
- Funziona su Linux, richiede Qt5 e permessi di accesso alle seriali virtuali.
- I file `moc_*` sono generati automaticamente e non sono tracciati da git.
