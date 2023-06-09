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

## Limitations
Keep in mind that there are some apps which "One Click Quits" cannot terminate. Such as apps have background processes, or apps that continue running in the background once its window is closed.

This is a limitation due to how the logic to close applications was programmed. It does not listen to close events, rather it checks periodically if the application's window ID is still valid. To keep the program from closing apps that do not have windows, it checks if the app in question has a "modeOnlyBackground" flag. If it does, it will not close the app.

## Compile the application (For development only)
Make sure you have g++ 10.3.0 or Apple Clang 14.x.x installed.
Run the command below to compile the application:
```
make
```
Then run the command below to run the application:
```
./OneClickQuits
```