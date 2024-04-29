Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.* 

1. What is the average current per period? (Remember, once you have measured your average current, average current is average current over all time. Average current doesn’t carry with it the units of the timespan over which it was measured).
   Answer: 14.53uA
   <br>Screenshot:  
   ![Avg_current_per_period_1](https://github.com/CU-ECEN-5823/ecen5823-assignment4-sriyagarde/assets/143979383/319e46b2-5c9b-4903-a40d-8f3626084511) 

2. What is the ave current from the time we sleep the MCU to EM3 until we power-on the 7021 in response to the LETIMER0 UF IRQ?
   Answer: 3.34uA
   <br>Screenshot:  
   ![Avg_current_LPM_Off](https://github.com/CU-ECEN-5823/ecen5823-assignment4-sriyagarde/assets/143979383/85e0f651-4e6e-4714-bf1e-f09a96e0dfe2)  

3. What is the ave current from the time we power-on the 7021 until we get the COMP1 IRQ indicating that the 7021's maximum time for conversion (measurement) has expired.
   Answer:
   <br>Screenshot: 333.05uA
   ![Avg_current_LPM_On](https://github.com/CU-ECEN-5823/ecen5823-assignment4-sriyagarde/assets/143979383/042c1733-1d81-48f5-8210-d63d496a0bb6) 

4. How long is the Si7021 Powered On for 1 temperature reading?
   Answer:
   <br>Screenshot:  96msec
   ![duration_lpm_on](https://github.com/CU-ECEN-5823/ecen5823-assignment4-sriyagarde/assets/143979383/0068de46-50e3-4a9a-b35c-9be332c3dc5b) 

5. Given the average current per period from Q1, calculate the operating time of the system for a 1000mAh battery? - ignoring battery manufacturers, efficiencies and battery derating - just a first-order calculation.
   Answer (in hours):
   To calculate the operating time of the system for a 1000mAh battery based on the average current consumption per period, we use the formula: 
   Operating time(hours) = Battery Cpacity(mAh)/Average Current(mA)
   Average Current = 14.53uA = 0.01453mA
   Operating time = 1000/0.01453 = 68823 hours


6. How has the power consumption performance of your design changed since the previous assignment?
   Answer:
   
The average current from the time we power-on the 7021 until the polled implementation of timerWaitUs() returns, indicating that the 7021's maximum time for conversion (measurement) has expired measured in Assignment 3 is 4130uA = 4.13mA
Whereas, the average current from the time we power-on the 7021 until we get the COMP1 IRQ indicating that the 7021's maximum time for conversion (measurement) has expired in Assignmemt 4 is 333.05uA.
The power consumed in Interrupt driven approach is 1.11mW whereas, in polled approach was 13.70mW. This means that the Interrupts driven approach took almost 13 times less power than Polled approach.
In polled approach, the processor is always on when the sensor is on until an event for Underflow(3 sec) is detected and hence a lot of energy is consumed.
In IRQ based approach, the Processor sleeps when I2C transactions take place and wakes up only when required. This reduces the power consumption by a significant amount.

