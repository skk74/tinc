#!/bin/env/python
import matplotlib.pyplot as plt
import numpy as np
import os


x=np.linspace(0,1,10);
y=np.linspace(0,1,10);
plt.plot(x,y);
plt.title(os.getcwd().split("/")[-1].replace("_",":"));
plt.savefig("voltage_curve.png");
