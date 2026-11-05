# COMPE571 PA3 - Real-time scheduler with DVFS

Requires Python 3.9+ (no external packages).

    python3 scheduler.py <input_file> <EDF|RM> [EE]

Examples:

    python3 scheduler.py input1.txt EDF
    python3 scheduler.py input1.txt RM
    python3 scheduler.py input1.txt EDF EE
    python3 scheduler.py input1.txt RM EE

Output lines: `<start> <task|IDLE> <freq|IDLE> <duration> <energy>J`, followed by
total energy, percent idle, and total execution time. Any deadline miss is
printed as a WARNING line.
