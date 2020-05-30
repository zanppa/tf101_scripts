/*
 * mpu3050_ioctl - A very simple userspace "driver" for
 * Invensense MPU3050 having the ioctl interface
 * i.e. 2011 version of the kernel driver.
 *
 * Copyright (C) 2020 Lauri Peltonen
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include<sys/ioctl.h>

#include "mpu.h"

#define SENSORS_ENA	(0x7F | 0x2000)	// 6 axis + temp

#define MAX_DATA_LEN	16		// Maximum data length from any read, bytes
#define GYRO_DATA_ADDR	0x1B		// Address of first gyroscope data register
#define GYRO_DATA_LEN	14		// Bytes of data to be read from gyro sensor

#define ACCEL_DATA_LEN	6

const char *dev_node = "/dev/mpu";

void get_status(int fd)
{
	unsigned char byte = 0;
	int ret;

	printf("Get mldl status..\n");
	ret = ioctl(fd, MPU_GET_MLDL_STATUS, &byte);
	printf("Return code: %d\n", ret);
	printf("Data: 0x%02x\n", byte);

	if(!ret) {
		if(byte & 0x01) printf("Gyro is suspended\n");
		if(byte & 0x02) printf("Accelerometer is suspended\n");
		if(byte & 0x04) printf("Compass is suspended\n");
		if(byte & 0x08) printf("Pressure is suspended\n");
		if(byte & 0x10) printf("Gyro is bypassed\n");
		if(byte & 0x20) printf("DMP is suspended\n");
		if(byte & 0x40) printf("Gyro needs config\n");
		if(byte & 0x80) printf("Device is suspended\n");
	}

	usleep(50000);
}

void read_gyro(int fd, float scale)
{
	int ret;
	struct mpu_read_write data_buf;
	float x, y, z;
	int temp;

	data_buf.data = malloc(GYRO_DATA_LEN);
	if(!data_buf.data) {
		printf("Malloc failed\n");
		return;
	}

	memset(data_buf.data, 0x00, GYRO_DATA_LEN);
	data_buf.address = GYRO_DATA_ADDR;
	data_buf.length = GYRO_DATA_LEN;

	ret = ioctl(fd, MPU_READ, &data_buf);
	if(ret) {
		printf("Ioctl returned %d\n", ret);
		return;
	}

	scale /= 32768.0;

	temp = ((data_buf.data[0] << 8) + data_buf.data[1]);
	x = ((signed short)((data_buf.data[2] << 8) + data_buf.data[3])) * scale;
	y = ((signed short)((data_buf.data[4] << 8) + data_buf.data[5])) * scale;
	z = ((signed short)((data_buf.data[6] << 8) + data_buf.data[7])) * scale;

//	printf("X: %06.2f\tY: %06.2f\tZ: %06.2f\tT: %d\n", x, y, z, temp);
	for(temp=0;temp<GYRO_DATA_LEN;temp++)
		printf("%02x ", data_buf.data[temp]);
	printf("\n");
	free(data_buf.data);
}

void read_accel(int fd, float scale)
{
	int ret;
	unsigned char *data_buf;
	float x, y, z;
	int temp;

	data_buf = malloc(ACCEL_DATA_LEN);
	if(!data_buf) {
		printf("Malloc failed\n");
		return;
	}

	memset(data_buf, 0x00, ACCEL_DATA_LEN);
	ret = ioctl(fd, MPU_READ_ACCEL, data_buf);
	//printf("Return code: %d\n", ret);
	if(ret) ret = errno;

	if(ret==113) printf("Accel data not ready\n");
	else if(ret==111) printf("Accel data overflow\n");
	else if(ret==112) printf("Accel data underflow\n");
	else if(ret==114) printf("Accel data error\n");
	else {
		scale /= 32768.0;

		x = ((signed short)((((data_buf[1] << 8) + data_buf[0]) & 0xFFF) << 4)) * scale;
		y = ((signed short)((((data_buf[3] << 8) + data_buf[2]) & 0xFFF) << 4)) * scale;
		z = ((signed short)((((data_buf[5] << 8) + data_buf[4]) & 0xFFF) << 4)) * scale;
		printf("X: %06.2f\tY: %06.2f\tZ: %06.2f\n", x, y, z);
//		for(temp=0;temp<ACCEL_DATA_LEN;temp++)
//			printf("%02x ", data_buf[temp]);
//		printf("\n");
	}


	free(data_buf);
}

void print_slave_plat_data(struct ext_slave_platform_data *slave)
{
	int i;
	printf("type: %d\n", slave->type);
	printf("irq: %d\n", slave->irq);
	printf("adapt_num: %d\n", slave->adapt_num);
	printf("bus: %d\n", slave->bus);
	printf("address: 0x%x\n", slave->address);
	printf("orientation: ");
	for(i=0;i<9;i++) printf("%d ", slave->orientation[i]);
	printf("\n");
}

int main()
{
	int fd;
	int ret;
	unsigned int i, j;
	unsigned char byte;
	long long_data;
	struct mpu_read_write data_buf;
	struct ext_slave_config cfg;
	struct ext_slave_platform_data slave;

	printf("Opening driver\n");
	fd = open(dev_node, O_RDWR);
	if(fd < 0) {
		printf("Could not open device file\n");
		return 0;
	}

	data_buf.data = malloc(MAX_DATA_LEN);
	if(!data_buf.data) {
		printf("Could not allocate data buffer\n");
		close(fd);
		return 0;
	}


	// RESET
	ret = ioctl(fd, MPU_RESUME, SENSORS_ENA);	// 0x7F => 6 axis (gyro+accel)
	printf("Return code: %d\n", ret);
	usleep(50000);

	// Reset
	data_buf.address = 0x3E;	// Power management
	data_buf.length = 1;
	data_buf.data[0] = 0x80;
	printf("Reset..\n");
	ret = ioctl(fd, MPU_WRITE, &data_buf);
	printf("Return code: %d\n", ret);
	usleep(50000);


	// POWER UP

	ret = ioctl(fd, MPU_RESUME, SENSORS_ENA);	// 0x7F => 6 axis (gyro+accel)
	printf("Return code: %d\n", ret);
	usleep(50000);

	// Read power byte
	memset(data_buf.data, 0x00, MAX_DATA_LEN);
	data_buf.address = 0x3E;	// Power management
	data_buf.length = 1;
	printf("Read power byte..\n");
	ret = ioctl(fd, MPU_READ, &data_buf);
	printf("Return code: %d\n", ret);
	printf("Data: 0x%02x\n", data_buf.data[0]);

	// Remove sleep mode
	data_buf.address = 0x3E;	// Power management
	data_buf.length = 1;
	data_buf.data[0] = data_buf.data[0] & (~0x40); // Sleep off -> clear 0x40
	printf("Remove sleep mode..\n");
	ret = ioctl(fd, MPU_WRITE, &data_buf);
	printf("Return code: %d\n", ret);
	usleep(50000);

	// Re-read power byte
	memset(data_buf.data, 0x00, MAX_DATA_LEN);
	data_buf.address = 0x3E;	// Power management
	data_buf.length = 1;
	printf("Read power byte..\n");
	ret = ioctl(fd, MPU_READ, &data_buf);
	printf("Return code: %d\n", ret);
	printf("Data: 0x%02x\n", data_buf.data[0]);

	// Read ID
/*	printf("Read ID, should be 0x68\n");
	memset(data_buf.data, 0x00, MAX_DATA_LEN);
	data_buf.address = 0x00;	// ID
	data_buf.length = 1;
	ret = ioctl(fd, MPU_READ, &data_buf);
	printf("Return code: %d\n", ret);
	printf("Data: 0x%02x\n", data_buf.data[0] & 0b1111110);	// ID is bits 6...1, bit 0 excluded...

	printf("Read version\n");
	memset(data_buf.data, 0x00, MAX_DATA_LEN);
	data_buf.address = 0x01;	// Product ID
	data_buf.length = 1;
	ret = ioctl(fd, MPU_READ, &data_buf);
	printf("Return code: %d\n", ret);
	printf("Data: 0x%02x\n", data_buf.data[0]);

	get_status(fd);
*/

	// CONFIGURE ACCELEROMETER

	// Turn on acclerometer by setting non-zero output data rate
	// given in 1/1000Hz
	long_data = 50000L;	// 50 Hz
	cfg.key = MPU_SLAVE_CONFIG_ODR_RESUME;
	cfg.len = sizeof(long_data);
	cfg.data = &long_data;
	cfg.apply = 1;
	printf("Config accel output data rate...\n");
	ret = ioctl(fd, MPU_CONFIG_ACCEL, &cfg);
	printf("Return code: %d\n", ret);

	// Set IRQ when data is ready
	long_data = MPU_SLAVE_IRQ_TYPE_DATA_READY;
	cfg.key = MPU_SLAVE_CONFIG_IRQ_RESUME;
	cfg.len = sizeof(long_data);
	cfg.data = &long_data;
	cfg.apply = 1;
	printf("Config accel irq...\n");
	ret = ioctl(fd, MPU_CONFIG_ACCEL, &cfg);
	printf("Return code: %d\n", ret);


	// Running... debug
