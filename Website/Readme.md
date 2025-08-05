# Code for Locally Hosted Website to Present IMU Data

App.py is the flask back-end for the project. It is run from the command line to launch the website and calls all relevant files including webpages. Also includes the code to allow the website to be accessed by all devices on the local network. 

### To run the website:
* In the cmd navigate to the folder with app.py
* Run ```python app.py```
* Expected output:
  ```
  * Serving Flask app 'app'
    * Debug mode: on
   WARNING: This is a development server. Do not use it in a production deployment. Use a production WSGI server instead.
    * Running on all addresses (0.0.0.0)
    * Running on http://127.0.0.1:5000
    * Running on http://10.139.22.235:5000
   Press CTRL+C to quit
    * Restarting with stat
    * Debugger is active!
    * Debugger PIN: 474-086-179```
* http://127.0.0.1:5000 represents URL for local computer
* http://10.139.22.235:5000 represents URL that can be accessed by any device connected to the same wifi.
* The first time you run this you may need to give your firewall permission for external devices to access the website. 

### Static Folder
Contains the image for the home page of the website.

### Templates Folder
Contains all pages of the website.
* all:
  * not used for the final website - contains some snippets of code for graphs and errors - used for debugging.
* athlete:
   * contains the content for the athlete page with the movement tracker graph.
* complete:
   * contains the content for the summary page with all graphs and alarms. 
* health:
   * contains the content for the health monitor page with the heartrate and temperaturee graphs.
* health_card:
   * contains the content for the vitals summary page with tracking of data being received from the IMU. 
* index:
   * contains the content for the home page including the navigation to other pages.
* motion_intensity:
   * contains the content for the motion intensity page including the motion intensity graph. 
