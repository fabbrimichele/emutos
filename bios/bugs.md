From time to time when moving windows around or resizing them I get Panic error.
So far I saw Panic: Line F Emulator. It only happens running at about 32MHZ, at 25MHz it does not happen.

For example:
```
Panic: Line F Emulator
sr=2008 pc=00391ee4

D0-3: 00378162 00005555 00000001 ffffffff
D4-7: 0000ffff 00000001 0000019e 00000000
A0-3: 00378162 00378194 0000ace2 0000acf0
A4-7: 00000000 00000002 00000018 0000ac4c

 USP: 00000000



basepage=00025b66
text=003a4940 data=00000000 bss=00000000
```

the PC seems to fails often at this line: 00391ee4