/*
	printf("Get slave plat data..\n");
	memset(&slave, 0, sizeof(slave));
	slave.type = EXT_SLAVE_TYPE_ACCEL;
	ret = ioctl(fd, MPU_GET_EXT_SLAVE_PLATFORM_DATA, &slave);
	printf("Return code: %d\n", ret);
	print_slave_plat_data(&slave);
*/
	// Set slave platform data (not needed)
/*	memset(&slave, 0, sizeof(slave));
	slave.type = EXT_SLAVE_TYPE_ACCEL;
	slave.irq = 300;	// TODO
	slave.adapt_num = 0;	// i2c-0 on tf101
	slave.bus = EXT_SLAVE_BUS_SECONDARY;
	slave.addess = 0xF;
	slave.orientation = {1, 0, 0, 0, 1, 0, 0, 0, 1};
	ret = ioctl(fd, MPU_SET_EXT_SLAVE_PLATFORM_DATA, &slave);
	printf("Return code: %d\n", ret);
*/
/*
	i=0;
	printf("Reading req sensors byte..\n");
	ret = ioctl(fd, MPU_GET_REQUESTED_SENSORS, (char *)&i);
	printf("Return code: %d\n", ret);
	printf("Data: 0x%04x\n", i & 0xFFFF);
	usleep(50000);

	byte = 0;
	printf("Get slaves enabled..\n");
	ret = ioctl(fd, MPU_GET_I2C_SLAVES_ENABLED, &byte);
	printf("Return code: %d\n", ret);
	printf("Data: 0x%02x\n", byte);
	usleep(50000);
*/
//	ret = ioctl(fd, MPU_RESUME, SENSORS_ENA);	// 0x7F => 6 axis (gyro+accel)
//	printf("Return code: %d\n", ret);

