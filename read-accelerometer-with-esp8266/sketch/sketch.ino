#include "mpu9250.h"

/* Chip select is on pin 16 (Wemos D1 mini: D0) */
#define MPU9250_CS_PIN 16
/* Interrupt is on pin 0 (Wemos D1 mini: D1) */
#define MPU9250_INT_PIN digitalPinToInterrupt(5)

/* An MPU9250 object with the MPU-9250 sensor on SPI bus 0 */
bfs::Mpu9250 imu(&SPI, MPU9250_CS_PIN);

/* Keep track if interrupt is active */
volatile uint8_t irq_active = 0;
/* This is set in the ISR if new data is available */
volatile uint8_t new_data = 0;

void setup()
{
  /* Open the serial port, wait until ready */
  Serial.begin(115200);
  while (!Serial)
  {
  }

  SPI.begin();

  Serial.println("\n\nCalibrating...");

  /* Start communication with the IMU */
  if (!imu.Begin())
  {
    Serial.println("Error: IMU initialization unsuccessful. Check wiring.");
    Serial.print("Status: ");
    Serial.println(status);
    while (1)
    {
    }
  }

  /* Set bandwidth of the Digital Low Pass Filter to 20 Hz */
  bool status = imu.ConfigDlpfBandwidth(bfs::Mpu9250::DLPF_BANDWIDTH_20HZ);
  if (!status)
  {
    Serial.println("Error: Setting low pass filter value unsuccessful.");
    while (1)
    {
    }
  }

  /* Set the sample rate divider to 100 (results in 10 Hz update rate) */
  if (!imu.ConfigSrd(100))
  {
    Serial.println("Error: Setting sample rate unsuccessful.");
    while (1)
    {
    }
  }

  /* Enable the data ready interrupt */
  if (!imu.EnableDrdyInt())
  {
    Serial.println("Error: Activating interrupt failed.");
    while (1)
    {
    }
  }

  /* Attach the interrupt to microcontroller pin MPU9250_INT_PIN */
  pinMode(MPU9250_INT_PIN, INPUT);
  attachInterrupt(MPU9250_INT_PIN, isr_imu, FALLING);

  delay(500);
  Serial.println("\n\nInitialized MPU-9250! Let's go.");
  Serial.println("x\t\t y\t\t z");
  delay(500);

  irq_active = true;
}

void loop()
{
  if (new_data)
  {
    new_data = 0;

    /* Display the data (mps2 = meters per second squared (m/s^2) */
    Serial.print(imu.accel_x_mps2(), 6);
    Serial.print("\t");
    Serial.print(imu.accel_y_mps2(), 6);
    Serial.print("\t");
    Serial.print(imu.accel_z_mps2(), 6);
    Serial.println();
  }
}

ICACHE_RAM_ATTR void isr_imu()
{
  if (!irq_active)
  {
    return;
  }

  /* Read the sensor */
  if (imu.Read())
  {
    new_data = 1;
  }
}
