import serial.tools.list_ports
from PIL import Image
import serial, time, mss

REDUCTION_FACTOR = 10
NUM_SIDE_LED = 0
NUM_TOP_LED = 0
FADE_SPEED = 0

def main():

    ports = serial.tools.list_ports.comports()
    print([port.device for port in ports])

    port = ports[int(input("Which port? (0-?) "))]

    global NUM_SIDE_LED
    global NUM_TOP_LED
    global FADE_SPEED
    NUM_SIDE_LED = int(input("How many LEDs on the side? "))
    NUM_TOP_LED = int(input("How many LEDs on top? "))
    FADE_SPEED = int(input("Fade speed between frames? "))

    # Begin serial communication with ESP8266 chip
    ser = serial.Serial()
    ser.baudrate = 115200
    ser.port = port.device
    #ser.writeTimeout = 30000
    ser.timeout = 1
    ser.open()
    time.sleep(2)

    # Send initial data to microcontroller
    total_leds = 2 * NUM_SIDE_LED + NUM_TOP_LED
    init_bytes = bytearray([total_leds, FADE_SPEED])

    print("Sending init data...")
    ser.write(init_bytes)
    print(ser.readline().decode('ascii'))
    print(ser.readline().decode('ascii'))

    sct = mss.mss()
    
    while True:
        pixel_bytes = getByteArrayForScreen(sct)
        ser.write(pixel_bytes)
        time.sleep(0.03)

    ser.close()

def avgColor(screen, area):
    size = 1, 1
    temp_img = screen.crop(area)
    temp_img.thumbnail(size)
    return list(temp_img.getpixel((0, 0)))

def getByteArrayForScreen(sct):
    # Takes raw pixels from screen
    screen_source = sct.grab(sct.monitors[0])
    # Creates the image
    screen = Image.frombytes("RGB", screen_source.size, screen_source.bgra, "raw", "BGRX")

    w, h = screen.size

    height_pixels = int(h / NUM_SIDE_LED)
    width_pixels = int(w / NUM_TOP_LED)
    depth_pixels = int(REDUCTION_FACTOR)

    border_colors = []

    # Adds the color in a single array
    for left_index in range(0, NUM_SIDE_LED):
        area = (0, h - (left_index + 1) * height_pixels, depth_pixels, h - (left_index * height_pixels))
        border_colors.append(avgColor(screen, area))

    for top_index in range(0, NUM_TOP_LED):
        area = (top_index * width_pixels, 0, (top_index + 1) * width_pixels, depth_pixels)
        border_colors.append(avgColor(screen, area))

    for right_index in range(0, NUM_SIDE_LED):
        # area has a datatype of tuple because it is the most memory efficient
        # (left, upper, right, lower)
        area = (w - depth_pixels, right_index * height_pixels, w, (right_index + 1) * height_pixels)
        border_colors.append(avgColor(screen, area))

    # Copies the array data into a bytearray
    byte_list = bytearray()

    for border_color in border_colors:
        for color_index in border_color:
            byte_list.append(color_index)

    return byte_list


if __name__ == '__main__':
    main()