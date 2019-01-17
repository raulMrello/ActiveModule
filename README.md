# ActiveModule

ActiveModule is a C++ class implementation of the ACTIVE OBJECT design pattern as explained [here](http://www.state-machine.com/doc/AN_Active_Objects_for_Embedded.pdf) and following this common schema:

![](https://github.com/raulMrello/ActiveModule/AO.jpg)


  
## Changelog

---
### **17.01.2019**
- [x] Added ```./class_impl/build_impl.py``` script file to build ```ActiveModule``` derived classes in desired output folder.

---
### **07.03.2018**
- [x] Enabled ```DefaultPutTimeout``` to avoid dead-locks in ```mutex.lock```

---
### **14.02.2018**
- [x] Enabled ```ready``` flag once processed ```Init::EV_ENTRY```
  

---
### **13.02.2018*
- [x] Added MBED and ESP-IDF compatibility
