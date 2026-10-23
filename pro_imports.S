/* Unico import especifico do PRO-C; dispensa instalar o SDK do PRO. */
.set noreorder
#include "pspimport.s"
/* Igual ao stub original do PRO: versao minima 0, import fraco.
 * 0x00010011 exigia versao 0.17 e podia impedir module_start. */
IMPORT_START "SystemCtrlForKernel", 0x00090000
IMPORT_FUNC "SystemCtrlForKernel", 0x159AF5CC, sctrlHENFindFunction
