#include "library.h"

#include "game.h"

JNIEXPORT jchar JNICALL Java_nl_utwente_quarto_ai_ShminimaxedAIPlayer_getBestMove
(JNIEnv* env, jobject thisObject, jcharArray boardStateArr, jchar selectionState, jint selectedPiece,
 jint timeRemaining)
{
    const jchar* boardState = env->GetCharArrayElements(boardStateArr, nullptr);

    const auto game = quarto::game(boardState, selectionState, selectedPiece);
    // game.print_state();
    const uint16_t evaluation = game.compute_move(timeRemaining);

    env->ReleaseCharArrayElements(boardStateArr, const_cast<jchar*>(boardState), JNI_ABORT);

    return evaluation;
}
