figuring out how to work with the priority queue was fairly simple since there was a standard library that essentially does all the heavy-
lifting of figuring out where to place each node. The most difficult part was actually implementing it to work with ncurses, but aside from
that, it felt much more straightforward that most of the other milestones. 
I will say that this milestone somehow inspired me to further refactor the code to implement classes for a lot of things that I feel are kind
of "floating around" in main. It would be much cleaner if I can compartmentalize most of my code into classes. That way, instead of 
converting my document by first saving it to a file AND THEN converting, I can convert it straight from the Text Editor because I'll then
have a better handle on it.It'll also just clean up the code to look nicer, read better, and probably have a little more flexibility with 
the actual implementation of future features.
