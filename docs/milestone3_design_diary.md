# Design Diary

Use this space to talk about your process.  For inspiration, see [my prompts](../../../docs/sample_reflection.md) 

figuring out how to "connect" the underlying data structure with the
temporary surface display of characters on the window is challenging.
it involves a lot of playing around with indexing vectors. i think the
best way to save changes is that whenever something changes the screen
(ie scrolling down, up, save button), save the current screen to a vector.
a challenge is noticing changes when necessary, as well as when new text
is typed somewhere, i have to go to the correct line(s) and column(s) and
then go to the correct place in the string in the line(s).

it has been an incredible challenge and very frustrating at times working with vectors. 
fully accomplishing this milestone will take me beyond the due date without a doubt.


