#include <Arduino.h>
#include <Servo.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <nuttx/sensors/cxd5602pwbimu.h>
#include <arch/board/cxd56_cxd5602pwbimu.h>
#include <math.h>

#define CXD5602PWBIMU_DEVPATH "/dev/imu0"
#define MESUREMENT_FREQUENCY 1920
#define MAX_NFIFO 1

Servo servo;
int imuFd;
static cxd5602pwbimu_data_t g_data[MAX_NFIFO];

static int start_sensing(int fd, int rate, int adrange, int gdrange, int nfifos) {
  cxd5602pwbimu_range_t range;
  ioctl(fd, SNIOC_SSAMPRATE, rate);
  range.accel = adrange;
  range.gyro = gdrange;
  ioctl(fd, SNIOC_SDRANGE, (unsigned long)(uintptr_t)&range);
  ioctl(fd, SNIOC_SFIFOTHRESH, nfifos);
  ioctl(fd, SNIOC_ENABLE, 1);
  return 0;
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial connection

  servo.attach(PIN_D09);
  servo.write(90); // Initial position

  board_cxd5602pwbimu_initialize(5);

  imuFd = open(CXD5602PWBIMU_DEVPATH, O_RDONLY);
  if (imuFd < 0) {
    Serial.println("Failed to open IMU device.");
    while (1); // Freeze to indicate error
  }

  start_sensing(imuFd, MESUREMENT_FREQUENCY, 8, 2000, MAX_NFIFO);
  Serial.println("IMU initialized.");
}

void loop() {
  cxd5602pwbimu_data_t data;

  ssize_t ret = read(imuFd, &data, sizeof(data));
  if (ret == sizeof(data)) {
    float ax = data.ax, ay = data.ay, az = data.az;
    float gx = data.gx, gy = data.gy, gz = data.gz;

    // Calculate angle from accelerometer data
    float acc_angle_x = atan2(ay, az + fabs(ax)) * 180.0 / PI;
    float acc_angle_y = atan2(ax, az + fabs(ay)) * 180.0 / PI;
    float acc_angle_z = atan2(ay, ax + fabs(az)) * 180.0 / PI;

    // Calculate angle from gyro data
    static float gyro_angle_x = 0, gyro_angle_y = 0, gyro_angle_z = 0;
    static unsigned long last = millis();
    unsigned long now = millis();
    float dt = (now - last) * 0.001;
    last = now;

    gyro_angle_x += gx * dt;
    gyro_angle_y += gy * dt;
    gyro_angle_z += gz * dt;

    // Fusion of angles using complementary filter
    float angleX = 0.98 * gyro_angle_x + 0.02 * acc_angle_x;
    float angleY = 0.98 * gyro_angle_y + 0.02 * acc_angle_y;
    float angleZ = 0.98 * gyro_angle_z + 0.02 * acc_angle_z;

    gyro_angle_x = angleX;
    gyro_angle_y = angleY;
    gyro_angle_z = angleZ;

    // Control servo motor based on X-axis angle
    int servoAng = constrain(map((int)angleX, -90, 90, 0, 180), 0, 180);
    servo.write(servoAng);

    // Debug output
    Serial.print("angleX: ");
    Serial.print(angleX, 2);  // Display with 2 decimal places
    Serial.print("\t");       // Tab for alignment
    Serial.print("angleY: ");
    Serial.print(angleY, 2);  // Display with 2 decimal places
    Serial.print("\t");       // Tab for alignment
    Serial.print("angleZ: ");
    Serial.print(angleZ, 2);  // Display with 2 decimal places
    Serial.print("\t");       // Tab for alignment
    Serial.print("Servo: ");
    Serial.println(servoAng);
  }
}
