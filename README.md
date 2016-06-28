# getrlimit/setrlimit OS X Bug

A simple test for an annoying OS X bug.

On OS X, the limit on the number of files opened using fopen()
cannot be changed after the first call to stdio (e.g. printf() or fopen()).

Consequently, this has to be the first thing to do in main().
