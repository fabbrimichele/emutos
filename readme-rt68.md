# Build
```shell
make rt68
```

# Useful infos
* https://github.com/emutos/emutos/compare/master...aslak3:emutos:master
* https://github.com/emutos/emutos/pull/29/files
* https://sourceforge.net/p/emutos/mailman/emutos-devel/thread/5100478E.2050207%40ist-schlau.de/#msg30391800

#ROM boot address
The ROM image of EmutTOS contains in the first 4 bytes the SP and in the following 4 the PC.
That should be compatible with RT68 ROM boot mapping performed by the GLUE CPLD.