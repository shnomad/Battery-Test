# Battery-Test
# systemd control service
# file name : jig_program.service

[Unit]
Description=Qt application autostart
After=graphical.target
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/Battery_LifeTime_Test/bin
#ExecStart=/home/pi/Battery_LifeTime_Test/bin/Battery_LifeTime_Test
ExecStart=/bin/bash -c '. "$0" && exec "$@"' /etc/profile.d/qt_eglfs.sh /home/pi/Battery_LifeTime_Test/bin/Battery_LifeTime_Test


[Install]
WantedBy=multi-user.target