/*
	// Set MPU to generate interrupt
	data_buf.address = 23;	// Interrupt conf
	data_buf.length = 1;
	data_buf.data[0] = 0;
	ret = ioctl(fd, MPU_READ, &data_buf);

	data_buf.data[0] |= 0x01;	// 1=raw data ready, 2=dmp is done, 4=mpu pll ready
	ret = ioctl(fd, MPU_WRITE, &data_buf);
	printf("Return code: %d\n", ret);
*/

/*
	// Set sample rate and ext sync
	data_buf.address = 22;		// low-pass, full-scale, ext sync
	data_buf.length = 1;
	data_buf.data[0] = 0x18 | 0x04;
	ret = ioctl(fd, MPU_WRITE, &data_buf);
	printf("Return code: %d\n", ret);
*/

	get_status(fd);
	printf("\n");


/*	printf("Read something\n");
	memset(data_buf.data, 0x00, MAX_DATA_LEN);
	data_buf.address = 24;
	data_buf.length = 1;
	ret = ioctl(fd, MPU_READ, &data_buf);
	printf("Return code: %d\n", ret);
	printf("Data: 0x%02x\n", data_buf.data[0]);
*/

	printf("\n");

	byte = 0xFF;	// 50 Hz
	cfg.key = MPU_SLAVE_FIFO_ENABLE;
	cfg.len = 1;
	cfg.data = &byte;
	cfg.apply = 1;
	ret = ioctl(fd, MPU_CONFIG_GYRO, &cfg);
	printf("Return code: %d\n", ret);


	printf("Reading sensor data...\n");
	for(j=0;j<10;j++) {
		data_buf.address = 26;	// Interrupt status (read clears)
		data_buf.length = 1;
		data_buf.data[0] = 0;
		ret = ioctl(fd, MPU_READ, &data_buf);
		printf("INT_stat: 0x%02x\n", data_buf.data[0]);

		data_buf.address = 58;	// fifo count
		data_buf.length = 2;
		data_buf.data[0] = 0;
		data_buf.data[1] = 0;
		ret = ioctl(fd, MPU_READ, &data_buf);
		printf("FIFO_len: 0x%02x 0x%02x\n", data_buf.data[0], data_buf.data[1]);

		//printf("Read %d: 0x%02x\n", read(fd, data_buf.data, 1), data_buf.data[0]);


		//read_gyro(fd, 250); // 250 deg/s default
		usleep(100000);
	}

/*
	printf("Read accelerometer...\n");
	for(j=0;j<10;j++) {
		read_accel(fd, 2.0);	// 2.0 g default
		usleep(100000);
	}
*/

	printf("Closing driver\n");
	close(fd);
	free(data_buf.data);

	return 0;
}
