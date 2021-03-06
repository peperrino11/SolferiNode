#these damn libraries
from digi.xbee.devices import XBeeDevice
from PIL import Image, ImageFile, ImageDraw, ImageFont
import PIL.Image as Image
import io
import os
import time
import dropbox, sys

#opening ports and baud rates
PORT="/dev/ttyUSB0"
BAUD_RATE = 9600

def main():
    print(" +-------------------------------------------------+")
    print(" | XBee Python Library Receive Data Polling Sample |")
    print(" +-------------------------------------------------+\n")

    device = XBeeDevice(PORT, BAUD_RATE)

    try:
        device.open()
        device.flush_queues()
        print("Waiting for data...\n")
        combo_data=bytearray()
        names=100;
        
        
        while True: #infinite loop
                
            xbee_message = device.read_data()
            
            if xbee_message is not None: #aka if xbee is receiving data
                
                r_data=xbee_message.data
                semaforo=r_data.decode('utf-8',errors="ignore") #trying to receive the 'close' command from tx
                combo_data+=r_data #accumulating bytes
                    
                if semaforo in ['CLOSE']: #all the bytes for image have been sent
                    
                    print("writing data to file")
                    time.sleep(1)
                    del combo_data[0:4] #first five characters of bytes are not needed for image
                    byte_data=bytes(combo_data) #bytearrays are immutable... bytes are mutable
                    array_length=len(byte_data)
                    del combo_data[array_length-5:array_length] #deleting 'CLOSE' bytes
                    
                    with open("bytes.txt","wb") as f:
                        
                        f.write(combo_data) #writing image bytes
                        f.close()
                        
                    with open("bytes.txt","rb") as f:
                        
                        img_data=f.read() #reading image bytes to create image
                        i=0
                        
                        while os.path.exists(f"/home/pi/Desktop/SolfeNode/Pics/Image{i}.jpeg"):
                            i+=1
                        ImageFile.LOAD_TRUNCATED_IMAGES = True #will not crash image if not all bytes are read
                        
                        try:
                            img=Image.open(io.BytesIO(img_data)) #this is what creates image from bytes
                        
                        except OSError as error:
                            print("OS Error Identified")
                            os.execv(sys.executable,['python']+sys.argv) #sometimes, not all bytes are sent and still crash code. if this happens, restart code
                        
                        newsize=(640,480)
                        img=img.resize(newsize)
                        img.show()
                        img.save(f"/home/pi/Desktop/SolfeNode/Pics/Image{i}.jpeg")
                        
                        f.close()
                        
                        with open(f"/home/pi/Desktop/SolfeNode/Pics/Image{i}.jpeg","rb") as fp:
                            contents=fp.read()
                        
                        #uploading images to dropbox
                        dbx=dropbox.Dropbox('FX0joxikCBMAAAAAAAAAAVdHYMj16J5XiZJ7oKwzH0dHIFoI0wmBD06WLjy389Ft')
                        dbx.files_upload(contents,f'/SolfeNode/image{i}.jpeg',dropbox.files.WriteMode.add,mute=True)          
                        fp.close()
                        
                        if os.path.exists("bytes.txt"):
                            os.remove("bytes.txt")
                            
                    combo_data=bytearray() #deleting all data to start clean for new image       
                
    finally:
        device.close()

if __name__ == '__main__':
    main()