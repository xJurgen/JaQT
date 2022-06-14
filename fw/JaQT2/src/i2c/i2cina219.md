On 32V-2A mode the device configures INA219 to be able to measure up to 32V and 2A
            of current.  Each unit of current corresponds to 100uA, and
            each unit of power corresponds to 2mW. Counter overflow
            occurs at 3.2A. These calculations assume a 0.1 ohm resistor is present

On 32V-1A mode the device configures INA219 to be able to measure up to 32V and 1A
            of current.  Each unit of current corresponds to 40uA, and each
            unit of power corresponds to 800uW. Counter overflow occurs at
            1.3A. These calculations assume a 0.1 ohm resistor is present

Testing lower current measurements described in http://cpre.kmutnb.ac.th/esl/learning/ina219b-current-sensor/ina219_sensor_demo.ino

INA219 is able to get the raw current value and raw shunt voltage as a 16-bit signed integer (+-32767).
The shunt voltage is in unsigned mV range (+-327mV).
