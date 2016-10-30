# AntiMine
Simple mine sweeper bot (OpenCV)

Works for minesweeper from Windows XP.

## How to use
Run. Select region of minesweeper: field should be larger than half of it. Do not move minesweeper window out of selected region.
When selecting region be sure that all cells completely visible (sometimes console window overlap it).

## Info
One cell per step, it doesn't use "check all neighbours" feature.

Timing:
Noob: 1sec
Medium: 3sec
Pro: 8sec

With defined PARSE_SINGLE time is constant, because it checks every cell (one-cell view).