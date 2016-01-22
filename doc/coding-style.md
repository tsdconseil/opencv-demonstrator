# Coding style rules

This document describe the coding rules to be applied in the project, so as to preserve coherency.


## GIT repository usage rules

The following rules come in addition to common practice:

- Comment each commit with a meaningful, synthetic description. A description should not be "updated this.file", but
should describe what has been done.
- A commit should not be split into artificial multiple commits with the same description. I.e. if you have one feature
and it is split into several commits, please
[squash them into one commit with Git](https://davidwalsh.name/squash-commits-git) before pushing.
- A commit comment should begin with [...], where ... is the module / feature / file impacted by the modification
- A commit comment should not necessarily specify the list of impacted files (it is already listed by Git)

Counter example (not to be done):

- commit 1: updated afile.cc
- commit 2: updated afile.cc

Good example:

- commit 1: [video] Added management of [a feature] in the video processing toolchain
- commit 2: [mmi] Automatic resize of the display window


## C++ coding rules

- Documentation: [Doxygen format](http://www.doxygen.org)
- Indentation: no tabulations, only spaces, and 2 spaces by indentation level.
- All variables, functions, member functions and variables, should be written in lowercase letters, and if an object
name is made of several words, these should be seperated by the `_` symbol. For instance: `variable_1`, `my_function`.
- All classes and structures names should be written according to the "CamelCase" convention (first letter in
uppercase, seperation by the uppercase)
- All files should be named in lowercase letters, with the different words separated by a `-` symbol.
- All text based files should use utf-8 encoding, and Unix style line endings. Unix uses just line feed ("\n").
  1.  [See issue #5](https://github.com/tsdconseil/opencv-demonstrator/issues/5)
  2.  See also [Github's advice](https://help.github.com/articles/dealing-with-line-endings/)
  3.  Note also the file .gitattributes in the root directory.
- Valid XML files.


## Rules for .md files
- ![markdown](img/md-39x24-solid.png)[use Girhub markdown](https://guides.github.com/features/mastering-markdown/)

Other rules may be added in the future.

Any suggestion welcome!
