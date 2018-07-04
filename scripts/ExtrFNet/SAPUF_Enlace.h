#include "tipos.h"

#ifdef __cplusplus
extern "C"
{
#endif

int RecuperarMsg(LPSTR resp, LPSTR Error_Code, LPSTR SubError_Code);
int SolicitarPassword(LPSTR lpstrOperacion,LPSTR lpstrMaqDestino,
		      LPSTR lpstrUsuarioDestino,LPSTR lpstrTipoDestino,
		      LPSTR lpstrAplDestino,LPSTR lpstrAplOrigen, 
		      LPSTR lpstrAtributo,  LPSTR Timer);

#ifdef __cplusplus
}
#endif
