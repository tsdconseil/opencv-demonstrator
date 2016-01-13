# Software architecture

To be written.


## Project directories

Source file are organized in 3 main folders:

 - ocvdemo: OpenCV demonstrator project
 - ocvext:  Some additional image processing routine (Deriche gradient and Hough transform)
 - libcutil: Helper classes to generate a MMI from a XML file
    1.Multimodal Interaction Activity (MMI)  https://en.wikipedia.org/wiki/Multimodal_interaction
    2.XML parcing with Pugixml https://github.com/zeux/pugixml

## Configuration files

(**note:** what follows is just a cut/past of what was in build instructions, this need to be, and will be, better explained) 
- Only when this folder does not exist, then the application will look in the "/usr/share/ocvdemo/data/" folder for the
constant data files. But the data files should have been copied there through the installer / .deb package (which is not
written).
- There is another data folder that will be used by the application, where user specific files will be stored
(application journal, application configuration, language selection). This folder may be:
  * ~/.ocvdemo on Linux, where ~/ is the current user home folder.
  * c:/appdata/.../current user/.../roaming/ocvdemo on Windows (the exact path is dependent on Windows version).
