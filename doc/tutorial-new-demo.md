# Tutorial for adding a new demonstration

This document describes how one can add its own demonstration in the software. 

## Step 1: Updating the table of contents (`data/model.xml`)

This XML file is organized as a list of **categories** (XML node `cat`), where each categories correspond to a folder in the demonstrator selection tree, and can contain other sub-categories or demonstration items (XML node `demo`).

So if a revelant category already exists in the model, please add your demonstration there. Otherwise, you can create your own category. A category is described by its identifier (short name), and a short, localized, label (please provide at least french and english translation). For instance, the *basic image processing* category is defined as:

```
<cat name = "basic"
     fr   = "Traitements d'image simples"
     en   = "Basic image processing">

  <!-- List of sub-categories and demo items -->

</cat>
```

A demonstration item is a XML node of type `demo`, and with the same attributes as a category (name identifier and localized labels), and another optionnal attributes, `default-img`, which specify the path of the default image to use for this demonstration (if not specified, a Lena image is used). 

For instance, to create a demonstration of id `dummy`, using the `data/img/box.png` file as default input image:

```
<demo name = "dummy" 
      fr   = "Ma démonstration" 
      en   = "My demonstration"
      default-img="data/img/box.png"/>
```
  
## Step 2: Defining the schema with the demonstration parameters and description 
At the end of the file `data/schema.xml`, just before the closing `</schema>` tag, add a node for your demonstration:

```
<node name="dummy">
 <description lang="en">Here put a longer description of your demo (possibly several sentences). Limited html markup is supported, due to GTK / Pango limitations (<i></i>, <b></b>). Do not use <br/> to add newline, but just use carriage return.</description>
</node>
```

Note: replace `dummy` by the identifier (`name` attribute) that you had used in the `ocvdemo/data/model.xml` file.

## Step 3: Implementing your demonstration

- Copy the file `ocvdemo/include/demo-items/demo-skeleton.hpp' as `ocvdemo/include/demo-items/mydemo.hpp', where `mydemo` is the name of your demonstration (without capital letter)
- Copy the file `ocvdemo/src/demo-items/demo-skeleton.cc' as `ocvdemo/src/demo-items/mydemo.cc'
- Follow the instructions you can find in these two files so as to customize them for your demonstration

## Step 4: Makefile update
In the `ocvdemo/Makefile` file, add the name of your source file at the end of the list of source files to be compiled (the list is defined in the constant `SOURCES_BASE`). Do not include the extension of the source file (as it is implied that it is `.cc`).

## Step 5: Registering your demonstration class
In the source file `ocvdemo/src/demos-registration.cc`, add a line for your demonstration class.

## Step 6: Rebuilding and checking
Rebuild the project (type `make` in the folder `ocvdemo`), and run the executable to check that your demonstration works correctly.

