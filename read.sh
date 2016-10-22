#Set permisions
DEVICE='ttyACM0'
# I set the permissions manually, though if you don't do that
# you may want to do something like the below
#sudo chmod o+rwx /dev/$DEVICE


#!/bin/bash

# Port setting
stty -F /dev/$DEVICE cs7 cstopb -ixon raw speed 115200

# Loop
while [ 1 ]; 
do
    echo -n 'rrr' > /dev/ttyACM0
    READ=`dd if=/dev/$DEVICE bs=1 count=127`
    echo $READ
done
