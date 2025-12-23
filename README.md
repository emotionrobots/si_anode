# si_anode
Silicon Anode FGIC

This project runs a battery simulation that includes three entities:

- batt: simulates the dynamic of a battery based on equivalent circuit model or electrochemical model
- fgic: fuel gauge IC with BMS functions that predicts accurate SOC and SOH
- system: the system that represents load and charger

## Build
```
make                # build 
make clean          # clean the build
```


