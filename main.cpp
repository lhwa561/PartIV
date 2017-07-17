#include <string>

#include "mbed.h"
#include "mbed_i2c.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "nrf51.h"
#include "nrf51_bitfields.h"

#include "BLE.h"
#include "DFUService.h"
#include "UARTService.h"

#define LOG(...)    { pc.printf(__VA_ARGS__); }

#define LED_GREEN   p21
#define LED_RED     p22
#define LED_BLUE    p23
#define BUTTON_PIN  p17
#define BATTERY_PIN p1

#define MPU6050_SDA p12
#define MPU6050_SCL p13

#define UART_TX     p9
#define UART_RX     p11
#define UART_CTS    p8
#define UART_RTS    p10

/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (100)

DigitalOut blue(LED_BLUE);
DigitalOut green(LED_GREEN);
DigitalOut red(LED_RED);

InterruptIn button(BUTTON_PIN);
AnalogIn    battery(BATTERY_PIN);
Serial pc(UART_TX, UART_RX);

InterruptIn motion_probe(p14);

int read_none_count = 0;

BLEDevice  ble;
UARTService *uartServicePtr;

volatile bool bleIsConnected = false;
volatile uint8_t tick_event = 0;
volatile uint8_t motion_event = 0;
static signed char board_orientation[9] = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
};


void check_i2c_bus(void);
unsigned short inv_orientation_matrix_to_scalar( const signed char *mtx);


void connectionCallback(const Gap::ConnectionCallbackParams_t *params)
{
    LOG("Connected!\n");
    bleIsConnected = true;
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *cbParams)
{
    LOG("Disconnected!\n");
    LOG("Restarting the advertising process\n");
    ble.startAdvertising();
    bleIsConnected = false;
}
uint32_t tick_counter = 0;
void tick(void)
{
//    static uint32_t count = 0;
    tick_counter++;
//    LOG("%d\r\n", count++);
    green = !green;
}

void detect(void)
{
//    LOG("Button pressed\r\n");  
    blue = !blue;
}

void motion_interrupt_handle(void)
{
    motion_event = 1;
}

void tap_cb(unsigned char direction, unsigned char count)
{
//    LOG("Tap motion detected\r\n");
}

void android_orient_cb(unsigned char orientation)
{
//    LOG("Oriention changed\r\n");
}

int size_of(int input) {
    int temp = input;
    int output = 1;
    
    bool done = false;
    while(!done) {
        if (temp > 10) {
            output++;
            temp = temp / 10;
        }
        else {
            done = true;
        }
    }
    return output;
}

string custom_to_char(int input) {
//    LOG("CURRENT INPUT IS: %i \r\n" ,input);
    int temp = input;
    char output[] = "+00000";
    if (temp < 0) {
        temp = -1 * temp;
        output[0] = '-';
    }
    //int size = size_of(temp);
    
    for (int i = 5; i > 0; i--) {
        output[i] = temp%10 + '0';
        temp = temp/10;
    }
    return output;
}

