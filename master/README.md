How to prepare Raspbian


1) Switch to read-only
2) Disable serial console
3) Install tools (gcc-avr, git)
4) Install source
5) Update nodes



1 Switching to read-only
~~~~~~~~~~~~~~~~~~~~~~~~



2 Disabling serial console
~~~~~~~~~~~~~~~~~~~~~~~~~~

Edit /boot/cmdline.txt and remove reference to AMA0

sudo systemctl disable serial-getty@ttyAMA0.service
sudo systemctl disable getty@ttyAMA0.service

3 Installing tools
~~~~~~~~~~~~~~~~~~

sudo apt-get install gcc-avr git python-gpiozero


4 Installing source
~~~~~~~~~~~~~~~~~~~

git clone https://github.com/kgoba/roombreak


5 Updating nodes
~~~~~~~~~~~~~~~~

cd roombreak
make update-all

