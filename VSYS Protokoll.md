# VSYS: Beschreibung des verwendeten Kommunikationsprotokolls

## SEND
Mit dem Send Befehl, kann ein User Sender, Empfänger, Betreff und Nachricht eingeben und diese an den Server senden.
Bevor der Send Befehl etwas an den Server sendet, wird der User Input zuerst client-seitig überprüft. Dies erfolgt über das einlesen des Userinputs auf einen String (input\_str). Anschließend wird dieser, auf Korrektheit (maximale Länge) überprüft und in einen Buffer gespeichert.Gibt der User einen inkorrektn Input an, wird der input\_str geleert und der User kann den Input wiederholen. Der input\_str wird zurückgesetzt. Dies wird für Sender Empfänger, Betreff und Nachricht wiederholt. 
Sobald der User die Nachricht mit '.\n' beendet, wird dies als letzter string auf den Buffer geschrieben und per send() über die Socketverbindung an den Server gesendet.
Der Server empfängt mit recv() den Buffer und beginnt per strtok() die empfangene Nachricht Stück für Stück auf eine Zwischenvariable 'token' zu legen.


## LIST
Wie bei den vorherigen Befehlen bereits beschrieben, wird auch hier der User-Input überprüft und anschließend auf einen buffer gelegt. Bei LIST wird ausschließlich die der UID des geswünschten Users abgefragt. Diese wird in den Buffer gelegt und an den Server gesendet. 
Der Server zerlegt den empfangenen Buffer wieder und bespielt die 'username' Variable mit der gesendeten UID. Das zugehörige File wird vom Server geöffnet und per getline() Befehl durchiteriert. Die Anzahl der Nachrichten wird ausgelesen und anschließend in den Buffer gelegt. Der Buffer wird mit send() über die Socketverbindung wieder zurück an den Client geschickt und dort ausgegeben.

## READ
Wie bei den vorherigen Befehlen bereits beschrieben, wird auch hier der User-Input überprüft und anschließend auf einen buffer gelegt. Bei READ wird zuerst nach der gewünschten UID gefragt und anschließend nach der Nummer der gewünschten Nachricht. Nachdem dies erfolgreich auf den Buffer geschrieben wurde, wird dieser per send() über die Socketverbindung an den Server gesendet.
Der Server liest UID und Nachrichtennummer aus dem Buffer. Das zur UID gehörende File wird geöffnet und es wird bis zur gewünschten Nachricht durchiteriert. Diese wird Zeile für Zeile ausgelesen und zurück in den Buffer gelegt. Der Buffer wird mit send() über die Socketverbindung wieder zurück an den Client geschickt und dort ausgegeben.

## DELETE
Wie bei den vorherigen Befehlen bereits beschrieben, wird auch hier der User-Input überprüft und anschließend auf einen buffer gelegt. Der DELETE Befehl verlang wie auch schon der READ Befehl nach einer UID und der Nummer einer Nachricht. Sind diese korrekt auf den Buffer gespielt, sendet der Client anschließend diesen per send() Befehl an den Server.
Der Server löscht die zu löschende Nachricht (hierfür wird alles außer der Nachricht kopiert und auf ein neues File gespielt, welches dann das alte ersetzt). Zuletzt wird 'OK\n' auf den Buffer gespielt und mit send() über die Socketverbindung zurück an den Client geschickt und dort ausgegeben.
Sollte die gewünschte UID nicht vorhanden sein oder die Nachricht aus einem anderen Grund nicjt gelöscht werden, wird 'ERR\n' auf den Buffer gespielt und mit send() über die Socketverbindung zurück an den Client geschickt und dort ausgegeben.

## LOGIN
Wie bei den vorherigen Befehlen bereits beschrieben, wird auch hier der User-Input überprüft und anschließend auf einen buffer gelegt. Es wird zuerst nach der UID (ifxxbxxx) gefragt und anschließend nach dem dazugehörigen Passwort. Die Zeichen des Passworts werden dabei beim Schreiben durch '*' ersetzt. 
Der Buffer wird danach über die Socketverbindung and den Server gesendet.
Es wird wie bereits beschrieben Befehlsart und Username aus dem Buffer ausgelesen und auf die jeweils zugehörigen Variablen gespeichert. Anschließend erfolgt der LDAP Anmeldeversuch. Je nach Ausgang des Anmeldeversuchs wird einer von zwei Kommunikationsprotokollen an den Client zurückgesendet. 
1. Erfolgreicher Anmeldeversuch
    In diesem Fall wird ein String mit 'OK\n' in den Buffer gelegt und per send() an den client verschickt.
2. Erfolgloser Anmeldeversuch
    In diesem Fall wird zuerst ein String mit 'ERR\n' in den Buffer gelegt und per send() an den client verschickt. Der Client wartet auf eine zweite Nachricht. Diese determiniert, ob die vom Client verwendete IP-Adresse blockiert wird oder noch Anmeldeversuche frei sind. Bei Blockieren der IP-Adresse legt der Server '3\n' in den Buffer. Bei verbleibenden Anmeldeversuchen '!3\n'.
## QUIT
Wenn der User den Befehl 'QUIT' eingibt, wird die Socketverbindung geschlossen und der Clientprozess terminiert.