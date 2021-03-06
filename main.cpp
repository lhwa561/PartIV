#include <string>
#include <cmath>
#include <stdio.h>

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
    //LOG("Connected!\n");
    bleIsConnected = true;
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *cbParams)
{
    //LOG("Disconnected!\n");
    //LOG("Restarting the advertising process\n");
    ble.startAdvertising();
    bleIsConnected = false;
}
//uint32_t tick_counter = 0;
bool ticked = false;
void tick(void)
{
//    static uint32_t count = 0;
    //tick_counter++;
//    LOG("%d\r\n", count++);
    green = !green;
    ticked = true;
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

string custom_to_char1(int input) {
//    LOG("CURRENT INPUT IS: %i \r\n" ,input);
    int temp = input;
    char output[] = "+000";
    if (temp < 0) {
        temp = -1 * temp;
        output[0] = '-';
    }
    //int size = size_of(temp);
    
    for (int i = 3; i > 0; i--) {
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

double quat_convert(float input) {
    double output = 0;
    int temp = 0;
    
    temp = input / 100000;
    
    output = temp/10000.0;
    
    return output;
}

int main(void)
{
    blue  = 1;
    green = 1;
    red   = 1;

    pc.baud(115200);
    
    wait(1);
    
    //LOG("---- Seeed Tiny BLE ----\r\n");
    
    mbed_i2c_clear(MPU6050_SDA, MPU6050_SCL);
    mbed_i2c_init(MPU6050_SDA, MPU6050_SCL);
    

    if (mpu_init(0)) {
        //LOG("failed to initialize mpu6050\r\n");
    }
    
    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    /* Push both gyro and accel data into the FIFO. */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    mpu_set_sample_rate(100);
    //mpu_set_sample_rate(DEFAULT_MPU_HZ);
    
    /* Read back configuration in case it was set improperly. */
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    mpu_get_sample_rate(&gyro_rate);
    mpu_get_gyro_fsr(&gyro_fsr);
    mpu_get_accel_fsr(&accel_fsr);
    
    //LOG("accel fsr: %d \r\n", accel_fsr);
    //LOG("gyro fsr: %d \r\n", gyro_fsr);
    
    dmp_load_motion_driver_firmware();
    dmp_set_orientation(
        inv_orientation_matrix_to_scalar(board_orientation));
    
    uint16_t dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
                       DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(dmp_features);
    dmp_set_fifo_rate(DEFAULT_MPU_HZ);
    mpu_set_dmp_state(1);
    
    Ticker ticker;
    ticker.attach(tick, 0.01);

    //LOG("Initialising the nRF51822\r\n");
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

    int lastSent = 0;

    double t = 0.05;
    double t2 = 0.0025;
    
    double sX, sY, sZ, uX, uY, uZ;
    uX = 0;
    uY = 0;
    uZ = 0;
    sX = 0;
    sY = 0;
    sZ = 0;

    float aDivider = 16384/9.81;
    float gDivider = 16.4;
    double aX, aY, aZ, gX, gY, gZ;
    double q0, q1, q2, q3;
    double _2q0, _2q1, _2q2, _2q3;
    double ytemp1, ytemp2, ptemp, rtemp1, rtemp2;
  
    double roll, pitch, yaw;
    double gtempX, gtempY, gtempZ, aFilt;
    gtempX = 0;
    gtempY = 0;
    gtempZ = 0;
    aFilt = 0.2;
    
    int sCount = 0;
    
    double saX[10] = {0,0,0,0,0,0,0,0,0,0};
    double saY[10] = {0,0,0,0,0,0,0,0,0,0};
    double saZ[10] = {0,0,0,0,0,0,0,0,0,0};
    
    double vfX[10] = {0,0,0,0,0,0,0,0,0,0};
    double vfY[10] = {0,0,0,0,0,0,0,0,0,0};
    double vfZ[10] = {0,0,0,0,0,0,0,0,0,0};
    
    double sfX, sfY, sfZ;
    sfX = 0;
    sfY = 0;
    sfZ = 0;
    
    string s_string = "s+00000+00000+00000]";
    string e_string = "e+00000+00000+00000]";
    double tX, tY, tZ;
    tX = 0;
    tY = 0;
    tZ = 0;
    double p, q, r, k, x;
    p = 0;
    q = 0.2;
    r = 0.2;
    k = 0;
    x = 0;
    int rotation = 0;
    float rotVar = 0;
    int stationary = false;
    
    while (true) {
        if (ticked) {
            ticked = false;
            unsigned long sensor_timestamp;
            short gyro[3], accel[3], sensors;
            long quat[4];
            unsigned char more = 1;
            
            dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
            
                        
            aX = accel[0]/aDivider;
            aY = accel[1]/aDivider;
            aZ = accel[2]/aDivider;
            gX = gyro[0]/gDivider;
            gY = gyro[1]/gDivider;
            gZ = gyro[2]/gDivider;            
            
            q0 = quat_convert(quat[0]);
            q1 = quat_convert(quat[1]);
            q2 = quat_convert(quat[2]);
            q3 = quat_convert(quat[3]);

            _2q0 = q0*q0;
            _2q1 = q1*q1;
            _2q2 = q2*q2;
            _2q3 = q3*q3;
            
            ytemp1 = 2*q1*q2 - 2*q0*q3;
            ytemp2 = 2*(_2q0) + 2*(_2q1) - 1;
            ptemp = 2*q1*q3 + 2*q0*q2;
            rtemp1 = 2*q2*q3 - 2*q0*q1;
            rtemp2 = 2*(_2q0) + 2*(_2q3) - 1;
            
            yaw = atan2(ytemp1, ytemp2);
            pitch = -asin(ptemp);
            roll = atan2(rtemp1, rtemp2);
            
            /*Gravity Filter*/
            gtempX = (1-aFilt)*gtempX + aFilt*aX;
            aX = aX - gtempX;
            
            gtempY = (1-aFilt)*gtempY + aFilt*aY;
            aY = aY - gtempY;
                    
            gtempZ = (1-aFilt)*gtempZ + aFilt*aZ;
            aZ = aZ - gtempZ;
        
            /*
            kalman filter
            x = x
            p = p + q;

            k = p / (p + r);
            x = x + k * (measurement – x);
            p = (1 – k) * p;
            */
            double accMag = sqrt(aX*aX + aY*aY + aZ*aZ);
            p = p + q;
            k = p/(p + r);
            x = x + k*(accMag - x);
            p = (1 - k)*p;
            
            double gyroMag = sqrt(gX*gX + gY*gY + gZ*gZ);
            
            //Determine if motion has occured or not
            if (x < 1) {
                stationary = true;
            }
            else {
                stationary = false;
            }

            //Determine if Rotation has occured or not
            if (gyroMag < 50) {
                rotation = 0;
            }
            else if (gyroMag < 100) {
                rotation = 1;
            }
            else {
                rotation = 2;
            }
            
            //Set last acc as old acc            
            saX[0] = saX[1];
            saY[0] = saY[1];
            saZ[0] = saZ[1];
            
            //Set last vel as old vel
            vfX[0] = vfX[1];
            vfY[0] = vfY[1];
            vfZ[0] = vfZ[1];
            
            //Scale the accelerometer values if rotation is occuring and motion is happening
            if (!stationary) {
                if (rotation == 0) {
                    rotVar = 1;
                }
                else if (rotation == 1) {
                    rotVar = 0.2;
                }
                else {
                    rotVar = 0;
                }
                saX[1] = aX * rotVar;
                saY[1] = aY * rotVar;
                saZ[1] = aZ * rotVar;
            }
            else {
               saX[1] = 0;
               saY[1] = 0;
               saZ[1] = 0;              
            }
            vfX[1] = 0.5 * 0.01 * (saX[0] + saX[1]);
            vfY[1] = 0.5 * 0.01 * (saY[0] + saY[1]);
            vfZ[1] = 0.5 * 0.01 * (saZ[0] + saZ[1]);
                
            //LOG("%f %f %f\r\n",vfX[1], vfY[1], vfZ[1]);
            
            sfX = 0.5 * (vfX[0] + vfX[1]);
            sfY = 0.5 * (vfY[0] + vfY[1]);
            sfZ = 0.5 * (vfZ[0] + vfZ[1]);
            
            //count for 100
            if (sCount < 100) {
                //Print the last set output displacements
                const char *sstr = s_string.c_str();
                const char *estr = e_string.c_str();
                
                if (lastSent == 1) {
                    uartService.writeString(sstr);
                    lastSent = 0;
                }
                else {
                    uartService.writeString(estr);
                    lastSent = 1;
                }
                sCount++;
                tX += sfX;
                tY += sfY;
                tZ += sfZ;
            }
            else {
                //Set the output displacemenet for the next second
                LOG("%f %f %f %f %f %f\r\n", tX*100, tY*100, tZ*100, roll, pitch, yaw);
                s_string = "s" + custom_to_char(tX*10000) + custom_to_char(tY*10000) + custom_to_char(tZ*10000) + "]"; 
                e_string = "e" + custom_to_char(roll*10000) + custom_to_char(pitch*10000) + custom_to_char(yaw*10000) + "]";
                tX = 0;
                tY = 0;
                tZ = 0;
                sCount = 0;
            }

        }
        else {
            //CalcCounter++;
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

