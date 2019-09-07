!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Events.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Events.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Events.dll"
	-@erase "$(OUTDIR)\MQ2Events.exp"
	-@erase "$(OUTDIR)\MQ2Events.lib"
	-@erase "$(OUTDIR)\MQ2Events.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\Detours\lib60\detours.lib ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Events.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Events.dll" /implib:"$(OUTDIR)\MQ2Events.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Events.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Events.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Events.dep")
!INCLUDE "MQ2Events.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Events.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Events.cpp

"$(INTDIR)\MQ2Events.obj" : $(SOURCE) "$(INTDIR)"

