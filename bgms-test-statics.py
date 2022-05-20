import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

st_ble_1 = pd.read_csv('D:/1.WORK/B.PROJECT/27.CareSens N Premier BLE/non-DCDC-Test/ST/BLE/dat00001_00.csv', usecols=["Sweep #","Time","Chan 101 (VDC)", "Chan 102 (VDC)", "Chan 104 (VDC)", "Chan 105 (VDC)", "Chan 107 (VDC)", "Chan 108 (VDC)", "Chan 110 (VDC)", "Chan 111 (VDC)", "Chan 113 (VDC)", "Chan 114 (VDC)"],encoding='utf-8')
st_ble_1.rename(columns = {'Chan 101 (VDC)':'meter-6_bat','Chan 102 (VDC)':'meter-6_ref','Chan 104 (VDC)':'meter-7_bat','Chan 105 (VDC)':'meter-7_ref','Chan 107 (VDC)':'meter-8_bat','Chan 108 (VDC)':'meter-8_ref','Chan 110 (VDC)':'meter-9_bat','Chan 111 (VDC)':'meter-9_ref','Chan 113 (VDC)':'meter-10_bat','Chan 114 (VDC)':'meter-10_ref'},inplace=True)
st_ble_1 = (st_ble_1.loc[(st_ble_1['meter-6_ref'] >= 2)])

st_ble_2 = pd.read_csv('D:/1.WORK/B.PROJECT/27.CareSens N Premier BLE/non-DCDC-Test/ST/BLE/dat00001_01.csv', usecols=["Sweep #","Time","Chan 101 (VDC)", "Chan 102 (VDC)", "Chan 104 (VDC)", "Chan 105 (VDC)", "Chan 107 (VDC)", "Chan 108 (VDC)", "Chan 110 (VDC)", "Chan 111 (VDC)", "Chan 113 (VDC)", "Chan 114 (VDC)"],encoding='utf-8')
st_ble_2.rename(columns = {'Chan 101 (VDC)':'meter-6_bat','Chan 102 (VDC)':'meter-6_ref','Chan 104 (VDC)':'meter-7_bat','Chan 105 (VDC)':'meter-7_ref','Chan 107 (VDC)':'meter-8_bat','Chan 108 (VDC)':'meter-8_ref','Chan 110 (VDC)':'meter-9_bat','Chan 111 (VDC)':'meter-9_ref','Chan 113 (VDC)':'meter-10_bat','Chan 114 (VDC)':'meter-10_ref'},inplace=True)
st_ble_2 = (st_ble_2.loc[(st_ble_2['meter-6_ref'] >= 2)])

st_ble_3 = pd.read_csv('D:/1.WORK/B.PROJECT/27.CareSens N Premier BLE/non-DCDC-Test/ST/BLE/dat00001_02.csv', usecols=["Sweep #","Time","Chan 101 (VDC)", "Chan 102 (VDC)", "Chan 104 (VDC)", "Chan 105 (VDC)", "Chan 107 (VDC)", "Chan 108 (VDC)", "Chan 110 (VDC)", "Chan 111 (VDC)", "Chan 113 (VDC)", "Chan 114 (VDC)"],encoding='utf-8')
st_ble_3.rename(columns = {'Chan 101 (VDC)':'meter-6_bat','Chan 102 (VDC)':'meter-6_ref','Chan 104 (VDC)':'meter-7_bat','Chan 105 (VDC)':'meter-7_ref','Chan 107 (VDC)':'meter-8_bat','Chan 108 (VDC)':'meter-8_ref','Chan 110 (VDC)':'meter-9_bat','Chan 111 (VDC)':'meter-9_ref','Chan 113 (VDC)':'meter-10_bat','Chan 114 (VDC)':'meter-10_ref'},inplace=True)
st_ble_3 = (st_ble_3.loc[(st_ble_3['meter-6_ref'] >= 2)])

fig, ax = plt.subplots(2, 2, tight_layout=True)
#fig, ax = plt.subplots(2, 2)
fig.set_size_inches(16, 10)
fig.suptitle('DCDC removed BGMS Measurement Test',fontsize = 22, fontweight ='bold')

ax[0, 0].scatter(st_ble_1['Sweep #'],st_ble_1['meter-6_bat'],label='meter-6_bat',color='green')
ax[0, 0].scatter(st_ble_1['Sweep #'],st_ble_1['meter-6_ref'],label='meter-6_ref',color='red')
#ax[0, 0].set_title('non-DCDC BGMS Measurement Test, Count:1000',fontsize = 14, fontweight ='bold')
ax[0, 0].set_xlabel('sweep',loc='right',fontsize=14)
ax[0, 0].set_ylabel('Voltage(V)',loc='center',fontsize=14)
ax[0, 0].axis([0,9000,2.0,3.2])
ax[0, 0].set_yticks([2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3.0, 3.1, 3.2])
ax[0, 0].grid(True, axis='y')
secax1 = ax[0, 0].secondary_xaxis('top')
secax1.set_xlabel('Measure Count')
secax1.set_xticks(np.arange(0, 9460, 860), labels=['0','100','200','300','400','500','600','700','800','900','1000'])

ax[0, 1].scatter(st_ble_2['Sweep #'],st_ble_2['meter-6_bat'],label='meter-6_bat',color='green')
ax[0, 1].scatter(st_ble_2['Sweep #'],st_ble_2['meter-6_ref'],label='meter-6_ref',color='red')
#ax[0, 1].set_title('non-CDC BGMS Measurement Test, Count:2000',fontsize = 14, fontweight ='bold')
ax[0, 1].set_xlabel('sweep',loc='right',fontsize=14)
ax[0, 1].set_ylabel('Voltage(V)',loc='center',fontsize=14)
ax[0, 1].axis([0,9000,2.0,3.2])
ax[0, 1].set_yticks([2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3.0, 3.1, 3.2])
ax[0, 1].grid(True, axis='y')
secax1 = ax[0, 1].secondary_xaxis('top')
secax1.set_xlabel('Measure Count')
secax1.set_xticks(np.arange(0, 9460, 860), labels=['1001','1100','1200','1300','1400','1500','1600','1700','1800','1900','2000'])

ax[1, 0].scatter(st_ble_3['Sweep #'],st_ble_3['meter-6_bat'],label='meter-6_bat',color='green')
ax[1, 0].scatter(st_ble_3['Sweep #'],st_ble_3['meter-6_ref'],label='meter-6_ref',color='red')
#ax[1, 0].set_title('non-CDC BGMS Measurement Test, Count:2003',fontsize = 14, fontweight ='bold')
ax[1, 0].set_xlabel('sweep',loc='right',fontsize=14)
ax[1, 0].set_ylabel('Voltage(V)',loc='center',fontsize=14)
ax[1, 0].axis([0,9000,2.0,3.2])
ax[1, 0].set_yticks([2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3.0, 3.1, 3.2])
ax[1, 0].grid(True, axis='y')
secax1 = ax[1, 0].secondary_xaxis('top')
secax1.set_xlabel('Measure Count')
secax1.set_xticks(np.arange(0, 9460, 860), labels=['2001','2100','2200','2300','2400','2500','2600','2700','2800','2900','3000'])

ax[0,0].legend()
ax[0,1].legend()
ax[1,0].legend()

plt.savefig("test.png", dpi=150, facecolor='w')
#plt.show()
