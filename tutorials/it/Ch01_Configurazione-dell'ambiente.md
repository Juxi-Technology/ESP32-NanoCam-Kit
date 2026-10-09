# Capitolo 1: Configurazione dell'ambiente

## 1.1 Ambiente per il flashing del firmware

### Metodo A: senza ambiente di sviluppo (consigliato ai principianti)

1. Installa il [driver seriale CH340K](https://www.wch.cn/download/CH341SER_EXE.html)
2. Apri il browser → [esptool-js](https://espressif.github.io/esptool-js/)
3. Collega il NanoCam, seleziona la porta seriale e il file firmware .bin
4. Fai clic su Program per eseguire il flashing

### Metodo B: riga di comando

```bash
pip install esptool
esptool.py --chip esp32s3 --port COM_x write_flash 0x0 nanocam_xxx.bin
```

### Metodo C: ambiente di sviluppo ESP-IDF (avanzato)

1. Installa VSCode + l'estensione ESP-IDF
2. F1 → `ESP-IDF: Configure ESP-IDF Extension`
3. Seleziona o installa ESP-IDF v5.4+
4. Compila: `idf.py build flash monitor`

### Metodo D: installazione tramite ESP-EIM-GUI

1. Scarica dal sito ufficiale: https://dl.espressif.cn/dl/eim/
2. Dopo il download, fai doppio clic per aprire la pagina EIM; nell'angolo in alto a destra puoi passare alla versione cinese
3. Fai clic su Avvia installazione
4. Al passaggio successivo seleziona l'installazione personalizzata
5. Prima di procedere è necessario aver installato `git` e `python3.12.x` (sorgente di download per la Cina per git: [CNPM Binaries Mirror](https://registry.npmmirror.com/binary.html?path=git-for-windows/v2.55.0.windows.3/))
6. Seleziona esp32s3 come dispositivo di destinazione
7. Alla scelta della versione di ESP-IDF occorre selezionare "mostra le vecchie versioni stabili" e, scorrendo verso il basso, scegliere la versione v5.4.1
8. Alla voce del mirror da scaricare non modificare nulla, poi vai avanti
9. Alla voce delle funzionalità di ESP-IDF si consiglia di selezionare tutto, poi continua
10. Alla voce degli strumenti vai avanti, poi scegli la posizione in cui installare e attendi il completamento
Dopo l'installazione, questa versione presenterà un problema di scompattazione: trova la directory C:\Espressif\dist\xtensa-esp-elf-14.2.0_20241119-x86_64-w64-mingw32.zip, copia l'archivio in C:\Espressif\tools\xtensa-esp-elf, dopo averlo scompattato individua la cartella xtensa-esp-elf e sostituisci la cartella presente nella directory C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119: la compilazione andrà quindi a buon fine

## 1.2 Ambiente server

### Servizio ufficiale xiaozhi.me (gratuito)

1. Visita [xiaozhi.me](https://xiaozhi.me) e registra un account
2. Accedi alla console
3. Dopo la connessione alla rete, il modulo annuncia un codice di verifica di 6 cifre
4. Fai clic su "Aggiungi dispositivo" a destra della sezione "Agente"
5. Inserisci il codice di verifica di 6 cifre annunciato
6. Dopo aver associato il dispositivo puoi iniziare a conversare
