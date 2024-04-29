Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.*

1. Provide screen shot verifying the Advertising period matches the values required for the assignment= 250msec
   <br>Screenshot:  
   ![advertising_period](https://github.com/CU-ECEN-5823/ecen5823-assignment5-sriyagarde/assets/143979383/fef6819f-6482-42fc-9c46-93787babda0f)


2. What is the average current between advertisements - i.e. when the MCU is sleeping in EM2. (Don't measure an interval with a LETIMER UF event)?
   Answer: 2.69 uA
   <br>Screenshot:  
   ![avg_current_between_advertisements](https://github.com/CU-ECEN-5823/ecen5823-assignment5-sriyagarde/assets/143979383/60b2db9f-7679-49ac-8a5c-3913099dc277)

3. What is the peak current of an advertisement? 
   Answer: 4.78 mA
   <br>Screenshot:  
   ![peak_current_of_advertisement](https://github.com/CU-ECEN-5823/ecen5823-assignment5-sriyagarde/assets/143979383/b642c487-cbce-475d-8035-4fdeb4f3ab5f) 

4. Provide screen shot showing the connection interval setting. Does the connection interval match the values you requested in your slave(server) code, or the master's(client) values?. Yes. The terminal log shpws a value of interval = 24. 24*1.25 = 30 msec which can be verified through energy profiler below.
   <br>Screenshot: 
   ![connection_interval](https://github.com/CU-ECEN-5823/ecen5823-assignment5-sriyagarde/assets/143979383/2f185634-c840-4163-a1aa-a0e7b70c805b)

5. What is the average current between connection intervals - i.e. when the MCU is sleeping in EM2. (Don't measure an interval with a LETIMER UF event)?
   Answer: 383.46 uA
   <br>Screenshot:  
   ![avg_current_between_connection_intervals](https://github.com/CU-ECEN-5823/ecen5823-assignment5-sriyagarde/assets/143979383/b5b71d58-57c3-4140-b8ba-bc10c342e8ea)

6. If possible, provide screen shot verifying the slave latency matches what was reported when you logged the values from event = gecko_evt_le_connection_parameters_id. 
   <br>Screenshot:  Parameters id shows Peripheral latency = 0
   ![slave_latency](https://github.com/CU-ECEN-5823/ecen5823-assignment5-sriyagarde/assets/143979383/40d70e23-cdbb-4b2d-afbd-83f7da0ecda8) 

