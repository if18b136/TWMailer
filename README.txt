Please write the folder name for the directory only - no "/" afterwards - it gets attached automatically.

Two upfront problems that still persist within the code:
1. Buffer is still static - so mach msg input is at about 950 chars.
2. We forgot to add a number update function after a message is deleted - so the old message numbers won't change atm.


@TODO

UE1
- dyn buffer
- zahlen neu vergeben nach del
- Nachdem eine leere Nachricht verschickt wurde, kann es zu einem Speicherzugriffsfehler(SIGSEGV) kommen
- immer dann, wenn falsche Benutzernamen eingegeben wurden, werden neue leere txt-Files erzeugt, nach kurzer Zeit ist das gesamte Verzeichnis voller "toter" Textfiles (z.B. DEL.txt, ..txt, asdfasdf.txt, ...)

UE2
- Thread catching at the end 
- LDAP Anmeldung Cleanup 
- LDAP Anmeldung Rueckmeldung an CLient
- Sperre der IP nach 3mal falscher Eingabe


// INFO - LDAP zuerst anonyme anmeldung, dann nach userdaten suchen, wenn gefunden, mit den Userdaten anmelden. Distuingished name bauen (= Pfad)
// INFO - Testen ausserhalb nur mit VPN Tunnel
