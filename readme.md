<div align=center>
    <img width="150px" src=assets/logo.png>
</div>
<h3 align=center>One Click Quits</h3>
<p align=center>An Open-Source MacOS application that allows you to quit your open applications with a single click.</p>


## Compatibility list
| OS          | Compatible          |
| ----------- | ------------------- |
| MacOS 13.4  | :heavy_check_mark:  |

## Autostart the application
To autostart the application, you need to create an Automator application that runs the application on login. To do this, follow these steps:

1. Open Automator
2. Select "Application" as the document type
3. Copy the code below this list into the Automator application
4. Save the application as "One Click Quits" in your Applications folder
5. Search for Login Items in System Preferences
6. Add the application to the list of Login Items

Code for Automator application (Make sure to replace the path to the application):
```bash
#!/bin/bash
nohup /PATH/TO/PROGRAM > output.log &
kill $$
```

## Compile the application (For development only)
Make sure you have g++ 10.3.0 or Apple Clang 14.x.x installed.
Run the command below to compile the application:
```
make
```
Then run the command below to run the application:
```
./Program
```