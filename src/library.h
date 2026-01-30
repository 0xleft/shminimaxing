#ifndef SHMINIMAXING_LIBRARY_H
#define SHMINIMAXING_LIBRARY_H

#include <jni.h>

#ifndef _Included_nl_utwente_quarto_ai_ShminimaxedAIPlayer
#define _Included_nl_utwente_quarto_ai_ShminimaxedAIPlayer
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     nl_utwente_quarto_ai_ShminimaxedAIPlayer
 * Method:    evalBoard
 * Signature: ([CCI)I
 */
    JNIEXPORT jchar JNICALL Java_nl_utwente_quarto_ai_ShminimaxedAIPlayer_getBestMove
(JNIEnv*, jobject, jcharArray, jchar, jint, jint);

#ifdef __cplusplus
}
#endif
#endif

#endif // SHMINIMAXING_LIBRARY_H
