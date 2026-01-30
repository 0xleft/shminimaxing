# shminimaxing

## Build


```bash
cmake . DJAVA_HOME=<your java home>
cmake --build . --target shminimaxing --config Release
```

## Usage

Call `getBestMove` from in java. Make sure you have the library loaded using `System.loadLibrary("shminimaxing")`.

```java
/**
 * The function assumes that the current state is where next expected move is a {@link PlacementMove} type.
 *
 * @param boardState     the 80 bit serialization of a board
 * @param selectionState the 16 bit selection state of the board
 * @param timeLeft       how much time is left to evaluate this position in milliseconds (this doesn't
 *                       guarantee that it will be done in that amount, as it could return quicker)
 * @return the upper 4 bits being the move and the lower 4 bits being the selection mvoe
 */
public native char getBestMove(char[] boardState, char selectionState, int selectedPiece, int timeLeft);
```

## Performance

Both monte carlo and minimax searches are multi threaded up to 16 threads. Monte carlo shares one tree for all the threads, which makes it slower due to locking, however this allows for way deeper search of the tree.

During the tournament we noticed that our monte carlo was blundering in certain cases, therefore its implementation should probably not be trusted.

Full game (meaning it will return the true best move) solution in ~3 seconds at 7 pieces on the board, and ~20 seconds for 6 pieces on the board.
