# zk

This is a Zettelkasten. It kastens Zettel.

zk is a small notekeeping software inspired by the Zettelkasten (note box) system as used by Jules Vernes and Niklas Luhmann. You can create notes, tag them and print out notes with certain tags.

# HOWTO
Before first time usage, `config.h` must be adjusted to your need, then the software must be compiled with `make`. You can install it with `make install`. Change the install path in the makefile if you want something different.

To create a new Zettel, use `zk add`. This opens a new Zettel file with your chosen editor.
The first lines are the Zettel content.
Tags are written in lines marked with a `#` symbol.
This is an example:

```
I am an example Zettel.
This system is really great for keeping short notes.

#Tag1
#Tag2
#Zettel
#Test
```

To list all existing tags, type `zk tags`. To print all Zettel with a certain tag, type `zk browse [tag]`.

Instead of `zk add`, which opens an editor from the zk software, you could also use something like ``` vim ~/`date +%Y_%m_%d_%H_%M_%S` ```
