#include <Arduino.h>
#include <ros.h>
#include <sensor_msgs/Imu.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

extern "C" {
  #include <nuttx/sensors/cxd5602pwbimu.h>
  #include <arch/board/cxd56_cxd5602pwbimu.h>
}

#define IMU_DEV "/dev/imu0"
#define RATE_HZ 1920
#define ACC_RANGE_G 8
#define GYRO_RANGE_DPS 2000
#define FIFO_LEVEL 1

ros::NodeHandle nh;
sensor_msgs::Imu imu_msg;
ros::Publisher imu_pub("imu/data_raw", &imu_msg);

int imuFd;

void setup() {
  nh.initNode();
  nh.advertise(imu_pub);

  if (board_cxd5602pwbimu_initialize(5) < 0) {
    nh.logerror("IMU init failed");
    while (1);
  }

  imuFd = open(IMU_DEV, O_RDONLY);
  if (imuFd < 0) {
    nh.logerror("Cannot open IMU device");
    while (1);
  }

  cxd5602pwbimu_range_t range;
  ioctl(imuFd, SNIOC_SSAMPRATE, RATE_HZ);
  range.accel = ACC_RANGE_G;
  range.gyro  = GYRO_RANGE_DPS;
  ioctl(imuFd, SNIOC_SDRANGE, (unsigned long)(uintptr_t)&range);
  ioctl(imuFd, SNIOC_SFIFOTHRESH, FIFO_LEVEL);
  ioctl(imuFd, SNIOC_ENABLE, 1);
}

void loop() {
  cxd5602pwbimu_data_t data;
  if (read(imuFd, &data, sizeof(data)) == sizeof(data)) {
    imu_msg.header.frame_id = "imu_link";
    imu_msg.header.stamp = nh.now();
    imu_msg.linear_acceleration.x = data.ax;
    imu_msg.linear_acceleration.y = data.ay;
    imu_msg.linear_acceleration.z = data.az;
    imu_msg.angular_velocity.x = data.gx;
    imu_msg.angular_velocity.y = data.gy;
    imu_msg.angular_velocity.z = data.gz;
    imu_pub.publish(&imu_msg);
  }
  nh.spinOnce();
}
