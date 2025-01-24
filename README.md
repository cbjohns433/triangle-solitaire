<header>

# The Triangle Solitaire puzzle

This is a small triangular puzzle which may be familiar to anybody who
has eaten at a Cracker Barrel restaurant in the US.

The objective is to make a series of moves that result in only a single peg
being left, where each move is a jump over a peg into a hole beyond, and
where the jumped over peg is removed.

For an example of this, see https://blog.crackerbarrel.com/2021/08/13/how-to-beat-the-cracker-barrel-peg-game/

</header>

## Building the code

On macOS or Linux, simply do:

cc -o tri_solitaire tri_solitaire.c

## Running the code

Running the binary with no command-line options will solve the puzzle and display the
move-by-move solution on the screen, where the last peg to be moved each time is
highlighted in reverse video.

To get an interactive step-by-step display, use the -v option.

For information on how the code is working as it computes the solution, use the -d option.
