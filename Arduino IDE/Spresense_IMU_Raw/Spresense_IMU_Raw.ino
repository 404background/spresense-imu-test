#include <Arduino.h>
#include <Servo.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

extern "C" {
  #include <nuttx/sensors/cxd5602pwbimu.h>        // Definition of cxd5602pwbimu_data_t
  #include <arch/board/cxd56_cxd5602pwbimu.h>    // Declaration of board_cxd5602pwbimu_initialize
}

#define IMU_DEV       "/dev/imu0"
#define RATE_HZ       1920
#define ACC_RANGE_G   8
#define GYRO_RANGE_DPS 2000
#define FIFO_LEVEL    1

static int start_sensing(int fd, int rate, int adrange, int gdrange, int nfifo) {
  cxd5602pwbimu_range_t range;
  ioctl(fd, SNIOC_SSAMPRATE, rate);
  range.accel = adrange;
  range.gyro  = gdrange;
  ioctl(fd, SNIOC_SDRANGE, (unsigned long)(uintptr_t)&range);
  ioctl(fd, SNIOC_SFIFOTHRESH, nfifo);
  ioctl(fd, SNIOC_ENABLE, 1);
  return 0;
}

int imuFd;
Servo servo;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize IMU driver (bus number depends on board specification)
  if (board_cxd5602pwbimu_initialize(5) < 0) {
    Serial.println("IMU init failed");
    while (1);
  }
  Serial.println("IMU initialized");

  imuFd = open(IMU_DEV, O_RDONLY);
  if (imuFd < 0) {
    Serial.println("Cannot open IMU device");
    while (1);
  }
  start_sensing(imuFd, RATE_HZ, ACC_RANGE_G, GYRO_RANGE_DPS, FIFO_LEVEL);
  Serial.println("Start sensing...");

  servo.attach(PIN_D09);
  servo.write(90);
}

void loop() {
  cxd5602pwbimu_data_t data;
  if (read(imuFd, &data, sizeof(data)) == sizeof(data)) {
    // CSV format with fixed width: timestamp,temp,gx,gy,gz,ax,ay,az
    char buffer[128];
    sprintf(buffer, "%10lu,%6.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f", 
            data.timestamp, 
            data.temp,
            data.gx, data.gy, data.gz,
            data.ax, data.ay, data.az);
    Serial.println(buffer);
  }
}
