Note: For all assignments and Energy Profiler measurements you’ll be taking this semester,  Peak measurements are instantaneous measurements taken at a specific point in time. In the Energy Profiler, this is accomplished by left-clicking at a location along the time axis.
Average measurements are measurements that are taken over a time-span. In the Energy Profiler, this is accomplished by left-clicking and dragging a region along the time axis.

Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to StrongAlternateStrong?**
   Answer:
   The instantaneous current measured when a single LED is on with GPIO set to StrongAlternateStrong is 5.06mA.


**2. How much current does the system draw (instantaneous measurement) when a single LED is on with the GPIO pin set to WeakAlternateWeak?**
   Answer:
   The instantaneous current measured when a single LED is on with GPIO set to WeakAlternateWeak is 5.05mA.


**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, referencing the main board schematic, WSTK-Main-BRD4001A-A01-schematic.pdf or WSTK-Main-BRD4002A-A06-schematic.pdf, and AEM Accuracy in the ug279-brd4104a-user-guide.pdf. Both of these PDF files are available in the ECEN 5823 Student Public Folder in Google drive at: https://drive.google.com/drive/folders/1ACI8sUKakgpOLzwsGZkns3CQtc7r35bB?usp=sharing . Extra credit is available for this question and depends on your answer.**
   Answer:
   There is no significant difference between the values measured with StrongAlternateStrong vs WeakAlternateWeak.
   The Wireless STK Mainboard (BRD4001A) has an AEM (Advanced Energy Monitor module).Energy saving and efficiency are at the top of developers’ priorities for an ever-growing number of embedded systems applications. This partiular module is used to provide Real-time information on current consumption providing advanced energy software monitoring capabilities.
   The AEM circuitry on the Wireless Pro Kit Mainboard is capable of measuring current signals in the range of approximately 0.1 µA to
   95 mA.  The expected typical accuracy of the AEM on the Wireless Pro Kit Mainboard is within 1 %, except for currents in the low tens of microamps where offset errors start to dominate. In this low current region, the expected typical accuracy is some hundred nanoamps. At kitpower-up or on a power-cycle, an automatic AEM calibration is performed which compensates for offset errors in the current senseamplifiers. 
   Since the values in both cases lie between 0-95mA, AEM calibration is performed and the values come out to be approximately the same.
   
   


**4. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: The average current for 1 complete on-off cycle for 1 LED with an on-off duty cycle of 50% with the WeakAlternateWeak drive strength setting is 4.88mA.


**5. With the WeakAlternateWeak drive strength setting, what is the average current for 1 complete on-off cycle for 2 LEDs (both on at the time same and both off at the same time) with an on-off duty cycle of 50% (approximately 1 sec on, 1 sec off)?**
   Answer: The average current for 1 complete on-off cycle for 2 LEDs with an on-off duty cycle of 50% with the WeakAlternateWeak drive strength setting is 5.07mA