string counter_to_char(int count) {
    char output[] = "[000000000000000000]";
    int size = size_of(count);
    int temp = count;
    if (size > 18) {
        for (int i = 1; i < 20; i++) {
            output[i] = '9';
        }
    }
    else {
        for (int i = 0; i < size; i++) {
            output[19 - i] = temp%10 + '0';
            temp = temp/10;
        }
    }
        
    return output;
}
int main(void)
{
    blue  = 1;
    green = 1;
    red   = 1;

    pc.baud(115200);
    
    wait(1);
    
    LOG("---- Seeed Tiny BLE ----\r\n");
    
    mbed_i2c_clear(MPU6050_SDA, MPU6050_SCL);
    mbed_i2c_init(MPU6050_SDA, MPU6050_SCL);
    

    if (mpu_init(0)) {
        LOG("failed to initialize mpu6050\r\n");
    }
    
    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    /* Push both gyro and accel data into the FIFO. */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    mpu_set_sample_rate(DEFAULT_MPU_HZ);
    
    /* Read back configuration in case it was set improperly. */
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    mpu_get_sample_rate(&gyro_rate);
    mpu_get_gyro_fsr(&gyro_fsr);
    mpu_get_accel_fsr(&accel_fsr);
    
    LOG("accel fsr: %d \r\n", accel_fsr);
    LOG("gyro fsr: %d \r\n", gyro_fsr);
    
    dmp_load_motion_driver_firmware();
    dmp_set_orientation(
        inv_orientation_matrix_to_scalar(board_orientation));
    dmp_register_tap_cb(tap_cb);
    dmp_register_android_orient_cb(android_orient_cb);
    
    uint16_t dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
                       DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
                       DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(dmp_features);
    dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    mpu_set_dmp_state(1);
    
    dmp_set_interrupt_mode(DMP_INT_GESTURE);
    dmp_set_tap_thresh(TAP_XYZ, 50);
    
    
    motion_probe.fall(motion_interrupt_handle);


    
    Ticker ticker;
    ticker.attach(tick, 0.1);

    button.fall(detect);

    LOG("Initialising the nRF51822\r\n");
    ble.init();
    ble.gap().onDisconnection(disconnectionCallback);
    ble.gap().onConnection(connectionCallback);
 
 
    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                     (const uint8_t *)"ARM", sizeof("ARM"));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
                                     (const uint8_t *)UARTServiceUUID_reversed, sizeof(UARTServiceUUID_reversed));
    DFUService dfu(ble);                                 
    UARTService uartService(ble);
    uartServicePtr = &uartService;
    //uartService.retargetStdout();
 
    ble.setAdvertisingInterval(160); /* 100ms; in multiples of 0.625ms. */
    ble.gap().startAdvertising();
    
    char r[4];
    r[1]='\r';
    r[2]='\n';
    r[3]='\0';
    
    int lastSent = 0;
    
    while (true) {
        //LOG("WHILE TRUE\r\n");
        static uint32_t currentCount = 0;
        
        if (tick_counter > currentCount/*motion_event*/) {\
            int c;      
            r[0]=c=uartService._getc();
            if (c<=0) {
                if (c == 'R' || c == 'r'){
                    red = 0;
                }
                else {
                    r[0] = '?';
                }
            }
            //LOG("C: %c \r\n", r[0]);
            
            unsigned long sensor_timestamp;
            short gyro[3], accel[3], sensors;
            long quat[4];
            unsigned char more = 1;
            
            dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
            if (accel_fsr == 2) {
                //16384
                //LOG("ACC: %i, %i, %i\r\n", accel[0], accel[1], accel[2]);                
            }
            if (gyro_fsr == 2000) {
                //16.4
                //LOG("GYRO: %i, %i, %i\r\n", gyro[0], gyro[1], gyro[2]);
            }
            //string s = "[" + custom_to_char(accel[0]) + "," + custom_to_char(accel[1]) + "," + custom_to_char(accel[2]) + "," + custom_to_char(gyro[0]) + "," + custom_to_char(gyro[1]) + "," + custom_to_char(gyro[2]) + "]";
            string a_string = "a" + custom_to_char(accel[0]) + custom_to_char(accel[1]) + custom_to_char(accel[2]) + "]"; 
            string g_string = "g" + custom_to_char(gyro[0]) + custom_to_char(gyro[1]) + custom_to_char(gyro[2]) + "]";
            		
			//LOG("String s: %s %s\r\n", a_string, g_string);
            //char s[] = "[" + custom_to_char(accel[0]) + "]"; // + "," + accel[1] + "," + accel[2] + "," + gyro[0] + "," + gyro[1] + "," + gyro[2] + "]";
            
            const char *astr = a_string.c_str();//"kellog\n";
            const char *gstr = g_string.c_str();
            if (lastSent == 1) {
                uartService.writeString(astr);
                lastSent = 0;
            }
            else {
                uartService.writeString(gstr);
                lastSent = 1;
            }
            /*
			LOG("%i\r\n", tick_counter);
            string count_string = counter_to_char(tick_counter);
            const char *s = count_string.c_str();
            uartService.writeString(s);
			*/
			
			
            currentCount = tick_counter;
            motion_event = 0;
        }
         
        if (motion_event) {
            
            motion_event = 0;
        }
        else {
            //LOG("WAITING FOR EVENT\r\n");
            ble.waitForEvent();
        }
    }
}

/* These next two functions converts the orientation matrix (see
 * gyro_orientation) to a scalar representation for use by the DMP.
 * NOTE: These functions are borrowed from Invensense's MPL.
 */
static inline unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}

unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}

